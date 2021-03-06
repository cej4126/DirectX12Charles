#include "Transform.hlsli"
ConstantBuffer<TransformType> transform : register(b0);

struct VSOut
{
	float3 viewPos : Position;
	float3 normal : Normal;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 viewNormal : Normal)
{
	VSOut vso;
	vso.viewPos = (float3)mul(float4(pos, 1.0f), transform.modelViewProj);
	vso.normal = mul(viewNormal, (float3x3)transform.modelViewProj);
	vso.pos = mul(float4(pos, 1.0f), transform.modelView);
	return vso;
}