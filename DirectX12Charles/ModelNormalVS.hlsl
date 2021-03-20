#include "Transform.hlsli"
ConstantBuffer<TransformType> transform : register(b0);

struct VS_OUTPUT
{
   float3 ViewPos : Position;
   float3 normal : Normal;
   float4 pos : SV_Position;
};

VS_OUTPUT main(float3 position : POSITION, float3 viewNormal : Normal)
{
   VS_OUTPUT vso;
   vso.ViewPos = (float3)mul(float4(position, 1.0f), transform.modelViewProj);
   vso.normal = mul(viewNormal, (float3x3)transform.modelViewProj);
   vso.pos = mul(float4(position, 1.0f), transform.modelView);
   return vso;
}
