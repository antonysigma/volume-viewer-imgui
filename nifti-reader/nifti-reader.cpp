#include "nifti-reader.h"

#include <zlib.h>

#include <cmath>

namespace storage {

Expected<NiftiReader, Error>
NiftiReader::open(const char filename[]) {
    const auto fp = gzopen(filename, "rb");
    if (fp == nullptr) {
        return CANNOT_OPEN_FILE;
    }

    NiftiReader file{};
    const auto bytes_read = gzread(fp, &(file.header), sizeof(nifti_1_header));
    if (bytes_read != sizeof(nifti_1_header)) {
        return INVALID_GZIP_STREAM;
    }

    const auto& header = file.header;
    if (header.sizeof_hdr != 348) {
        return NOT_LITTLE_ENDIAN;
    }

    if ((header.datatype != 2) || (header.bitpix != 8)) {
        return NOT_8BIT;
    }

    return file;
}

types::Dimensions
NiftiReader::dimensions() const {
    return {header.dim[1], header.dim[2], header.dim[3]};
}

types::VoxelSize
NiftiReader::voxelSize() const {
    const auto normalize = [](const float x) -> float { return x == 0.0f ? 1.0f : std::abs(x); };

    return {normalize(header.pixdim[1]), normalize(header.pixdim[2]), normalize(header.pixdim[3])};
}
}  // namespace storage