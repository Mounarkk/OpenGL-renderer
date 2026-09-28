// Cook-Torrance microfacet BRDF, metallic-roughness parameterization.
// References: Karis 2013 "Real Shading in Unreal Engine 4", Lagarde and
// de Rousiers 2014 "Moving Frostbite to PBR".

const float PI = 3.14159265359;

// Everything a light needs to know about the shaded point
struct Surface {
    vec3 position;
    vec3 normal;
    vec3 diffuseColor; // albedo of the non-metallic part, black for metals
    vec3 f0;           // reflectance at normal incidence
    float roughness;   // perceptual roughness, squared for the lobes
    float occlusion;   // ambient occlusion, only applied to the ambient term
};

// GGX / Trowbridge-Reitz normal distribution: how many microfacets face h
float distributionGGX(float NdotH, float alpha)
{
    float a2 = alpha * alpha;
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

// Height-correlated Smith masking-shadowing, already divided by the
// 4 * NdotL * NdotV denominator of the microfacet model
float visibilitySmithGGX(float NdotV, float NdotL, float alpha)
{
    float a2 = alpha * alpha;
    float ggxV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    float ggxL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);
    return 0.5 / max(ggxV + ggxL, 1e-5);
}

// Schlick approximation of Fresnel: reflection grows at grazing angles
vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

// Outgoing radiance towards the viewer for light arriving from lightDir
// with the given radiance (already attenuated).
vec3 shadeSurface(Surface s, vec3 lightDir, vec3 viewDir, vec3 radiance)
{
    vec3 h = normalize(lightDir + viewDir);
    float NdotL = max(dot(s.normal, lightDir), 0.0);
    if (NdotL <= 0.0)
        return vec3(0.0);
    float NdotV = max(dot(s.normal, viewDir), 1e-4);
    float NdotH = max(dot(s.normal, h), 0.0);
    float VdotH = max(dot(viewDir, h), 0.0);

    // Very low roughness makes the highlight of punctual lights vanish
    float alpha = max(s.roughness * s.roughness, 0.002);

    vec3 F = fresnelSchlick(VdotH, s.f0);
    vec3 specular = distributionGGX(NdotH, alpha) *
                    visibilitySmithGGX(NdotV, NdotL, alpha) * F;
    // Light reflected by the specular lobe is not available for diffusion
    vec3 diffuse = (1.0 - F) * s.diffuseColor / PI;

    return (diffuse + specular) * radiance * NdotL;
}
