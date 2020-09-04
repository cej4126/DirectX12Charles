struct Foo
{
   matrix modelViewProj;
   matrix model;
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
   vso.pos = mul(float4(position, 1.0f), mydata.model);
   return vso;
}
