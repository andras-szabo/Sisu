#include "stdafx.h"
#include <array>
#include "D3DRenderer.h"
#include "ICameraService.h"
#include "IGUIService.h"
#include "UIElement.h"

bool D3DRenderer::Init()
{
#if defined(DEBUG) || defined(_DEBUG)		// Enable D3D12 debug layer
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	_textureManager = std::make_unique<TextureManager>(this);

	Init_00_CreateDXGIFactory();
	Init_01_CreateDevice();
	Init_02_CreateFence();
	Init_03_QueryDescriptorSizes();
	Init_04_CheckMSAAQualitySupport();

#ifdef _DEBUG
	_logger = std::make_unique<D3DLogger>();
	_logger->LogAdapters(_dxgiFactory, _backBufferFormat);
#endif

	Init_05_CreateCommandObjects();
	Init_06_CreateSwapChain();
	Init_07_CreateRtvAndDsvDescriptorHeaps();

	Init_08_CreateUIHeap();

	OnResize();

	//TODO - can we actually fail here?
	return true;
}

ID3D12Resource* D3DRenderer::CurrentBackBuffer() const
{
	return _swapChainBuffer[_currentBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DRenderer::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		_currentBackBufferIndex,
		_RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DRenderer::DepthStencilView() const
{
	return _dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void D3DRenderer::ResetCommandList()
{
	ThrowIfFailed(
		_commandList->Reset(_commandAllocator.Get(), nullptr)
	);
}

void D3DRenderer::CloseAndExecuteCommandList()
{
	ThrowIfFailed(_commandList->Close());
	ID3D12CommandList* commandLists[] = { _commandList.Get() };
	// TODO - why _countof(commandLists)? We know that here it's
	// always going to be 1, no?
	_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
}

void D3DRenderer::FlushCommandQueue()
{
	// Advance fence value, and then add a new command
	// to the queue to set a new fence point.
	_currentFence++;
	ThrowIfFailed(_commandQueue->Signal(_fence.Get(), _currentFence));

	// Wait until the GPU has completed commands up to the new fence point
	if (_fence->GetCompletedValue() < _currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(_fence->SetEventOnCompletion(_currentFence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void D3DRenderer::OnResize()
{
	assert(_d3dDevice);
	assert(_swapChain);
	assert(_commandAllocator);

	auto dimensions = _windowManager->Dimensions();
	auto width = dimensions.first;
	auto height = dimensions.second;

	FlushCommandQueue();
	ThrowIfFailed(_commandList->Reset(_commandAllocator.Get(), nullptr));

	for (int i = 0; i < SwapChainBufferCount; ++i)
	{
		_swapChainBuffer[i].Reset();
	}

	_depthStencilBuffer.Reset();

	// Resize the swap chain
	ThrowIfFailed(_swapChain->ResizeBuffers(
		SwapChainBufferCount,
		width, height,
		_backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)
	);

	_currentBackBufferIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; ++i)
	{
		ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_swapChainBuffer[i])));
		_d3dDevice->CreateRenderTargetView(_swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, _RtvDescriptorSize);
	}

	CreateDepthBuffer();

	// Execute the resize commands

	CloseAndExecuteCommandList();
	FlushCommandQueue();

	// TODO - cleanup - do the scissorRects only
	SetupViewport();
	
	_cameraService->OnResize();

	//TODO: Call on callback
	//_gui->OnResize();
}

void D3DRenderer::Init_00_CreateDXGIFactory()
{
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));
}

void D3DRenderer::Init_01_CreateDevice()
{
	auto hardwareResult = D3D12CreateDevice
	(
		nullptr,					// default adapter
		D3D_FEATURE_LEVEL_12_0,		// min feature level
		IID_PPV_ARGS(&_d3dDevice)	// used to retrive an interface id and corresponding interface pointer
	);
}

void D3DRenderer::Init_02_CreateFence()
{
	ThrowIfFailed
	(
		_d3dDevice->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&_fence)
	)
	);
}

void D3DRenderer::Init_03_QueryDescriptorSizes()
{
	_RtvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_DsvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	_CbvSrvUavDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3DRenderer::Init_04_CheckMSAAQualitySupport()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = _backBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;

	ThrowIfFailed(
		_d3dDevice->CheckFeatureSupport
		(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)
	)
	);

	_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	// Could assert here that quality levels are fine
}

