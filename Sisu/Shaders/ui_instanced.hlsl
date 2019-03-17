struct InstanceData
{
	float4x4 world;
	float4 uv;
};

StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);

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

Texture2D gMainTexture : register(t0, space0);
SamplerState gsamPointWrap : register(s0);

struct VertexIn
{
	float3 PosL  : POSITION;
	float2 PosUV : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float2 TexC  : TEXCOORD;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	InstanceData iData = gInstanceData[instanceID];
	float4 posW = mul(float4(vin.PosL, 1.0f), iData.world);
	vout.PosH = mul(posW, gViewProj);
	vout.TexC = iData.uv.xy + (iData.uv.zw * vin.PosUV);
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return gMainTexture.Sample(gsamPointWrap, pin.TexC);
}


