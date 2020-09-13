struct Foo
{
   matrix modelViewProj;
   matrix modelView;
};
ConstantBuffer<Foo> mydata : register(b0);

float4 main(float3 pos : Position) : SV_Position
{
   //return mul(float4(pos,1.0f), mydata.modelViewProj);
   return mul(float4(pos,1.0f), mydata.modelView);
}