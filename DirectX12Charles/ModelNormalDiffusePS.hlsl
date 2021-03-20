#include "PointLight.hlsli"
ConstantBuffer <PointLightType> light: register(b1);

#include "Material.hlsli"
ConstantBuffer <MaterialType> material: register(b2);

Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float2 tc : Texcoord) : SV_Target
{
	// renormalize interpolated normal
   viewNormal = normalize(viewNormal);

	// fragment to light vector data
	const float3 vToL = light.viewLightPos - viewPos;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// attenuation
	const float att = 1.0f / (light.attConst + light.attLin * distToL + light.attQuad * (distToL * distToL));
	// diffuse intensity
	const float3 diffuse = light.diffuseColor * light.diffuseIntensity * att * max(0.0f, dot(dirToL, viewNormal));

	// reflected light vector
	const float3 w = viewNormal * dot(vToL, viewNormal);
	const float3 r = w * 2.0f - vToL;
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
	const float3 specular = att * (light.diffuseColor * light.diffuseIntensity) *
		material.specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), material.specularPower);

	// final color
	return float4(saturate((diffuse + light.ambient) * t1.Sample(s1, tc).rgb + specular), 1.0f);
}
