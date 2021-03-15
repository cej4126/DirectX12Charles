struct LightBuf
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
ConstantBuffer <LightBuf> light: register(b1);

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

Texture2D tex : register(t0);
Texture2D spec : register(t1);
Texture2D nmap : register(t2);
SamplerState s1 : register(s0);

float4 main(float3 viewPos : Position, float3 n : Normal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
	// build the tranform (rotation) into tangent space
   const float3x3 tanToView = float3x3(
	    normalize(tan),
	    normalize(bitan),
	    normalize(n));

    // unpack normal data
    const float3 normalSample = nmap.Sample(s1, tc).xyz;
	 n = normalSample * 2.0f - 1.0f;
    // bring normal from tanspace into view space
    n = mul(n, tanToView);

	// fragment to light vector data
	const float3 vToL = light.lightPos - viewPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// attenuation
	const float att = 1.0f / (light.attConst + light.attLin * distToL + light.attQuad * (distToL * distToL));
	// diffuse intensity
	const float3 diffuse = light.diffuseColor * light.diffuseIntensity * att * max(0.0f, dot(dirToL, n));

	// reflected light vector
	const float3 w = n * dot(vToL, n);
	const float3 r = w * 2.0f - vToL;

	float3 specularReflectionColor;
	float specularPower = material.specularPower;
	if (material.hasSpecture)
	{
		const float4 specularSample = spec.Sample(s1, tc);
		// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
		specularReflectionColor = specularSample.rgb * material.specularWeight;
    	if (material.hasGloss)
	   {
	 	   specularPower = pow(2.0f, specularSample.a * 13.0f);
	   }
   }
	else
	{
	   specularReflectionColor = material.specularColor;
	}

	const float3 specular = att * (light.diffuseColor * light.diffuseIntensity) * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))),
		specularPower);
	// final color
	return float4(saturate((diffuse + light.ambient) * tex.Sample(s1, tc).rgb + specular * specularReflectionColor), 1.0f);
}