void D3DRenderer::Init_05_CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// So for the command queue, it looks fine to release _commandQueue and then to
	// get a new one. But for cmd allocator and cmd list, we don't want to release it.
	// I don't really understand; it should be uninitialized in the beginning anyway, no?
	ThrowIfFailed(_d3dDevice->
		CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));

	ThrowIfFailed(_d3dDevice->
		CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_commandAllocator.GetAddressOf())));

	ThrowIfFailed(_d3dDevice->
		CreateCommandList(
		0,		// nodeMask
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_commandAllocator.Get(),			// associated command allocator
		nullptr,							// initial PipelineStateObject
		IID_PPV_ARGS(_commandList.GetAddressOf())
	));

	_commandList->Close();
}

void D3DRenderer::Init_06_CreateSwapChain()
{
	_swapChain.Reset();

	auto dimensions = _windowManager->Dimensions();
	auto width = dimensions.first;
	auto height = dimensions.second;

	DXGI_SWAP_CHAIN_DESC sd;

	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = _backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	sd.SampleDesc.Count = _4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = _windowManager->MainWindowHandle();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(
		_dxgiFactory->CreateSwapChain
		(
		_commandQueue.Get(),
		&sd,
		_swapChain.GetAddressOf()
	)
	);
}

void D3DRenderer::Init_07_CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;		// clearly, we need as many desc as there are buffers
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	ThrowIfFailed(
		_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(_rtvHeap.GetAddressOf()))
	);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;		// clearly there's only 1 depth / stencil buffer we need
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	ThrowIfFailed(
		_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(_dsvHeap.GetAddressOf()))
	);

	// Q though: why do we have to have the dsv descriptor on a heap,
	// if theres is only one of it?... well, it seems that 'because we do' is the answer
}

void D3DRenderer::Init_08_CreateUIHeap()
{
	// How many views do we need?
	// One for each texture, that's independent of rame resources.
	// Then, per frame resource:
	//		- one for the camera
	//		- one for each ui item
	UINT numDescriptors = MaxTextureCount + FrameResourceCount * (1 + MaxUIObjectCount);
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDescriptor;

	cbvHeapDescriptor.NumDescriptors = numDescriptors;
	cbvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDescriptor.NodeMask = 0;

	ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&cbvHeapDescriptor, IID_PPV_ARGS(&_uiHeap)));
}

