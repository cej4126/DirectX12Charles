struct Foo
{
	// not sure which is model
	matrix modelViewProj;
	matrix model;
};
ConstantBuffer<Foo> mydata : register(b0);

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal)
{
	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), mydata.modelViewProj);
	vso.normal = mul(n, (float3x3)mydata.modelViewProj);
	vso.pos = mul(float4(pos, 1.0f), mydata.model);
	return vso;
}