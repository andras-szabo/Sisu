#pragma once
#include <vector>
#include <wrl/client.h>

class GameTimer;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
struct UIElement;

struct IRenderer
{
	virtual ~IRenderer() {};

	virtual ID3D12Device* GetDevice() = 0;
	virtual ID3D12GraphicsCommandList* GetCommandList() = 0;
	virtual Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetTextureHeap() = 0;
	virtual UINT GetSrvDescriptorSize() const = 0;

	virtual bool IsSetup() const = 0;
	virtual bool Init() = 0;
	virtual void OnResize() = 0;
	virtual void Update(const GameTimer& gt) = 0;
	virtual std::size_t Draw(const GameTimer& gt) = 0;
	virtual void SetDirty() = 0;
	virtual void SetWireframe(bool state) = 0;

	virtual std::size_t AddUIRenderItem(const UIElement& uiElement) = 0;
	virtual void RefreshUIItem(const UIElement& uiElement) = 0;
};
