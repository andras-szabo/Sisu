#include "stdafx.h"
#include "BrickRenderer.h"
#include "InputService.h"

bool BrickRenderer::Init()
{
	if (!D3DRenderer::Init())
	{
		return false;
	}

	std::clog << "BrickRenderer init.\n";

	ResetCommandList();

	BuildRootSignatures();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();

	BuildFrameResources();

	BuildDescriptorHeaps();
	BuildConstantBufferViews();

	BuildPSOs();

	CloseAndExecuteCommandList();
	FlushCommandQueue();

	return true;
}

void BrickRenderer::Update(const GameTimer& gt)
{
	UpdateCamera(gt);
	WaitForNextFrameResource();
	UpdateInstanceData();
	UpdateMainPassCB(gt);
}

void BrickRenderer::UpdateCamera(const GameTimer& gt)
{
	static Sisu::Vector3 inputAxes;
	static Sisu::Vector3 inputEuler;

	auto a = _inputService->GetKey(KeyCode::A);
	auto d = _inputService->GetKey(KeyCode::D);
	auto s = _inputService->GetKey(KeyCode::S);
	auto w = _inputService->GetKey(KeyCode::W);
	auto q = _inputService->GetKey(KeyCode::Q);
	auto e = _inputService->GetKey(KeyCode::E);
	auto r = _inputService->GetKey(KeyCode::R);
	auto f = _inputService->GetKey(KeyCode::F);

	inputAxes.x = (a ? -1 : 0) + (d ? 1 : 0);
	inputAxes.z = (s ? -1 : 0) + (w ? 1 : 0);
	inputAxes.y = (f ? -1 : 0) + (r ? 1 : 0);

	inputEuler.z = (e ? -1.0 : 0) + (q ? 1.0 : 0);

	if (_inputService->GetMouseButton(0))
	{
		auto mouseDelta = _inputService->GetMouseDelta();
		inputEuler.y = mouseDelta.x; 
		inputEuler.x = mouseDelta.y;
	}
	else
	{
		inputEuler.x = 0.0f;
		inputEuler.y = 0.0f;
	}

	_camera.Update(gt, inputAxes, inputEuler);
}

void BrickRenderer::UpdateMainPassCB(const GameTimer& gt)
{
	DirectX::XMMATRIX viewMatrix = DirectX::XMLoadFloat4x4(_camera.ViewMatrix());
	DirectX::XMMATRIX projectionMatrix = DirectX::XMLoadFloat4x4(_camera.ProjectionMatrix());

	DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
	DirectX::XMMATRIX inverseView = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(viewMatrix), viewMatrix);
	DirectX::XMMATRIX inverseProj = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(projectionMatrix), projectionMatrix);
	DirectX::XMMATRIX inverseViewProj = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(viewProj), viewProj);

	DirectX::XMStoreFloat4x4(&_mainPassCB.view, DirectX::XMMatrixTranspose(viewMatrix));
	DirectX::XMStoreFloat4x4(&_mainPassCB.invView, DirectX::XMMatrixTranspose(inverseView));
	DirectX::XMStoreFloat4x4(&_mainPassCB.proj, DirectX::XMMatrixTranspose(projectionMatrix));
	DirectX::XMStoreFloat4x4(&_mainPassCB.invProj, DirectX::XMMatrixTranspose(inverseProj));
	DirectX::XMStoreFloat4x4(&_mainPassCB.viewProj, DirectX::XMMatrixTranspose(viewProj));
	DirectX::XMStoreFloat4x4(&_mainPassCB.invViewProj, DirectX::XMMatrixTranspose(inverseViewProj));

	_mainPassCB.eyePosW = _camera.Position();

	auto windowDimensions = _windowManager->Dimensions();
	_mainPassCB.renderTargetSize = DirectX::XMFLOAT2((float)windowDimensions.first, (float)windowDimensions.second);
	_mainPassCB.invRenderTargetSize = DirectX::XMFLOAT2(1.0f / windowDimensions.first, 1.0f / windowDimensions.second);
	_mainPassCB.nearZ = 1.0f;
	_mainPassCB.farZ = 1000.0f;
	_mainPassCB.totalTime = gt.SecondsSinceReset();
	_mainPassCB.deltaTime = gt.DeltaTimeSeconds();

	auto currPassCB = _currentFrameResource->passConstantBuffer.get();
	currPassCB->CopyData(0, _mainPassCB);
}

void BrickRenderer::UpdateInstanceData()
{	
	if (_dirtyFrameCount > 0)
	{
		auto currentInstanceBuffer = _currentFrameResource->instanceBuffer.get();

		UINT bufferIndex = 0;
		for (auto& brick : *_bricks)
		{
			if (brick.isVisible)
			{
				DirectX::XMMATRIX worldMatrix = ToXMMatrix(brick.transform);
				FRObjectConstants objConstants(worldMatrix);
				objConstants.color = DirectX::XMFLOAT4(brick.color.r, brick.color.g, brick.color.b, brick.color.a);
				currentInstanceBuffer->CopyData(bufferIndex++, objConstants);
			}
		}

		_dirtyFrameCount--;
		_drawableObjectCount = bufferIndex;
	}
}

