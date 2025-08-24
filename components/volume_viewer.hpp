#pragma once

#include <GLFW/glfw3.h>

#include <optional>

#include "data_models/frame3d.h"
#include "view_models/scale.hpp"

namespace {

void
setGLAlphaBlending(const float alpha, const float threshold = 0.03f) {
    constexpr bool is_clip_plane = false;
    if constexpr (is_clip_plane) {
        glEnable(GL_CLIP_PLANE0);
    } else {
        glDisable(GL_CLIP_PLANE0);
    }

    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glAlphaFunc(GL_GREATER, threshold);
    glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);

    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    glBlendColor(1.0f, 1.0f, 1.0f, alpha);
    glBlendEquation(GL_ADD);
}

/** volume render using a single 3D texture */
void
drawGL3D(const view_models::Frame3D& volume, float scale, types::Orientation o, float quality) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(0.5f, 0.5f, 0.5f);
    glScalef(1.0f / scale, 1.0f / scale, 1.0f / scale);
    glRotatef(90, 1, 0, 0);

    o.normalize();
    glRotatef(o.azimuth, 0, 1, 0);
    glRotatef(-o.elevation, 1, 0, 0);

    glTranslatef(-0.5f, -0.5f, -0.5f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volume.texture);
    glEnable(GL_TEXTURE_3D);
    const auto [x, y, z] = volume.dim;
    const auto render_quality = 1.7f * 1.0f / (x + y + z) * quality;
    for (auto fz = -0.5f; fz <= 0.5f; fz += render_quality) {
        const float tz = fz + 0.5f;
        const float vz = (fz * 2.0f) - 0.2f;

        glBegin(GL_QUADS);
        glTexCoord3f(0.0f, 0.0f, tz);
        glVertex3f(-1.0f, -1.0f, vz);
        glTexCoord3f(1.0f, 0.0f, tz);
        glVertex3f(1.0f, -1.0f, vz);
        glTexCoord3f(1.0f, 1.0f, tz);
        glVertex3f(1.0f, 1.0f, vz);
        glTexCoord3f(0.0f, 1.0f, tz);
        glVertex3f(-1.0f, 1.0f, vz);
        glEnd();
    }
}

}  // namespace

namespace components {

struct VolumeViewer {
    static inline float volume_step_size{1.0f};
    static inline float alpha{0.02f};
    static inline types::Orientation orientation{};
    static inline std::optional<view_models::Frame3D> volume{std::nullopt};

    static void render() {
        using view_models::scale;
        if (!volume) {
            return;
        }

        // glDisable(GL_DEPTH_TEST);
        // glDisable(GL_ALPHA_TEST);
        glDisable(GL_LIGHTING);
        setGLAlphaBlending(alpha);
        drawGL3D(*volume, scale, orientation, volume_step_size);
    }
};
}  // namespace components