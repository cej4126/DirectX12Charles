#include "Transform.hlsli"
ConstantBuffer<TransformType> transform : register(b0);

struct VS_INPUT
{
   float4 pos : POSITION;
   float2 texCoord: TEXCOORD;
};

struct VS_OUTPUT
{
   float4 pos: SV_POSITION;
   float2 texCoord: TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
   VS_OUTPUT output;
   output.pos = mul(input.pos, transform.modelView);
   output.texCoord = input.texCoord;
   return output;
}

