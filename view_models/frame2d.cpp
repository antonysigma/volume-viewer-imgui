#include "frame2d.h"
#include <array>
#include <cassert>

using data_models::Image;

namespace view_models {
Frame2D::Frame2D(const Image im) : width{im.width}, height{im.height}, texture{0} {
        assert(im.isValid());
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        constexpr bool interp_nearest = true;
        if constexpr(interp_nearest) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        update(im);

        constexpr std::array<GLint, 4> swizzleMask{GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask.data());
    }

void Frame2D::update(const Image& im) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, im.width, im.height, 0, GL_RED, GL_UNSIGNED_BYTE,
                     im.raw.data());
    }
}