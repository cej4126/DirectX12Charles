#include "Transform.hlsli"
ConstantBuffer<TransformType> transform : register(b0);

float4 main(float3 position : POSITION) : SV_Position
{
	return mul(float4(position, 1.0f), transform.modelView);
}
