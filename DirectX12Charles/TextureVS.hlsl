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

struct Foo
{
   // It is backward
   matrix modelViewProj;
   matrix modelView;
};
ConstantBuffer<Foo> mydata : register(b0);

VS_OUTPUT main(VS_INPUT input)
{
   VS_OUTPUT output;
   output.pos = mul(input.pos, mydata.modelView);
   output.texCoord = input.texCoord;
   return output;
}

