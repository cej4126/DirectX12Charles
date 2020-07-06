cbuffer ConstantBuffer : register(b0)
{
   float4x4 transform;
};

float4 main(float3 position : POSITION) : SV_Position
{
   return mul(float4(position.x, position.y, position.z, 1.0f), transform);
}