void D3DRenderer::Init_09_BuildUIConstantBufferViews()
{
	auto passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	// OK so _uiHeap should consist of:
	// - first 128 (or however many) views: for textures
	// then
	// first 3 views on the heap: pertaining to the UI camera,
	// for each of the 3 frame resources
	// and create one extra, per FR, for our mock ui quad

	auto textureViewIndexOffset = MaxTextureCount;

	// So first set per-pass views
	for (int frameIndex = 0; frameIndex < FrameResourceCount; ++frameIndex)
	{
		auto uiConstantBuffer = _frameResources[frameIndex]->UIPassConstantBuffer->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = uiConstantBuffer->GetGPUVirtualAddress();
		auto heapIndex = textureViewIndexOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(_uiHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, _CbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}

	auto passCBVoffset = FrameResourceCount;
	auto objectCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(FRObjectConstants));

	// Constant buffer view layout:
	// pass0, pass1, pass2; obj0-F0, obj1-F0 ... objn-F0, obj0-F1 ... objn-F1 ... etc.

	for (int frameIndex = 0; frameIndex < FrameResourceCount; ++frameIndex)
	{
		auto uiConstantBuffer = _frameResources[frameIndex]->UIObjectConstantBuffer->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = uiConstantBuffer->GetGPUVirtualAddress();

		for (int i = 0; i < MaxUIObjectCount; ++i)
		{
			auto heapIndex = MaxTextureCount + passCBVoffset + (MaxUIObjectCount * frameIndex) + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(_uiHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, _CbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress + (i * objectCBByteSize);
			cbvDesc.SizeInBytes = objectCBByteSize;

			_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 1> D3DRenderer::GetStaticSamplers() const
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shader sampler register
		D3D12_FILTER_MIN_MAG_MIP_POINT,		// filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// address mode for U
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// address mode for V
		D3D12_TEXTURE_ADDRESS_MODE_WRAP		// address mode for W
	);

	return { pointWrap };
}

void D3DRenderer::Init_10_BuildUIRootSignature()
{
	//TODO - there's gotta be a better way of keeping track which
	//		register is used for what. :(
	CD3DX12_ROOT_PARAMETER slotRootParams[3];
	CD3DX12_DESCRIPTOR_RANGE cbvTablePerPass;
	cbvTablePerPass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);		// per pass to register 0
	slotRootParams[0].InitAsDescriptorTable(1, &cbvTablePerPass);

	// For drawing instanced UI:
	slotRootParams[1].InitAsShaderResourceView(0, 1);		// per instance data to shader register 0 in
															// reguster space 1, so as not to overlap
															// with textures in register space 0.

	/* If we used constant buffers:
	CD3DX12_DESCRIPTOR_RANGE cbvTablePerObject;
	cbvTablePerObject.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);		// per object to register 1
	slotRootParams[1].InitAsDescriptorTable(1, &cbvTablePerObject);
	*/

	CD3DX12_DESCRIPTOR_RANGE srvTable;									// srv for teh texture to tex register 0
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	srvTable.BaseShaderRegister = 0;
	slotRootParams[2].InitAsDescriptorTable(1, &srvTable);
	slotRootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParams, 
		(UINT)staticSamplers.size(), staticSamplers.data(), 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	ThrowIfFailed(hr);
	ThrowIfFailed(_d3dDevice->CreateRootSignature(0,
		serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(),
		IID_PPV_ARGS(_uiRootSignature.GetAddressOf()))
	);
}

void D3DRenderer::RefreshUIItem(const UIElement& ui)
{
	auto& renderItem = _uiRenderItems[ui.renderItemIndex];

	DirectX::XMFLOAT4X4 xm
	(ui.scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, ui.scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		ui.position.x, ui.position.y, 0.0f, 1.0f);

	DirectX::XMStoreFloat4x4(&renderItem.World, DirectX::XMLoadFloat4x4(&xm));
	renderItem.NumFramesDirty = FrameResourceCount;

	_UIDirtyFrameCount = FrameResourceCount;
}

std::size_t D3DRenderer::AddUIRenderItem(const UIElement& ui)
{
	UIRenderItem quad;

	DirectX::XMFLOAT4X4 xm
		(ui.scale.x,	0.0f,			0.0f, 0.0f,
		 0.0f,			ui.scale.y,		0.0f, 0.0f,
		 0.0f,			0.0f,			1.0f, 0.0f,
		 ui.position.x, ui.position.y,	0.0f, 1.0f);

	DirectX::XMStoreFloat4x4(&quad.World, DirectX::XMLoadFloat4x4(&xm));
	quad.uvData = DirectX::XMFLOAT4(ui.uvData.x, ui.uvData.y, ui.uvData.z, ui.uvData.w);

	quad.Geo = _geometries["shapeGeo"].get();
	quad.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SubmeshGeometry q = quad.Geo->drawArgs["quad"];

	quad.IndexCount = q.indexCount;
	quad.StartIndexLocation = q.startIndexLocation;
	quad.BaseVertexLocation = q.baseVertexLocation;

	// CBVIndex should be: get first free CBV index,
	// or take the next one.
	std::size_t uiRenderItemCBVIndex = 0;
	if (_freeUIRenderItemIndices.size() > 0)
	{
		uiRenderItemCBVIndex = _freeUIRenderItemIndices.top();
		_freeUIRenderItemIndices.pop();

		quad.SetCBVIndex(uiRenderItemCBVIndex);
		_uiRenderItems[uiRenderItemCBVIndex] = quad;
	}
	else
	{
		uiRenderItemCBVIndex = _uiRenderItems.size();
		quad.SetCBVIndex(uiRenderItemCBVIndex);
		_uiRenderItems.push_back(quad);
	}

	_UIDirtyFrameCount = FrameResourceCount;

	return uiRenderItemCBVIndex;
}

void D3DRenderer::Init_12_BuildUIInputLayout()
{
	_uiShaders["instancedVS"] = d3dUtil::CompileShader(L"Shaders\\ui_instanced.hlsl", nullptr, "VS", "vs_5_1");
	_uiShaders["instancedPS"] = d3dUtil::CompileShader(L"Shaders\\ui_instanced.hlsl", nullptr, "PS", "ps_5_1");

	// _inputLayout is a std::vector<D3D12_INPUT_ELEMENT_DESC>; signature:
	// semantic name, semantic index, format, input slot, aligned byte offset, input slot class, instance data step rate 
	// about input slots: there are 16 of them (0-15), through which you can feed vertex data to the GPU
	// and notice that the order here doesn't matter; what matters is that the aligned byte offset
	// corresponds to the actual layout of the vertex struct (first position, then color)

	_uiInputLayout =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};
}

