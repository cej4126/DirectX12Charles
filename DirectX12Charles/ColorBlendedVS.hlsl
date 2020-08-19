//cbuffer ConstantBuffer : register(b0)
//{
//   float4x4 transform;
//};

struct Foo
{
   float4x4 transform;
};
ConstantBuffer<Foo> mydata : register(b0);

struct VS_OUTPUT
{
   float4 pos : SV_POSITION;
   float4 color : Color;
};

VS_OUTPUT main(float3 position : POSITION, float4 color : COLOR)
{
   VS_OUTPUT vso;
   vso.color = color;
   vso.pos = mul(float4(position, 1.0f), mydata.transform);
   //position[3] = 1.0f;
   //vso.pos = mul(position, mydata.transform);
   return vso;
}