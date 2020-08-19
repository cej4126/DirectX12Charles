struct Foo
{
   float4x4 transform;
};
ConstantBuffer<Foo> mydata : register(b0);

float4 main(float3 position : POSITION) : SV_Position
{
   return mul(float4(position.x, position.y, position.z, 1.0f), mydata.transform);
}

