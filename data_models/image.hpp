#pragma once
#include <cstdint>
#include <vector>

namespace data_models {
struct Image {
    int width;
    int height;
    std::vector<uint8_t> raw;

    Image(int w, int h) : width{w}, height{h}, raw(w * h) {}

    inline bool isValid() const { return raw.size() == static_cast<size_t>(width) * height; }
};

}  // namespace data_models