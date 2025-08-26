zlib_dep = dependency('zlib')
nifti_reader_lib = static_library('nifti-reader',
    sources: 'nifti-reader.cpp',
    include_directories: data_models_inc,
    dependencies: zlib_dep,
)

nifti_reader_dep = declare_dependency(
    link_with: nifti_reader_lib,
    include_directories: '.',
    dependencies: zlib_dep,
)