struct CBuf
{
	float3 lightPos;
	float3 materialColor;
	float3 ambient;
	float3 diffuseColor;
	float diffuseIntensity;
	float attConst;
	float attLin;
	float attQuad;
};
ConstantBuffer <CBuf> buf: register(b1);


float4 main(float3 worldPos : Position, float3 n : Normal) : SV_Target
{
	// fragment to light vector data
	const float3 vToL = buf.lightPos - worldPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// diffuse attenuation
	const float att = 1.0f /(buf.attConst + buf.attLin * distToL + buf.attQuad * (distToL * distToL));
	// diffuse intensity
	const float3 diffuse = buf.diffuseColor * buf.diffuseIntensity * att * max(0.0f, dot(dirToL,n));
	// final color
	return float4(saturate((diffuse + buf.ambient) * buf.materialColor), 1.0f);
}
