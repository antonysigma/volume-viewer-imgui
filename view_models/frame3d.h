#pragma once
#include "data_models/types.hpp"
#include "data_models/volume.hpp"
#include <GLFW/glfw3.h>

namespace view_models {
struct Frame3D {
    types::Dimensions dim;
    GLuint texture;

    Frame3D(const data_models::Volume& im);
    Frame3D(const Frame3D&) = delete;
    Frame3D& operator=(const Frame3D&) = delete;
};

}