#pragma once
#include <wrl/client.h>

class GameTimer;
class ID3D12Device;
class ID3D12GraphicsCommandList;
class ID3D12DescriptorHeap;

struct IRenderer
{
	virtual ID3D12Device* GetDevice() = 0;
	virtual ID3D12GraphicsCommandList* GetCommandList() = 0;
	virtual Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetTextureHeap() = 0;
	virtual UINT GetSrvDescriptorSize() const = 0;

	virtual bool IsSetup() const = 0;
	virtual bool Init() = 0;
	virtual void OnResize() = 0;
	virtual void Update(const GameTimer& gt) = 0;
	virtual void Draw(const GameTimer& gt) = 0;
	virtual void SetDirty() = 0;
	virtual void SetWireframe(bool state) = 0;
};
