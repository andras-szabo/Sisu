#include "stdafx.h"
#include "D3DRenderer.h"

bool D3DRenderer::Init()
{
#if defined(DEBUG) || defined(_DEBUG)		// Enable D3D12 debug layer
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

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

	SetupViewport();
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
	_screenViewport.Width = static_cast<float>(width);
	_screenViewport.Height = static_cast<float>(height);
	_screenViewport.MinDepth = 0.0f;
	_screenViewport.MaxDepth = 1.0f;

	_scissorRect = { 0, 0, width, height };
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