#include "PointLight.hlsli"
ConstantBuffer <PointLightType> light: register(b1);

#include "Material.hlsli"
ConstantBuffer <MaterialType> material: register(b2);

float4 main(float3 viewPos : Position, float3 viewNormal : Normal) : SV_Target
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
	const float4 specular = att * (float4(light.diffuseColor, 1.0f) * light.diffuseIntensity) *
		material.specularColor * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), material.specularPower);

	// final color
	return saturate(float4(diffuse + light.ambient, 1.0f) * material.materialColor + specular);
}
