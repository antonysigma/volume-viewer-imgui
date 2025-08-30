#pragma once

#include <GLFW/glfw3.h>

#include <optional>

#include "data_models/frame3d.h"
#include "data_models/types.hpp"
#include "view_models/scale.hpp"

namespace {
void
setGLAlphaBlending(const types::BlendMode blend_mode, const float alpha,
                   const float threshold = 0.03f) {
    glAlphaFunc(GL_GREATER, threshold);
    glEnable(GL_BLEND);

    using namespace types;
    switch (blend_mode) {
        case NORMAL: {
            glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
            glBlendColor(1.0f, 1.0f, 1.0f, alpha);
            glBlendEquation(GL_ADD);
            break;
        }
        case ATTENUATE: {
            // https://www.opengl.org/archives/resources/code/samples/advanced/advanced98/notes/node230.html
            glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
            glBlendColor(1.0f, 1.0f, 1.0f, alpha);
            glBlendEquation(GL_ADD);
            break;
        }
        case MAX_INTENSITY:
            // https://www.opengl.org/archives/resources/code/samples/advanced/advanced98/notes/node231.html
            glBlendEquation(GL_MAX);
    }
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
    static inline types::BlendMode blend_mode{types::ATTENUATE};
    static inline float volume_step_size{1.0f};
    static inline float alpha{5e-3f};
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
        setGLAlphaBlending(blend_mode, alpha);
        drawGL3D(*volume, scale, orientation, volume_step_size);
    }
};
}  // namespace components