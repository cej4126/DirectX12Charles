struct CBuf
{
	float3 viewLightPos;
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
	float4 materialColor;
	float specularIntensity;
	float specularPower;
	int hasNormal;
	int hasGloss;
	int hasSpecture;
	float4 specularColor;
	float specularWeight;
};
ConstantBuffer <MaterialBuf> material: register(b2);

float4 main(float3 viewPos : Position, float3 viewNormal : Normal) : SV_Target
{
	// renormalize interpolated normal
   viewNormal = normalize(viewNormal);

	// fragment to light vector data
	const float3 vToL = buf.viewLightPos - viewPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// attenuation
	const float att = 1.0f / (buf.attConst + buf.attLin * distToL + buf.attQuad * (distToL * distToL));
	// diffuse intensity
	const float3 diffuse = buf.diffuseColor * buf.diffuseIntensity * att * max(0.0f, dot(dirToL, viewNormal));

	// reflected light vector
	const float3 w = viewNormal * dot(vToL, viewNormal);
	const float3 r = w * 2.0f - vToL;
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
	const float4 specular = att * (float4(buf.diffuseColor, 1.0f) * buf.diffuseIntensity) *
		material.specularColor * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), material.specularPower);

	// final color
	return saturate(float4(diffuse + buf.ambient, 1.0f) * material.materialColor + specular);
}
