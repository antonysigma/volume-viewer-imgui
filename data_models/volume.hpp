#pragma once
#include <cstdint>
#include <vector>

#include "types.hpp"

namespace data_models {

struct Volume {
    types::Dimensions dim;
    types::VoxelSize voxel_size;
    std::vector<uint8_t> buffer;

    Volume(types::Dimensions d, types::VoxelSize vs, std::vector<uint8_t>&& b)
        : dim{d}, voxel_size{vs}, buffer{b} {}
    Volume(types::Dimensions d)
        : dim{std::move(d)}, voxel_size{1.0f, 1.0f, 1.0f}, buffer(d.count()) {}

    bool isValid() const { return buffer.size() == static_cast<size_t>(dim.count()); }
};

}  // namespace data_models