#include "Transform.hlsli"
ConstantBuffer<TransformType> transform : register(b0);


float4 main(float3 pos : Position) : SV_Position
{
   return mul(float4(pos,1.0f), transform.modelView);
}