#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

struct FRObjectConstants	// FR, as in, Frame Resource, to not confuse this with e.g. BoxApp::ObjectConstants
{
	FRObjectConstants(const DirectX::XMMATRIX& worldMatrixToInit)
	{
		// We need to transpose the world matrix, because "by default,
		// matrices in the constant buffer are expected to be in column
		// major format, whereas on the C++ side we're working with
		// row-major matrices." Because DirectX.
		DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixTranspose(worldMatrixToInit));
	}

	DirectX::XMFLOAT4X4 worldMatrix = MathHelper::Identity4x4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 view = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 invView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 proj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 invProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 viewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 invViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT3 eyePosW = { 0.0f, 0.0f, 0.0f };

	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 renderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };
	float nearZ = 0.0f;
	float farZ = 0.0f;
	float totalTime = 0.0f;
	float deltaTime = 0.0f;
};

struct FrameResource
{
	FrameResource(ID3D12Device* device, UINT passCount, UINT cbObjectCount, UINT maxInstancedObjectCount)
		: _objectCount (0)
	{
		ThrowIfFailed(
			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(commandAllocator.GetAddressOf()))
		);

		// ddevice*, count, isConstantBuffer
		passConstantBuffer = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
		objectConstantBuffer = std::make_unique<UploadBuffer<FRObjectConstants>>(device, cbObjectCount, true);

		instanceBuffer = std::make_unique<UploadBuffer<FRObjectConstants>>(device, maxInstancedObjectCount, false);
	}

	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource() {}

	bool UpdateConstantBufferIfNeeded(ID3D12Device* device, std::size_t objectCount)
	{
		if (objectCount != _objectCount)
		{
			_objectCount = objectCount;
			objectConstantBuffer = nullptr;
			objectConstantBuffer = std::make_unique<UploadBuffer<FRObjectConstants>>(device, objectCount, true);
			return true;
		}

		return false;
	}

public:
	// We cannot reset the command allocator until the GPU is done processing
	// the commands. => Each frame needs their own allocator, and also their
	// own constant buffers.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	std::unique_ptr<UploadBuffer<PassConstants>> passConstantBuffer = nullptr;
	std::unique_ptr<UploadBuffer<FRObjectConstants>> objectConstantBuffer = nullptr;
	std::unique_ptr<UploadBuffer<FRObjectConstants>> instanceBuffer = nullptr;

	UINT64 fence = 0;

private:
	UINT _objectCount;
};
