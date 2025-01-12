#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

struct sCamera {
    glm::vec3 position = {};

    bool is_dirty = false;

    glm::mat4x4 view_mat;
    glm::mat4x4 proj_mat;
    glm::mat4x4 view_proj_mat;

    void config_view(const glm::vec3 position, const glm::vec3 look_at, const glm::vec3 up = {0.0f, 0.0f, 1.0f});
    void config_projection(const float rad_fov, const float aspect_ratio, const float near = 0.01f, const float far = 10.0f);
    void config_oblique_projection(const glm::vec4 clipping_plane, const float rad_fov, const float aspect_ratio, const float near = 0.01f, const float far = 10.0f);
};