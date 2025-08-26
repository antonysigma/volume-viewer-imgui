#pragma once
#include <GLFW/glfw3.h>

#include "data_models/image.hpp"

namespace view_models {
struct Frame2D {
    int width;
    int height;
    GLuint texture;

    Frame2D(const data_models::Image im);
    ~Frame2D();

    Frame2D(const Frame2D&) = delete;
    Frame2D& operator=(const Frame2D&) = delete;

    void update(const data_models::Image& im);
};

}  // namespace view_models