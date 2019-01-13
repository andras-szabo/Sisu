#include "stdafx.h"
#include "BrickRenderer.h"

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

void BrickRenderer::PreDraw()
{
	WaitForNextFrameResource();
	
	if (_dirtyFrameCount > 0)
	{
		auto currentInstanceBuffer = _currentFrameResource->instanceBuffer.get();

		UINT bufferIndex = 0;
		for (const auto& brick : _bricks)
		{
			if (brick.isVisible)
			{
				DirectX::XMMATRIX worldMatrix = ToXMMatrix(brick.worldMatrix);
				FRObjectConstants objConstants(worldMatrix);
				currentInstanceBuffer->CopyData(bufferIndex++, objConstants);
			}
		}

		_dirtyFrameCount--;
	}
}

void BrickRenderer::Draw(GameTimer* gt)
{
	//TODO
}

void BrickRenderer::OnResize()
{
	D3DRenderer::OnResize();
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi,
		_windowManager->AspectRatio(), 1.0f, 1000.0f);
	DirectX::XMStoreFloat4x4(&_projMatrix, projectionMatrix);
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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

void BrickRenderer::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	auto box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);

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

	geo->drawArgs["box"] = boxSubmesh;

	_geometries[geo->name] = std::move(geo);
}

UINT BrickRenderer::AddToVertexBuffer(const GeometryGenerator::MeshData& mesh,
	std::vector<BrickVertex>& vertices, UINT startIndex, DirectX::XMFLOAT4 color) const
{
	for (std::size_t i = 0; i < mesh.Vertices.size(); ++i, ++startIndex)
	{
		vertices[startIndex].Pos = mesh.Vertices[i].Position;
		vertices[startIndex].Color = color;
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

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = instancedPSOdesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&_PSOs["instanced_wireframe"])))
}