void BrickRenderer::Draw(const GameTimer& gt)
{
	//--> ResetCommandAllocator
	auto commandAllocator = _currentFrameResource->commandAllocator;
	ThrowIfFailed(commandAllocator->Reset());

	//--> Reset command list with pipeline state
	auto currentPSOID = _isWireframe ? "instanced_wireframe" : "instanced";
	ThrowIfFailed(_commandList->Reset(commandAllocator.Get(), _PSOs[currentPSOID].Get()));

	_commandList->RSSetViewports(1, &_screenViewport);
	_commandList->RSSetScissorRects(1, &_scissorRect);

	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	_commandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::CornflowerBlue, 0, nullptr);
	_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);	
	_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { _cbvHeap.Get() };
	_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	_commandList->SetGraphicsRootSignature(_instancedRootSignature.Get());

	auto passCBVindex = _currentFrameResourceIndex;
	auto passCBVhandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	passCBVhandle.Offset(passCBVindex, _CbvSrvUavDescriptorSize);

	_commandList->SetGraphicsRootDescriptorTable(0, passCBVhandle);
	DrawBricks(_commandList.Get());

	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(_commandList->Close());

	ID3D12CommandList* cmdsLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	ThrowIfFailed(_swapChain->Present(0, 0));
	_currentBackBufferIndex = (_currentBackBufferIndex + 1) % SwapChainBufferCount;

	_currentFrameResource->fence = ++_currentFence;
	_commandQueue->Signal(_fence.Get(), _currentFence);
}

void BrickRenderer::DrawBricks(ID3D12GraphicsCommandList* cmdList)
{
	auto buffer = _currentFrameResource->instanceBuffer->Resource();
	auto brick = _geometries["shapeGeo"]->drawArgs["brick"];

	cmdList->IASetVertexBuffers(0, 1, &_geometries["shapeGeo"]->GetVertexBufferView());
	cmdList->IASetIndexBuffer(&_geometries["shapeGeo"]->GetIndexBufferView());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->SetGraphicsRootShaderResourceView(1, buffer->GetGPUVirtualAddress());
	cmdList->DrawIndexedInstanced(brick.indexCount, _drawableObjectCount, brick.startIndexLocation, brick.baseVertexLocation, 0);
}

void BrickRenderer::OnResize()
{
	D3DRenderer::OnResize();
	_camera.OnResize(_windowManager->AspectRatio());
}

void BrickRenderer::BuildShadersAndInputLayout()
{
	_shaders["instancedVS"] = d3dUtil::CompileShader(L"Shaders\\color_instanced.hlsl", nullptr, "VS", "vs_5_1");
	_shaders["instancedPS"] = d3dUtil::CompileShader(L"Shaders\\color_instanced.hlsl", nullptr, "PS", "ps_5_1");

	// _inputLayout is a std::vector<D3D12_INPUT_ELEMENT_DESC>; signature:
	// semantic name, semantic index, format, input slot, aligned byte offset, input slot class, instance data step rate 
	// about input slots: there are 16 of them (0-15), through which you can feed vertex data to the GPU
	// and notice that the order here doesn't matter; what matters is that the aligned byte offset
	// corresponds to the actual layout of the vertex struct (first position, then color).

	_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void BrickRenderer::BuildRootSignatures()
{
	// First, for instanced: we'll have 2 parameters: 
	//		= a descriptor table with 1 element for the per pass constant buffer
	//		= a root descriptor for the instance data
	CD3DX12_DESCRIPTOR_RANGE cbvTablePerPass;
	cbvTablePerPass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// base register 0

	CD3DX12_ROOT_PARAMETER slotRootParams[2];
	slotRootParams[0].InitAsDescriptorTable(1, &cbvTablePerPass);
	slotRootParams[1].InitAsShaderResourceView(0);					// register space t0, 
																	// becasue there's nothing to overlap with
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> serializedRS = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRS.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	ThrowIfFailed(hr);
	// OK now we can create it
	ThrowIfFailed(_d3dDevice->CreateRootSignature(
		0,
		serializedRS->GetBufferPointer(),
		serializedRS->GetBufferSize(),
		IID_PPV_ARGS(_instancedRootSignature.GetAddressOf()))
	);
}

DirectX::XMMATRIX BrickRenderer::ToXMMatrix(const Sisu::Matrix4& m) const
{
	//TODO - we could probably optimize this.
	DirectX::XMFLOAT4X4 xm(m.r0.x, m.r0.y, m.r0.z, m.r0.w,
						   m.r1.x, m.r1.y, m.r1.z, m.r1.w,
						   m.r2.x, m.r2.y, m.r2.z, m.r2.w,
						   m.r3.x, m.r3.y, m.r3.z, m.r3.w);
	
	return DirectX::XMLoadFloat4x4(&xm);
}

void BrickRenderer::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	auto box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 0);

	UINT boxVertexOffset = 0;
	UINT boxIndexOffset = 0;

	SubmeshGeometry boxSubmesh;
	boxSubmesh.indexCount = (UINT)box.Indices32.size();
	boxSubmesh.startIndexLocation = boxIndexOffset;
	boxSubmesh.baseVertexLocation = boxVertexOffset;

	// Extract the vertex elements we are interested in, and pack
	// the vertices of all the meshes into one vertex buffer.
	//	--> ? What do you mean, the vertex elements we're interested in?
	//	Ah, as in: just give me pos and color, for now we're not
	//  interested about normals and whatnot.

	auto totalVertexCount = box.Vertices.size();
	std::vector<BrickVertex> vertices(totalVertexCount);

	UINT vbIndex = 0;
	vbIndex = AddToVertexBuffer(box, vertices, vbIndex, DirectX::XMFLOAT4(DirectX::Colors::White));

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(BrickVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->vertexBuffersCPU[0]));
	CopyMemory(geo->vertexBuffersCPU[0]->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->indexBufferCPU));
	CopyMemory(geo->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->vertexBuffersGPU[0] = d3dUtil::CreateDefaultBuffer(_d3dDevice.Get(),
		_commandList.Get(), vertices.data(), vbByteSize, geo->vertexBufferUploader);

	geo->indexBufferGPU = d3dUtil::CreateDefaultBuffer(_d3dDevice.Get(),
		_commandList.Get(), indices.data(), ibByteSize, geo->indexBufferUploader);

	geo->vertexByteStride[0] = sizeof(BrickVertex);
	geo->vertexBufferByteSize[0] = vbByteSize;
	geo->indexFormat = DXGI_FORMAT_R16_UINT;
	geo->indexBufferByteSize = ibByteSize;

	geo->drawArgs["brick"] = boxSubmesh;

	_geometries[geo->name] = std::move(geo);
}

