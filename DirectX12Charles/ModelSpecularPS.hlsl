struct CBuf
{
	float3 lightPos;
	float pad1;
	float3 ambient;
	float pad3;
	float3 diffuseColor;
	float pad4;
	float diffuseIntensity;
	float attConst;
	float attLin;
	float attQuad;
};
ConstantBuffer <CBuf> buf: register(b1);

struct MaterialBuf
{
	float3 materialColor;
	float specularIntensity;
	float specularPower;
	float pad[2];
};
ConstantBuffer <MaterialBuf> material: register(b2);

Texture2D tex : register(t0);
Texture2D spec : register(t1);
SamplerState s1 : register(s0);

float4 main(float3 worldPos : Position, float3 n : Normal, float2 tc : Texcoord) : SV_Target
{
	// fragment to light vector data
	const float3 vToL = buf.lightPos - worldPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// attenuation
	const float att = 1.0f / (buf.attConst + buf.attLin * distToL + buf.attQuad * (distToL * distToL));
	// diffuse intensity
	const float3 diffuse = buf.diffuseColor * buf.diffuseIntensity * att * max(0.0f, dot(dirToL, n));

	// reflected light vector
	const float3 w = n * dot(vToL, n);
	const float3 r = w * 2.0f - vToL;

	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
	const float4 specularSample = spec.Sample(s1, tc);
	const float3 specularReflectionColor = specularSample.rgb;
	const float specularPower = pow(2.0f, specularSample.a * 13.0f);
	const float3 specular = att * (buf.diffuseColor * buf.diffuseIntensity) * pow(max(0.0f, dot(normalize(-r), normalize(worldPos))),
		specularPower);
	// final color
	return float4(saturate((diffuse + buf.ambient) * tex.Sample(s1, tc).rgb + specular * specularReflectionColor), 1.0f);
}
