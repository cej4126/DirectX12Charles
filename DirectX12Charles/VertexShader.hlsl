struct PSInput
{
   float4 position : SV_POSITION;
   float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
   float4x4 transform;
};

PSInput main(float4 position : POSITION, float4 color : COLOR)
{
   PSInput output;
   output.position = mul(float4(position.x, position.y, 0.0f, 1.0f), transform);
   output.color = color;
   return output;
}