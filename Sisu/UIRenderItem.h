#pragma once
#include "MathHelper.h"
#include "d3dUtil.h"

struct UIRenderItem
{
	UIRenderItem() = default;
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

	//TODO = NumFramesDirty should equal to renderer's FrameResourceCount,
	//		 but FFS dependencies
	int NumFramesDirty = 3;
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