//struct LightBuf
//{
//	float3 viewLightPos;
//	float pad1;
//	float3 ambient;
//	float pad3;
//	float3 diffuseColor;
//	float pad4;
//	float diffuseIntensity;
//	float attConst;
//	float attLin;
//	float attQuad;
//};
#include "PointLight.hlsli"

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

#include "ShaderOps.hlsli"

Texture2D tex : register(t0);
Texture2D spec : register(t1);
Texture2D nmap : register(t2);
SamplerState s1 : register(s0);

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
	// normalize the mesh normal
	viewNormal = normalize(viewNormal);

   if (material.hasNormal)
	{
		viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, nmap, s1);
	}

	// fragment to light vector data
	const float3 viewFragToL = light.viewLightPos - viewPos;
	const float distFragToL = length(viewFragToL);
	const float3 viewDirFragToL = viewFragToL / distFragToL;

	// reflected light vector
	//const float3 w = viewNormal * dot(viewFragToL, viewNormal);
	//const float3 r = w * 2.0f - viewFragToL;

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
	   specularReflectionColor = material.specularColor.rgb;
	}

	// attenuation
	const float att = Attenuate(light.attConst, light.attLin, light.attQuad, distFragToL);
	// diffuse intensity
	const float3 diffuse = Diffuse(light.diffuseColor, light.diffuseIntensity, att, viewDirFragToL, viewNormal);

	//const float3 specular = att * (light.diffuseColor * light.diffuseIntensity) * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))),
	//	specularPower);
		 // specular reflected
	const float3 specularReflected = Speculate(
		specularReflectionColor, 1.0f, viewNormal,
		viewFragToL, viewPos, att, specularPower);

	// final color
	return float4(saturate((diffuse + light.ambient) * tex.Sample(s1, tc).rgb + specularReflected), 1.0f);
}
