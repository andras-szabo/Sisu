#pragma once

// Link necessary d3d12 libraries
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#include "IRenderer.h"
#include <vector>
#include <assert.h>
#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include "d3dUtil.h"
#include "D3DLogger.h"
#include <DirectXColors.h>
#include "WindowManager.h"
#include "GameTimer.h"
#include "FrameResource.h"

class D3DRenderer : public IRenderer
{
public:
	static const int SwapChainBufferCount = 2;
	static const int FrameResourceCount = 3;

	D3DRenderer(WindowManager* windowManager, GameTimer* gameTimer) :
		_windowManager(windowManager),
		_gameTimer(gameTimer)
	{
	}

	virtual ~D3DRenderer()
	{
		if (_d3dDevice != nullptr) { FlushCommandQueue(); }
	}

	virtual bool Init() override;
	virtual bool IsSetup() const override { return _d3dDevice != nullptr; }

protected:
	virtual void OnResize() override;

	void Init_00_CreateDXGIFactory();
	void Init_01_CreateDevice();
	void Init_02_CreateFence();
	void Init_03_QueryDescriptorSizes();
	void Init_04_CheckMSAAQualitySupport();
	void Init_05_CreateCommandObjects();
	void Init_06_CreateSwapChain();
	void Init_07_CreateRtvAndDsvDescriptorHeaps();

	void CreateDepthBuffer();
	void SetupViewport();

	void ResetCommandList();
	void CloseAndExecuteCommandList();
	void FlushCommandQueue();

	void WaitForNextFrameResource();

protected:
	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	std::vector<std::unique_ptr<FrameResource>> _frameResources;
	FrameResource* _currentFrameResource = nullptr;
	int _currentFrameResourceIndex = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

	Microsoft::WRL::ComPtr<IDXGIFactory6> _dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;

	Microsoft::WRL::ComPtr<ID3D12Device> _d3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
	UINT64 _currentFence = 0;

	int _currentBackBufferIndex = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> _swapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> _depthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeap;	// render target descriptor heap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap;	// depth and stencil buffer desc heap

	UINT _RtvDescriptorSize = 0;			// render target descriptor size
	UINT _DsvDescriptorSize = 0;			// depth and stencil buffer descriptor size
	UINT _CbvSrvUavDescriptorSize = 0;		// constant buffer, shader resource, unordered acces descriptor sizes

	DXGI_FORMAT _backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT _depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// MSAA:
	bool _4xMsaaState = false;
	UINT _4xMsaaQuality = 0;

	D3D12_VIEWPORT _screenViewport;
	D3D12_RECT _scissorRect;

	std::unique_ptr<D3DLogger> _logger;
	WindowManager* _windowManager;
	GameTimer* _gameTimer;
};