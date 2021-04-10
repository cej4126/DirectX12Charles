#include "PointLight.hlsli"
ConstantBuffer <PointLightType> light: register(b1);

#include "Material.hlsli"
ConstantBuffer <MaterialType> material: register(b2);

#include "LightVectorData.hlsli"
#include "ShaderOps.hlsli"

float4 main(float3 viewPos : Position, float3 viewNormal : Normal) : SV_Target
{
	// renormalize interpolated normal
   viewNormal = normalize(viewNormal);

   // fragment to light vector data
   const LightVectorData lv = CalculateLightVectorData(light.viewLightPos, viewPos);
   // attenuation
   const float att = Attenuate(light.attConst, light.attLin, light.attQuad, lv.distToL);
   // diffuse intensity
   const float3 diffuse = Diffuse(light.diffuseColor, light.diffuseIntensity, att, lv.dirToL, viewNormal);
   // specular
   const float3 specular = Speculate(light.diffuseColor, light.diffuseIntensity, viewNormal, lv.vToL, viewPos, att, material.specularPower);

   // final color
   return float4(saturate((diffuse + light.ambient) * material.materialColor.rgb + specular), 1.0f);
}
