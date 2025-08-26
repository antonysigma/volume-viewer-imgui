#include "nifti-reader.h"

#include <zlib.h>

#include <cmath>
#include <cstdint>
#include <memory>

namespace {
struct GzFileWrapper {
    gzFile fp{};

    GzFileWrapper(const char filename[]) { fp = gzopen(filename, "rb"); }

    ~GzFileWrapper() {
        if (fp != nullptr) {
            gzclose(fp);
        }
    }
};

}  // namespace

namespace storage {

Expected<NiftiReader, Error>
NiftiReader::open(const char filename[]) {
    GzFileWrapper gz_file{filename};
    const auto fp = gz_file.fp;
    if (fp == nullptr) {
        return CANNOT_OPEN_FILE;
    }

    NiftiReader file{};
    const auto bytes_header_read = gzread(fp, &(file.header), sizeof(nifti_1_header));
    if (bytes_header_read != sizeof(nifti_1_header)) {
        return INVALID_GZIP_STREAM;
    }

    const auto& header = file.header;
    if (header.sizeof_hdr != 348) {
        return NOT_LITTLE_ENDIAN;
    }

    if ((header.datatype != 2) || (header.bitpix != 8)) {
        return NOT_8BIT;
    }

    {
        const auto isPow2 = [](unsigned x) -> bool { return __builtin_popcount(x) == 1; };

        const auto [nx, ny, nz] = file.dimensions();
        const bool all_power_of_two = isPow2(nx) && isPow2(ny) && isPow2(nz);
        if (!all_power_of_two) {
            return NOT_POWER_OF_TWO;
        }
    }

    const auto n_voxels = file.dimensions().count();
    if (header.vox_offset > n_voxels * sizeof(uint8_t)) {
        return IMAGE_IS_COMPRESSED;
    }

    // Seek to the voxel data offset (only forward seeks are reliable in gzipped streams)
    if (gzseek(fp, static_cast<long>(header.vox_offset), SEEK_SET) == -1) {
        return GZ_SEEK_FAILED;
    }

    // Read the actual payload
    file.raw.resize(n_voxels);
    const auto bytes_read = gzread(fp, file.raw.data(), n_voxels * sizeof(uint8_t));
    if (bytes_read != n_voxels * sizeof(uint8_t)) {
        return VOXEL_READ_FAILED;
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