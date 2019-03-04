cbuffer cbPerObject : register(b1)
{
	float4x4 gWorld; 
};

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

Texture2D gMainTexture : register(t0);
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

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosH = mul(posW, gViewProj);
	vout.TexC = vin.PosUV;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	//return float4(pin.TexC.x, 0.0f, 0.0f, 1.0f);
	return gMainTexture.Sample(gsamPointWrap, pin.TexC);
	//return float4(1.0, 0.5, 0.5, 1.0);
}


