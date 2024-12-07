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

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    sVertex vertices[];
};

layout(push_constant) uniform constants {
    mat4 mvp_matrix;
    VertexBuffer vertex_buffer;
} PushConstants;

void main() {
    sVertex v = PushConstants.vertex_buffer.vertices[gl_VertexIndex];

    gl_Position = PushConstants.mvp_matrix * vec4(v.position, 1.0f);
    out_color = v.color.xyz;
    out_uv = v.uv;
}