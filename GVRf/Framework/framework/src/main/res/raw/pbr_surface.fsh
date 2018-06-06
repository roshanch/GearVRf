@MATERIAL_UNIFORMS

const float M_PI = 3.141592653589793;
const float c_MinRoughness = 0.04;

#ifdef HAS_ambientTexture
layout(location = 11) in vec2 ambient_coord;
#endif

#ifdef HAS_specularTexture
layout(location = 12) in vec2 specular_coord;
#endif

#ifdef HAS_emissiveTexture
layout(location = 13) in vec2 emissive_coord;
#endif

#ifdef HAS_lightMapTexture
layout(location = 14) in vec2 lightmap_coord;
#endif

#ifdef HAS_opacityTexture
layout(location = 15) in vec2 opacity_coord;
#endif

#ifdef HAS_normalTexture
layout(location = 16) in vec2 normal_coord;
layout(location = 4) in mat3 tangent_matrix;
#endif

layout(set = 0, binding = 10) uniform sampler2D diffuseTexture;
layout(set = 0, binding = 11) uniform sampler2D ambientTexture;
layout(set = 0, binding = 12) uniform sampler2D specularTexture;
layout(set = 0, binding = 13) uniform sampler2D emissiveTexture;
layout(set = 0, binding = 14) uniform sampler2D lightmapTexture;
layout(set = 0, binding = 15) uniform sampler2D opacityTexture;
layout(set = 0, binding = 16) uniform sampler2D normalTexture;

struct Surface
{
    vec4 diffuse;               // color contribution from diffuse lighting
    vec3 specular;              // color contribution from specular lighting
    vec3 emission;              // emitted light color
    vec3 viewspaceNormal;       // normal in view space
    vec2 brdf;                  // reflectance at 0 and 90
    float roughness;            // roughness value, as authored by the model creator (input to shader)
};

vec3 SRGBtoLINEAR(vec3 srgbIn)
{
    //fast srgb approximation
    return pow(srgbIn.xyz, vec3(2.2));
}

Surface @ShaderName()
{

    float perceptualRoughness;
    vec3 diffuse;
    vec3 specular;
    vec3 emission = emissive_color.xyz;

    //specular glossiness workflow
    #ifdef HAS_glossinessFactor
        diffuse = diffuse_color.rgb;
        specular = specular_color.rgb;
        float glossiness = glossinessFactor;

        #ifdef HAS_specularTexture
                specular.xyz *= SRGBtoLINEAR(texture(specularTexture, specular_coord.xy).rgb);
                glossiness *= texture(specularTexture, specular_coord.xy).a;
        #endif

        #ifdef HAS_diffuseTexture
                diffuse.xyz *= SRGBtoLINEAR(texture(diffuseTexture, diffuse_coord.xy).rgb);
        #endif
        perceptualRoughness = 1.0f - glossiness;

    #else
        //metallic roughness workflow
        vec4 basecolor = diffuse_color;
        float metal = metallic;
        perceptualRoughness = roughness;
        #ifdef HAS_metallicRoughnessTexture
                // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
                // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
                vec4 mrSample = texture(metallicRoughnessTexture, metallicRoughness_coord.xy);
                perceptualRoughness = mrSample.g * perceptualRoughness;
                metal = mrSample.b * metal;
        #endif
        metal = clamp(metal, 0.0, 1.0);

        #ifdef HAS_diffuseTexture
                basecolor.rgb *= SRGBtoLINEAR(texture(diffuseTexture, diffuse_coord.xy).rgb);
        #endif

        vec3 f0 = vec3(0.04);
        diffuse = basecolor.rgb * (vec3(1.0) - f0);
        diffuse *= 1.0 - metal;
        specular = mix(f0, basecolor.rgb, metal);
    #endif

    perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);

    // Compute reflectance (BRDF)
    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
    // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.

    float reflectance = max(max(specular.r, specular.g), specular.b);
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec2 brdf = vec2(reflectance, reflectance90);
    vec3 viewspaceNormal;

    #ifdef HAS_emissiveTexture
        emission = SRGBtoLINEAR(texture(emissiveTexture, emissive_coord.xy).rgb);
    #endif
    #if defined(HAS_normalTexture) && defined(HAS_a_tangent)
        viewspaceNormal = texture(normalTexture, normal_coord.xy).xyz * 2.0 - 1.0;
        viewspaceNormal = normalize(tangent_matrix * (viewspaceNormal * vec3(normalScale, normalScale, 1.0)));
    #else
        viewspaceNormal = viewspace_normal;
    #endif

    return Surface(vec4(diffuse, diffuse_color.a), specular, emission,
                   viewspaceNormal, brdf,
                   perceptualRoughness);
}