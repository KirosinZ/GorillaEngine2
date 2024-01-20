#version 460 core

float PI = 3.1415926535;

struct point_light_t
{
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct spot_light_t
{
    vec3 position;
    vec3 direction;
    float intensity;
    vec3 color;
    float cone_size;

    float constant;
    float linear;
    float quadratic;
};

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in mat3 fragTangentMatrix;
layout(location = 5) in vec3 camPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform point
{
    point_light_t lights[10];
    int n_lights;
} point_lights;

layout(set = 0, binding = 2) uniform spot
{
    spot_light_t lights[10];
    int n_lights;
} spot_lights;

layout(set = 1, binding = 1) uniform sampler2D albedoMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D roughnessMap;
layout(set = 1, binding = 4) uniform sampler2D metallicMap;
layout(set = 1, binding = 5) uniform sampler2D aoMap;

float attenuate_point_light(point_light_t light, float dist)
{
    return 1.0 / (light.constant + dist * light.linear + dist * dist * light.quadratic);
}

float attenuate_spot_light(spot_light_t light, float dist)
{
    return 1.0 / (light.constant + dist * light.linear + dist * dist * light.quadratic);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float den = (NdotH2 * (a2 - 1.0) + 1.0);
    den = PI * den * den;

    return num / den;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = r * r / 8.0;

    float num = NdotV;
    float den = NdotV * (1.0 - k) + k;

    return num / den;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{
//    vec4 albedo = texture(albedoMap, fragTexCoord);
//    outColor = albedo;
//
//    vec3 N = normalize(fragTangentMatrix[2]);
//    vec3 V = normalize(camPos - fragPos);
//
//    point_light_t light = point_lights.lights[0];
//
//    vec3 L = normalize(light.position - fragPos);
//
//    vec3 R = reflect(-L, N);
//
//    float Aq = 0.01;
//    float Dq = max(0.0, dot(L, N));
//    float Sq = pow(max(0.0, dot(R, V)), 12.0);
//
//    outColor = vec4(vec3(Aq + Dq / 2.0 + Sq) * albedo.xyz * light.intensity, 1.0);
//
//    return;

    vec3 tangent = fragTangentMatrix[0];
    vec3 bitangent = fragTangentMatrix[1];
    vec3 normal = fragTangentMatrix[2];

    mat3 matrix = fragTangentMatrix;

    vec3 sampled_normal = texture(normalMap, fragTexCoord).rgb;

    vec3 actual_normal = normalize(matrix * (sampled_normal * 2.0 - 1.0));
    vec4 albedo = texture(albedoMap, fragTexCoord);
    float roughness = texture(roughnessMap, fragTexCoord).r;
    float metallic = texture(metallicMap, fragTexCoord).r;
    float ao = texture(aoMap, fragTexCoord).r;

    vec3 N = normalize(actual_normal);
    vec3 V = normalize(camPos - fragPos);

    vec3 col = vec3(0.6, 0.6, 0.6);
    float g = max(0.0, dot(N, V));

    outColor = vec4(g * col, 1.0);


    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.xyz, vec3(metallic));

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < point_lights.n_lights; i++)
    {
        point_light_t light = point_lights.lights[i];
        vec3 L = normalize(light.position - fragPos);
        vec3 H = normalize(V + L);

        float distance = length(light.position - fragPos);
        float attenuation = attenuate_point_light(light, distance);
        vec3 radiance = light.intensity * light.color * attenuation;

        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);

        vec3 num = NDF * G * F;
        float den = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = num / den;

        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo.xyz / PI + specular) * radiance * NdotL;
    }

    for (int i = 0; i < spot_lights.n_lights; i++)
    {
        spot_light_t light = spot_lights.lights[i];
        vec3 L = normalize(light.position - fragPos);
        vec3 H = normalize(V + L);

        float DdotL = dot(-L, normalize(light.direction));
        if (DdotL < light.cone_size - 0.05)
                continue;

        float distance = length(light.position - fragPos);
        float attenuation = attenuate_spot_light(light, distance);
        float falloff = smoothstep(light.cone_size - 0.05, light.cone_size, DdotL);
        vec3 radiance = light.intensity * light.color * attenuation * falloff;

        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);

        vec3 num = NDF * G * F;
        float den = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = num / den;

        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo.xyz / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.003) * albedo.xyz * ao;
    vec3 color = ambient + Lo;

    color = color / (1.0 + color);
    color = pow(color, vec3(1.0 / 2.2));
    outColor = vec4(color, 1.0);

}