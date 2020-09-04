struct Foo
{
	// not sure which is model
	matrix modelViewProj;
	matrix model;
};
ConstantBuffer<Foo> mydata : register(b0);

float4 main(float3 position : POSITION) : SV_Position
{
	return mul(float4(position, 1.0f), mydata.model);
}