void D3DRenderer::Init_14_BuildUITextures()
{
	_textureManager->LoadFromFile("courier", L"Resources/Textures/font_courier_texture.dds");
	_textureManager->UploadToHeap("courier");
}

void D3DRenderer::Init_13_BuildUIPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC uiPSOdesc;
	ZeroMemory(&uiPSOdesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	uiPSOdesc.InputLayout = { _uiInputLayout.data(), (UINT)_uiInputLayout.size() };
	uiPSOdesc.pRootSignature = _uiRootSignature.Get();
	uiPSOdesc.VS =
	{
		reinterpret_cast<BYTE*>(_uiShaders["instancedVS"]->GetBufferPointer()),
		_uiShaders["instancedVS"]->GetBufferSize()
	};

	uiPSOdesc.PS =
	{
		reinterpret_cast<BYTE*>(_uiShaders["instancedPS"]->GetBufferPointer()),
		_uiShaders["instancedPS"]->GetBufferSize()
	};
	
	D3D12_RENDER_TARGET_BLEND_DESC bd;
	bd.BlendEnable = true;
	bd.LogicOpEnable = false;

	bd.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	bd.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	bd.BlendOp = D3D12_BLEND_OP_ADD;

	bd.SrcBlendAlpha = D3D12_BLEND_ONE;
	bd.DestBlendAlpha = D3D12_BLEND_ZERO;
	bd.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	bd.LogicOp = D3D12_LOGIC_OP_NOOP;
	bd.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	uiPSOdesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	uiPSOdesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	uiPSOdesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	uiPSOdesc.BlendState.RenderTarget[0] = bd;
	uiPSOdesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	uiPSOdesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	uiPSOdesc.SampleMask = UINT_MAX;
	uiPSOdesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	uiPSOdesc.NumRenderTargets = 1;
	uiPSOdesc.RTVFormats[0] = _backBufferFormat;
	uiPSOdesc.SampleDesc.Count = _4xMsaaState ? 4 : 1;
	uiPSOdesc.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
	uiPSOdesc.DSVFormat = _depthStencilFormat;

	ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&uiPSOdesc, IID_PPV_ARGS(&_uiPSO)));
}

