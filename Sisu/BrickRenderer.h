#pragma once
#include "D3DRenderer.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"

class GameTimer;

struct BrickVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

class BrickRenderer : public D3DRenderer
{
public:
	static const UINT MaxInstancedObjectCount = 4096;

	BrickRenderer(WindowManager* windowManager, GameTimer* gameTimer) :
		D3DRenderer(windowManager, gameTimer)
	{
	}

	virtual bool Init() override;
	virtual void Draw(GameTimer* gt) override;

protected:
	virtual void OnResize() override;

private:
	void BuildDescriptorHeaps();
	void BuildConstantBufferViews();
	void BuildShapeGeometry();
	void BuildFrameResources();			// This is what builds the constant buffers
	void BuildRootSignatures();
	void BuildShadersAndInputLayout();
	void BuildPSOs();

	UINT AddToVertexBuffer(const GeometryGenerator::MeshData& mesh,
		std::vector<BrickVertex>& vertices,
		UINT startIndex,
		DirectX::XMFLOAT4 color) const;

private:
	DirectX::XMFLOAT3 _eyePos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4X4 _viewMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 _projMatrix = MathHelper::Identity4x4();

	std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayout;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> _shaders;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> _geometries;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> _PSOs;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _cbvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> _instancedRootSignature = nullptr;

	PassConstants _mainPassCB;
};
