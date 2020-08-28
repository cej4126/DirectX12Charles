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
   float4x4 transform;
};
ConstantBuffer<Foo> mydata : register(b0);

VS_OUTPUT main(VS_INPUT input)
{
   VS_OUTPUT output;
   output.pos = mul(input.pos, mydata.transform);
   output.texCoord = input.texCoord;
   return output;
}