void D3DRenderer::CreateDepthBuffer()
{
	auto dimensions = _windowManager->Dimensions();
	auto width = dimensions.first;
	auto height = dimensions.second;

	D3D12_RESOURCE_DESC bufferDesc;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = width;
	bufferDesc.Height = height;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;

	// We need typeless format because SSAO requires an SRV to read from
	// the depth buffer (also shadow mapping, right?). So we'll need
	// two views to the same resource => the resource should have a
	// typeless format.

	bufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	bufferDesc.SampleDesc.Count = _4xMsaaState ? 4 : 1;
	bufferDesc.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = _depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(_depthStencilBuffer.GetAddressOf())
	)
	);

	// Create descriptor to mip level 0 of entire resource, using the format of the resource
	// Because the resource is typeless, we need dsvDesc; otherwise in the call
	// to CreateDepthStencilView the 2nd parameter could be nullptr
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = _depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	_d3dDevice->CreateDepthStencilView(_depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer

	_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(_depthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

void D3DRenderer::SetupViewport()
{
	auto dimensions = _windowManager->Dimensions();
	auto width = dimensions.first;
	auto height = dimensions.second;

	_screenViewport.TopLeftX = 0;
	_screenViewport.TopLeftY = 0;
	_screenViewport.Width = static_cast<float>(width) / 2.0f;
	_screenViewport.Height = static_cast<float>(height);
	_screenViewport.MinDepth = 0.0f;
	_screenViewport.MaxDepth = 1.0f;

	_viewports.push_back(_screenViewport);
	_viewports.push_back(_screenViewport);

	_viewports[1].TopLeftX = static_cast<float>(width / 2.0f);

	_scissorRect = { 0, 0, width, height };
}

void D3DRenderer::UpdateUIPassBuffer(const GameTimer& gt, const D3DCamera& uiCamera)
{
	DirectX::XMMATRIX viewMatrix = DirectX::XMLoadFloat4x4(&uiCamera.ViewMatrix());
	DirectX::XMMATRIX projectionMatrix = DirectX::XMLoadFloat4x4(&uiCamera.ProjectionMatrix());

	DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
	DirectX::XMMATRIX inverseView = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(viewMatrix), viewMatrix);
	DirectX::XMMATRIX inverseProj = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(projectionMatrix), projectionMatrix);
	DirectX::XMMATRIX inverseViewProj = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(viewProj), viewProj);

	DirectX::XMStoreFloat4x4(&_uiPassConstants.view, DirectX::XMMatrixTranspose(viewMatrix));
	DirectX::XMStoreFloat4x4(&_uiPassConstants.invView, DirectX::XMMatrixTranspose(inverseView));
	DirectX::XMStoreFloat4x4(&_uiPassConstants.proj, DirectX::XMMatrixTranspose(projectionMatrix));
	DirectX::XMStoreFloat4x4(&_uiPassConstants.invProj, DirectX::XMMatrixTranspose(inverseProj));
	DirectX::XMStoreFloat4x4(&_uiPassConstants.viewProj, DirectX::XMMatrixTranspose(viewProj));
	DirectX::XMStoreFloat4x4(&_uiPassConstants.invViewProj, DirectX::XMMatrixTranspose(inverseViewProj));

	_uiPassConstants.eyePosW = uiCamera.Position();

	auto windowDimensions = _windowManager->Dimensions();
	_uiPassConstants.renderTargetSize = DirectX::XMFLOAT2((float)windowDimensions.first, (float)windowDimensions.second);
	_uiPassConstants.invRenderTargetSize = DirectX::XMFLOAT2(1.0f / windowDimensions.first, 1.0f / windowDimensions.second);
	_uiPassConstants.nearZ = 1.0f;
	_uiPassConstants.farZ = 1000.0f;
	_uiPassConstants.totalTime = gt.SecondsSinceReset();
	_uiPassConstants.deltaTime = gt.DeltaTimeSeconds();

	auto currPassCB = _currentFrameResource->UIPassConstantBuffer.get();
	currPassCB->CopyData(uiCamera.CbvIndex(), _uiPassConstants);
}

void D3DRenderer::UpdateUIInstanceData()
{
	if (_UIDirtyFrameCount > 0)
	{
		auto currentInstanceBuffer = _currentFrameResource->UIInstanceBuffer.get();
		UINT bufferIndex = 0;
		for (auto& uiRenderItem : _uiRenderItems)
		{
			DirectX::XMMATRIX worldMatrix = DirectX::XMLoadFloat4x4(&uiRenderItem.World);
			UIObjectConstants objConstants(worldMatrix, uiRenderItem.uvData);
			currentInstanceBuffer->CopyData(bufferIndex++, objConstants);
		}

		_UIDirtyFrameCount--;
		_drawableUIItemCount = bufferIndex;
	}

	/* If we were using constant buffers:
	auto currentInstanceBuffer = _currentFrameResource->UIObjectConstantBuffer.get();

	for (auto& uiRenderItem : _uiRenderItems)
	{
		if (uiRenderItem.NumFramesDirty > 0)
		{
			DirectX::XMMATRIX worldMatrix = DirectX::XMLoadFloat4x4(&uiRenderItem.World);
			UIObjectConstants objConstants(worldMatrix, uiRenderItem.uvData);
			currentInstanceBuffer->CopyData(uiRenderItem.GetCBVIndex(), objConstants);
			uiRenderItem.NumFramesDirty--;
		}
	}
	*/
}

