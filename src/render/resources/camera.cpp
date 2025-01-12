#include "camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

void sCamera::config_view(  const glm::vec3 cam_position, 
                            const glm::vec3 look_at, 
                            const glm::vec3 up  ) {
    position = cam_position;
    view_mat = glm::lookAt( cam_position, 
                            look_at, 
                            up  );

    view_proj_mat = proj_mat * view_mat;
}
void sCamera::config_projection(    const float rad_fov, 
                                    const float aspect_ratio, 
                                    const float near, 
                                    const float far) {
    proj_mat = glm::perspective(rad_fov, aspect_ratio, near, far);
    proj_mat[1][1] *= -1.0f;

    view_proj_mat = proj_mat * view_mat;
}

// https://terathon.com/lengyel/Lengyel-Oblique.pdf
//https://gist.github.com/fuqunaga/d87a0123efde61d0aa9cf9718c38a63c
void sCamera::config_oblique_projection(    const glm::vec4 clipping_plane, 
                                            const float rad_fov,
                                            const float aspect_ratio, 
                                            const float near, 
                                            const float far ) {
    proj_mat = glm::perspective(rad_fov, aspect_ratio, near, far);
    proj_mat[1][1] *= -1.0f;

    const glm::vec4 q = glm::inverse(proj_mat) * glm::vec4(glm::sign(clipping_plane.x), glm::sign(clipping_plane.y), 1.0f, 1.0f);

    const glm::vec4 c = clipping_plane * (2.0f / glm::dot(clipping_plane, q));

    proj_mat[0][2] = c.x - proj_mat[0][3];
    proj_mat[1][2] = c.y - proj_mat[1][3];
    proj_mat[2][2] = c.z - proj_mat[2][3];
    proj_mat[3][2] = c.w - proj_mat[3][3];

    view_proj_mat = proj_mat * view_mat;
}