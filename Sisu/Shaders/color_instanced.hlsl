struct InstanceData
{
	float4x4 world;
	float4 color;
	float4 borderColor;
	float3 localScale;
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
	float3 TexCoord : TEXCOORD0;
	float3 LocScale : TEXCOORD1;
	float4 Color : COLOR;
	float4 BorderColor : TEXCOORD2;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout;
	float4 posW = mul(float4(vin.PosL, 1.0f), gInstanceData[instanceID].world);
	vout.PosH = mul(posW, gViewProj);
	vout.Color = gInstanceData[instanceID].color;
	vout.TexCoord = vin.PosL;
	vout.LocScale = gInstanceData[instanceID].localScale;
	vout.BorderColor = gInstanceData[instanceID].borderColor;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float isLeftOrRight = step(0.5 - (0.025 / pin.LocScale.x), abs(pin.TexCoord.x));
	float isTopOrBottom = step(0.5 - (0.025 / pin.LocScale.y), abs(pin.TexCoord.y));
	float isFrontOrBack = step(0.5 - (0.025 / pin.LocScale.z), abs(pin.TexCoord.z));
	
	return lerp(pin.Color, pin.BorderColor, step(2.0, isLeftOrRight + isTopOrBottom + isFrontOrBack));
}
