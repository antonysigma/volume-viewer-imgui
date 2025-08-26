#pragma once
#include <vector>
#include <cstdint>
#include "types.hpp"

namespace data_models {

struct Volume {
    types::Dimensions dim;
    std::vector<uint8_t> buffer;

    Volume(types::Dimensions d, std::vector<uint8_t>&& b) : dim{std::move(d)}, buffer{b} {}
    Volume(types::Dimensions d) : dim{std::move(d)}, buffer(d.count()) {}

    bool isValid() const { return buffer.size() == static_cast<size_t>(dim.count()); }
};

}