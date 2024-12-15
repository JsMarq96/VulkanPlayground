#version 450
#extension GL_EXT_buffer_reference : require

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;

struct sVertex {
    vec3 position;
    float pad1;
    vec3 normal;
    float pad2;
    vec2 uv;
    vec2 pad3;
    vec3 tangent;
    float pad0;
    vec4 color;
};

layout(set = 0u, binding = 0u) uniform sSceneData {
    mat4 view;
    mat4 proj;
    mat4 view_proj;
    vec4 ambient_color;
    vec3 sunlight_dir;
    float sun_power;
    vec4 sunlight_color;
} scene_data;

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    sVertex vertices[];
};

layout(push_constant) uniform constants {
    mat4 model_matrix;
    VertexBuffer vertex_buffer;
} PushConstants;

void main() {
    sVertex v = PushConstants.vertex_buffer.vertices[gl_VertexIndex];

    gl_Position = scene_data.view_proj * PushConstants.model_matrix * vec4(v.position, 1.0f);
    out_color = v.color.xyz * 0.5 + 0.5;
    out_uv = v.uv;
}