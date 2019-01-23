struct InstanceData
{
	float4x4 world;
	float4 color;
};

StructuredBuffer<InstanceData> gInstanceData : register(t0);

cbuffer cbPass : register(b0)
{
	float4x4 gView;
    	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float3 gEyePosW;
	float cbPerObjectPad1;
	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;	
};

struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout;
	float4 posW = mul(float4(vin.PosL, 1.0f), gInstanceData[instanceID].world);
	vout.PosH = mul(posW, gViewProj);
	vout.Color = gInstanceData[instanceID].color;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return pin.Color;
}
