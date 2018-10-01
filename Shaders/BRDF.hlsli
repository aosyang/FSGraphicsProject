//=============================================================================
// BRDF.hlsli by Shiyang Ao, 2018 All Rights Reserved.
//
// Source: https://github.com/Microsoft/Xbox-ATG-Samples/blob/master/Kits/DirectXTK/Src/Shaders/PBRCommon.fxh
//=============================================================================

#ifndef _BRDF_HLSLI
#define _BRDF_HLSLI

static const float PI = 3.14159265f;
static const float EPSILON = 1e-6f;

// Shlick's approximation of Fresnel
// https://en.wikipedia.org/wiki/Schlick%27s_approximation
float Fresnel_Shlick(float3 f0, float3 f90, float x)
{
	return f0 + (f90 - f0) * pow(1.0f - x, 5.0f);
}

// Burley B. "Physically Based Shading at Disney"
// SIGGRAPH 2012 Course: Practical Physically Based Shading in Film and Game Production, 2012.
float Diffuse_Burley(float NdotL, float NdotV, float LdotH, float roughness)
{
	float fd90 = 0.5f + 2.0f * roughness * LdotH * LdotH;
	return Fresnel_Shlick(1, fd90, NdotL).x * Fresnel_Shlick(1, fd90, NdotV).x;
}

// GGX specular D (normal distribution)
// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
float Specular_D_GGX(float alpha, float NdotH)
{
	const float alpha2 = alpha * alpha;
	const float lower = (NdotH * NdotH * (alpha2 - 1)) + 1;
	return alpha2 / max(EPSILON, PI * lower * lower);
}

// Schlick-Smith specular G (visibility) with Hable's LdotH optimization
// http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
// http://graphicrants.blogspot.se/2013/08/specular-brdf-reference.html
float G_Shlick_Smith_Hable(float alpha, float LdotH)
{
	return rcp(lerp(LdotH * LdotH, 1, alpha * alpha * 0.25f));
}

// A microfacet based BRDF.
//
// alpha:           This is roughness * roughness as in the "Disney" PBR model by Burley et al.
//
// specularColor:   The F0 reflectance value - 0.04 for non-metals, or RGB for metals. This follows model 
//                  used by Unreal Engine 4.
//
// NdotV, NdotL, LdotH, NdotH: vector relationships between,
//      N - surface normal
//      V - eye normal
//      L - light normal
//      H - half vector between L & V.
float3 Specular_BRDF(float alpha, float3 specularColor, float NdotV, float NdotL, float LdotH, float NdotH)
{
	// Specular D (microfacet normal distribution) component
	float specular_D = Specular_D_GGX(alpha, NdotH);

	// Specular Fresnel
	float specular_F = Fresnel_Shlick(specularColor, 1, LdotH);

	// Specular G (visibility) component
	float specular_G = G_Shlick_Smith_Hable(alpha, LdotH);

	return specular_D * specular_F * specular_G;
}

// Diffuse irradiance
float3 Diffuse_IBL(TextureCube IrradianceTexture, float3 N)
{
	return MakeLinearColorFromGammaSpace(IrradianceTexture.Sample(Sampler, N)).rgb;
}

// Approximate specular image based lighting by sampling radiance map at lower mips 
// according to roughness, then modulating by Fresnel term. 
float3 Specular_IBL(TextureCube RadianceTexture, float3 N, float3 V, float lodBias)
{
	const static int NumRadianceMipLevels = 2;

	float mip = lodBias * NumRadianceMipLevels;
	float3 dir = reflect(-V, N);
	return MakeLinearColorFromGammaSpace(RadianceTexture.SampleLevel(Sampler, dir, mip)).rgb;
}

#endif	// _BRDF_HLSLI
