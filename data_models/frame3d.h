#pragma once
#include <GLFW/glfw3.h>

#include "data_models/types.hpp"
#include "data_models/volume.hpp"

namespace view_models {
struct Frame3D {
    types::Dimensions dim;
    GLuint texture;

    Frame3D(const data_models::Volume& im);
    ~Frame3D();

    Frame3D(const Frame3D&) = delete;
    Frame3D& operator=(const Frame3D&) = delete;
};

}  // namespace view_models