UINT BrickRenderer::AddToVertexBuffer(const GeometryGenerator::MeshData& mesh,
	std::vector<BrickVertex>& vertices, UINT startIndex, DirectX::XMFLOAT4 color) const
{
	for (std::size_t i = 0; i < mesh.Vertices.size(); ++i, ++startIndex)
	{
		vertices[startIndex].Pos = mesh.Vertices[i].Position;
	}

	return startIndex;
}

void BrickRenderer::BuildFrameResources()
{
	UINT passCount = 1;
	UINT cbObjectCount = 0;
	for (int i = 0; i < FrameResourceCount; ++i)
	{
		_frameResources.push_back(
			std::make_unique<FrameResource>(_d3dDevice.Get(), passCount, cbObjectCount, MaxInstancedObjectCount)
		);
	}
}

// For now, we'll only build heap enough for 2 per-pass views,
// because that's all we're going to use constant buffer for.
void BrickRenderer::BuildDescriptorHeaps()
{
	UINT numDescriptors = FrameResourceCount;		// one view per frame resource, that's it
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDescriptor;

	cbvHeapDescriptor.NumDescriptors = numDescriptors;
	cbvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDescriptor.NodeMask = 0;
	ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&cbvHeapDescriptor, IID_PPV_ARGS(&_cbvHeap)));
}

void BrickRenderer::BuildConstantBufferViews()
{
	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	// The first 3 descriptors are the pass CBVs for each frame resource
	for (int frameIndex = 0; frameIndex < FrameResourceCount; ++frameIndex)
	{
		auto passConstantBuffer = _frameResources[frameIndex]->passConstantBuffer->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passConstantBuffer->GetGPUVirtualAddress();

		// Offset to the pass constant buffer view in the descriptor heap:
		int heapIndex = frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, _CbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void BrickRenderer::BuildPSOs()
{
	// PSO for instanced objects
	D3D12_GRAPHICS_PIPELINE_STATE_DESC instancedPSOdesc;
	ZeroMemory(&instancedPSOdesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	instancedPSOdesc.InputLayout = { _inputLayout.data(), (UINT)_inputLayout.size() };
	instancedPSOdesc.pRootSignature = _instancedRootSignature.Get();
	instancedPSOdesc.VS =
	{
		reinterpret_cast<BYTE*>(_shaders["instancedVS"]->GetBufferPointer()),
		_shaders["instancedVS"]->GetBufferSize()
	};

	instancedPSOdesc.PS =
	{
		reinterpret_cast<BYTE*>(_shaders["instancedPS"]->GetBufferPointer()),
		_shaders["instancedPS"]->GetBufferSize()
	};

	instancedPSOdesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	instancedPSOdesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	instancedPSOdesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	instancedPSOdesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	instancedPSOdesc.SampleMask = UINT_MAX;
	instancedPSOdesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	instancedPSOdesc.NumRenderTargets = 1;
	instancedPSOdesc.RTVFormats[0] = _backBufferFormat;
	instancedPSOdesc.SampleDesc.Count = _4xMsaaState ? 4 : 1;
	instancedPSOdesc.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
	instancedPSOdesc.DSVFormat = _depthStencilFormat;

	ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&instancedPSOdesc, IID_PPV_ARGS(&_PSOs["instanced"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframePSOdesc = instancedPSOdesc;
	wireframePSOdesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	wireframePSOdesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&wireframePSOdesc, IID_PPV_ARGS(&_PSOs["instanced_wireframe"])))
}