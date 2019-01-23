#pragma once
#include "D3DRenderer.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "Arena.h"
#include "GameObject.h"

class GameTimer;
class GameObject;

struct BrickVertex
{
	DirectX::XMFLOAT3 Pos;
};

class BrickRenderer : public D3DRenderer
{
public:
	static const UINT MaxInstancedObjectCount = 4096;

	BrickRenderer(WindowManager* windowManager, GameTimer* gameTimer, 
		Arena<GameObject>* bricks) :
		D3DRenderer(windowManager, gameTimer),
		_bricks (bricks)
	{
	}

	virtual bool Init() override;
	virtual void SetDirty() override { _dirtyFrameCount = FrameResourceCount; }
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;

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

	void DrawBricks(ID3D12GraphicsCommandList* cmdList);
	void UpdateCamera(const GameTimer& gt);
	void UpdateInstanceData();
	void UpdateMainPassCB(const GameTimer& gt);

	UINT AddToVertexBuffer(const GeometryGenerator::MeshData& mesh,
		std::vector<BrickVertex>& vertices,
		UINT startIndex,
		DirectX::XMFLOAT4 color) const;

	DirectX::XMMATRIX ToXMMatrix(const Sisu::Matrix4& matrix) const;

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

	Arena<GameObject>* _bricks;
	int _dirtyFrameCount = FrameResourceCount;
	bool _isWireframe;
	UINT _drawableObjectCount = 0;

	//Camera related
	float _theta = 1.5f * DirectX::XM_PI;
	float _phi = 0.2f * DirectX::XM_PI;
	float _radius = 15.0f;
};
