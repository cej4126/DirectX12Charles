struct Foo
{
	// not sure which is model
	matrix modelViewProj;
	matrix modelView;
};
ConstantBuffer<Foo> mydata : register(b0);

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float2 tc : Texcoord;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 viewNormal : Normal, float2 tc : Texcoord)
{
	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), mydata.modelViewProj);
	vso.normal = mul(viewNormal, (float3x3)mydata.modelViewProj);
	vso.pos = mul(float4(pos, 1.0f), mydata.modelView);
	vso.tc = tc;
	return vso;
}