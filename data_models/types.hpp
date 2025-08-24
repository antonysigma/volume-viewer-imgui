#pragma once
#include <vector>
#include <cstdint>

namespace  types {

struct Dimensions {
    int x, y, z;

    constexpr int count() const { return x * y * z; }
};

struct Voxel {
    uint8_t r, g, b, a;

    constexpr Voxel(uint8_t v) : r{v}, g{v}, b{v}, a{5} {}
    constexpr Voxel() : r{0}, g{0}, b{0}, a{0} {}
};
static_assert(sizeof(Voxel) == 4);

struct Orientation {
    int azimuth;
    int elevation;

    constexpr void normalize() {
        azimuth = azimuth % 360;
        elevation = (elevation + 90) % 181 - 90;
    }
};

}