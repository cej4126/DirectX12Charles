
float3 MapNormal(
	const float3 tan,
	const float3 bitan,
	const float3 normal,
	const float2 tc,
	Texture2D nmap,
	SamplerState splr)
{
	// build the tranform (rotation) into same space as tan/bitan/normal (target space)
	const float3x3 tanToTarget = float3x3(tan, bitan, normal);

	// unpack the normal from map into target space        
	const float3 normalSample = nmap.Sample(splr, tc).xyz;
	float3 tanNormal = normalSample * 2.0f - 1.0f;
	tanNormal.y = -tanNormal.y;
	// bring normal from tanspace into target space
	return normalize(mul(tanNormal, tanToTarget));
}

float Attenuate(uniform float attConst, uniform float attLin, uniform float attQuad, const in float distFragToL)
{
	//return 1.0f / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
	return 1.0f / (light.attConst + light.attLin * distFragToL + light.attQuad * (distFragToL * distFragToL));
}

float3 Diffuse(
	uniform float3 diffuseColor,
	uniform float diffuseIntensity,
	const in float att,
	const in float3 viewDirFragToL,
	const in float3 viewNormal)
{
	return diffuseColor * diffuseIntensity * att * max(0.0f, dot(viewDirFragToL, viewNormal));
}

float3 Speculate(
	const in float3 specularColor,
	uniform float specularIntensity,
	const in float3 viewNormal,
	const in float3 viewFragToL,
	const in float3 viewPos,
	const in float att,
	const in float specularPower)
{
	// calculate reflected light vector
	const float3 w = viewNormal * dot(viewFragToL, viewNormal);
	const float3 r = normalize(w * 2.0f - viewFragToL);
	// vector from camera to fragment (in view space)
	const float3 viewCamToFrag = normalize(viewPos);
	// calculate specular component color based on angle between
	// viewing vector and reflection vector, narrow with power function
	return att * specularColor * specularIntensity * pow(max(0.0f, dot(-r, viewCamToFrag)), specularPower);
}
