#pragma once
#include "D3DRenderer.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "Arena.h"
#include "GameObject.h"
#include "ICameraService.h"

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

	BrickRenderer(WindowManager* const windowManager, 
		GameTimer* const gameTimer, 
		Arena<GameObject>* const bricks,
		ICameraService* const cameraService) :
		D3DRenderer(windowManager, gameTimer, cameraService),
		_bricks (bricks)
	{
	}

	virtual bool Init() override;
	virtual void SetDirty() override { _dirtyFrameCount = FrameResourceCount; }
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;
	virtual void SetWireframe(bool state) override { _isWireframe = state; }

private:
	void BuildDescriptorHeaps();
	void BuildConstantBufferViews();
	void BuildShapeGeometry();
	void BuildFrameResources();			// This is what builds the constant buffers
	void BuildRootSignatures();
	void BuildShadersAndInputLayout();
	void BuildPSOs();

	void DrawBricks(ID3D12GraphicsCommandList* cmdList);
	void UpdateInstanceData();
	void UpdateMainPassCB(const GameTimer& gt, const D3DCamera& activeCamera, int index);

	UINT AddToVertexBuffer(const GeometryGenerator::MeshData& mesh,
		std::vector<BrickVertex>& vertices,
		UINT startIndex,
		DirectX::XMFLOAT4 color) const;

	DirectX::XMMATRIX ToXMMatrix(const Sisu::Matrix4& matrix) const;

private:
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
};
