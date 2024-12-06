#version 450
#extension GL_EXT_buffer_reference : require

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;

struct sVertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
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
    out_uv = vec2(v.uv_x, v.uv_y);
}