//TODO: cmdList instead of _commandList
std::size_t D3DRenderer::DrawUI(ID3D12GraphicsCommandList* cmdList)
{
	static std::size_t drawCallCount;
	drawCallCount = 0;
	cmdList->SetPipelineState(_uiPSO.Get());
	_commandList->SetGraphicsRootSignature(_uiRootSignature.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { _uiHeap.Get() };
	_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto guiCamera = _cameraService->GetGUICamera();
	auto cameraCBVhandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(_uiHeap->GetGPUDescriptorHandleForHeapStart());
	//TODO what's up with the cam's per-pass stuff
	//cameraCBVhandle.Offset(guiCamera.CbvIndex(), _CbvSrvUavDescriptorSize);
	cameraCBVhandle.Offset(MaxTextureCount + _currentFrameResourceIndex, _CbvSrvUavDescriptorSize);

	_commandList->RSSetViewports(1, &(guiCamera->viewport));
	_commandList->SetGraphicsRootDescriptorTable(0, cameraCBVhandle);	// 0-> per pass => camera.

	// ... this is where we'd call "DrawAllUIRenderItems". But for now:
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(FRObjectConstants));
	auto objectCB = _currentFrameResource->UIObjectConstantBuffer->Resource();

	//Region setting ui texture
	//ID3D12DescriptorHeap* descHeaps[] = { _srvHeap.Get() };
	//_commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);
	CD3DX12_GPU_DESCRIPTOR_HANDLE tex(_uiHeap->GetGPUDescriptorHandleForHeapStart());
	//tex.Offset(renderItem->giveMeTextureIndex, _CbvSrvUavDescriptorSize);

	_commandList->SetGraphicsRootDescriptorTable(2, tex);				// 2-> texture
	//endregion

	const auto& uiRenderItem = _uiRenderItems[0];
	auto buffer = _currentFrameResource->UIInstanceBuffer->Resource();

	_commandList->IASetVertexBuffers(0, 1, &uiRenderItem.Geo->GetVertexBufferView());
	_commandList->IASetIndexBuffer(&uiRenderItem.Geo->GetIndexBufferView());
	_commandList->IASetPrimitiveTopology(uiRenderItem.PrimitiveType);
	_commandList->SetGraphicsRootShaderResourceView(1, buffer->GetGPUVirtualAddress());
	_commandList->DrawIndexedInstanced(uiRenderItem.IndexCount, _drawableUIItemCount,
		uiRenderItem.StartIndexLocation, uiRenderItem.BaseVertexLocation, 0);

	drawCallCount++;

	/* If we were to use constant buffers:
	auto perPassOffset = FrameResourceCount;
	auto perFrameOffset = MaxUIObjectCount * _currentFrameResourceIndex;
	for (const auto& uiRenderItem : _uiRenderItems)
	{
		auto objectCBVindex = uiRenderItem.GetCBVIndex();
		auto cbvIndex = MaxTextureCount + perPassOffset + perFrameOffset + objectCBVindex;
		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(_uiHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(cbvIndex, _CbvSrvUavDescriptorSize);

		_commandList->SetGraphicsRootDescriptorTable(1, cbvHandle);			// 1-> per object stuff
		_commandList->DrawIndexedInstanced(uiRenderItem.IndexCount, 1, uiRenderItem.StartIndexLocation,
										   uiRenderItem.BaseVertexLocation, 0);
		drawCallCount++;
	}*/

	return drawCallCount;
}

void D3DRenderer::WaitForNextFrameResource()
{
	_currentFrameResourceIndex = (_currentFrameResourceIndex + 1) % FrameResourceCount;
	_currentFrameResource = _frameResources[_currentFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current
	// frame resource? If not, wait until the GPU has completed commands
	// up to this fence point.

	if (_currentFrameResource->fence != 0 && _fence->GetCompletedValue() < _currentFrameResource->fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(
			_fence->SetEventOnCompletion(_currentFrameResource->fence, eventHandle)
		);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}