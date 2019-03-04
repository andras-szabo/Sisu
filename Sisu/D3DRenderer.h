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

class D3DCamera;
class GameTimer;
class ICameraService;
class IGUIService;

struct UIRenderItem
{
	UIRenderItem() = default;
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

	//TODO = NumFramesDirty should equal to renderer's FrameResourceCount,
	//		 but FFS dependencies
	int NumFramesDiry = 3;
	MeshGeometry* Geo = nullptr;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	void SetCBVIndex(UINT index) { _objectCBIndex = index; }
	UINT GetCBVIndex() const { return _objectCBIndex; }

private:
	UINT _objectCBIndex;
};

class D3DRenderer : public IRenderer
{
public:
	static const int SwapChainBufferCount = 2;
	static const int FrameResourceCount = 3;
	static const int MaxTextureCount = 128;

	D3DRenderer(WindowManager* const windowManager, 
				GameTimer* const gameTimer, 
				ICameraService* const cameraService,
				IGUIService* const guiService):
		_windowManager(windowManager),
		_gameTimer(gameTimer),
		_cameraService(cameraService),
		_gui(guiService)
	{
	}

	virtual ~D3DRenderer()
	{
		if (_d3dDevice != nullptr) { FlushCommandQueue(); }
	}

	virtual bool Init() override;
	virtual bool IsSetup() const override { return _d3dDevice != nullptr; }

	ID3D12Device* GetDevice() { return _d3dDevice == nullptr ? nullptr : _d3dDevice.Get(); }
	ID3D12GraphicsCommandList* GetCommandList()
	{
		return _commandList == nullptr ? nullptr : _commandList.Get();
	}

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetTextureHeap() { return _srvHeap; }
	UINT GetSrvDescriptorSize() const { return _CbvSrvUavDescriptorSize; }

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

	void Init_08_CreateUIHeap();
	void Init_09_BuildUIConstantBufferViews();
	void Init_10_BuildUIRootSignature();
	void Init_11_BuildUIRenderItems(MeshGeometry* geometries);
	void Init_12_BuildUIInputLayout();
	void Init_13_BuildUIPSO();

	void CreateDepthBuffer();
	void SetupViewport();

	void ResetCommandList();
	void CloseAndExecuteCommandList();
	void FlushCommandQueue();
	
	void UpdateUIPassBuffer(const GameTimer& gt, const D3DCamera& uiCamera);
	void UpdateUIInstanceData();
	void DrawUI(ID3D12GraphicsCommandList* cmdList);
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
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _srvHeap;	// shader resource view heap 

	// We should wrap this desc heap into something that's easier to expand
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _uiHeap;	// ui-specific heap
	Microsoft::WRL::ComPtr<ID3D12RootSignature> _uiRootSignature = nullptr;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> _uiShaders;
	std::vector<D3D12_INPUT_ELEMENT_DESC> _uiInputLayout;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>_uiPSO;

	std::vector<UIRenderItem> _uiRenderItems;
	PassConstants _uiPassConstants;

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
	std::vector<D3D12_VIEWPORT> _viewports;

	std::unique_ptr<D3DLogger> _logger;
	WindowManager* const _windowManager;
	GameTimer* const _gameTimer;
	ICameraService* const _cameraService;
	IGUIService* const _gui;
};