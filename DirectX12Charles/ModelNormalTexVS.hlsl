#include "Transform.hlsli"
ConstantBuffer<TransformType> transform : register(b0);

struct VSOut
{
	float3 viewPos : Position;
	float3 normal : Normal;
	float3 tan : Tangent;
	float3 bitan : Bitangent;
	float2 tc : Texcoord;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 viewNormal : Normal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord)
{
	VSOut vso;
	vso.viewPos = (float3)mul(float4(pos, 1.0f), transform.modelViewProj);
	vso.normal = mul(viewNormal, (float3x3)transform.modelViewProj);
	vso.tan = mul(tan, (float3x3) transform.modelViewProj);
	vso.bitan = mul(bitan, (float3x3) transform.modelViewProj);
	vso.pos = mul(float4(pos, 1.0f), transform.modelView);
	vso.tc = tc;
	return vso;
}