#version 450

layout(set = 0, binding = 0) uniform view_projection_t
{
    mat4 view;
    mat4 projection;
    vec3 camPos;
} view_proj;

layout(set = 1, binding = 0) uniform model_t
{
    mat4 model;
} model;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vTexCoord;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec4 vTangent;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out mat3 fragTangentMatrix;
layout(location = 5) out vec3 camPos;

void main() {
    gl_Position = view_proj.projection * view_proj.view * model.model * vec4(vPosition, 1.0);
    fragPos = vec3(model.model * vec4(vPosition, 1.0));
    fragTexCoord = vTexCoord.xy;
    fragTexCoord.y = 1.0 - fragTexCoord.y;

    mat3 smallModel = mat3(model.model);
    mat3 antimodel = determinant(smallModel) * inverse(transpose(smallModel));

    vec3 tangent = normalize(smallModel * vTangent.xyz);
    vec3 normal = normalize(antimodel * vNormal);
    vec3 bitangent = normalize(vTangent.w * cross(normal, tangent));

    fragTangentMatrix = mat3(tangent, bitangent, normal);

    camPos = view_proj.camPos;
}