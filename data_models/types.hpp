#pragma once
#include <cstdint>
#include <vector>

namespace types {

template <typename T>
struct Vec3 {
    T x{};
    T y{};
    T z{};
};

struct Dimensions : Vec3<int> {
    constexpr int count() const { return x * y * z; }
};

using VoxelSize = Vec3<float>;

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

enum BlendMode {
    NORMAL,
    ATTENUATE,
    MAX_INTENSITY,
};
}  // namespace types