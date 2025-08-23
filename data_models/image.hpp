#pragma once
#include <vector>
#include <cstdint>

namespace data_models {
struct Image {
    size_t width;
    size_t height;
    std::vector<uint8_t> raw;

    Image(size_t w, size_t h) : width{w}, height{h}, raw(w * h) {}

    inline bool isValid() const { return raw.size() == width * height; }
};

}