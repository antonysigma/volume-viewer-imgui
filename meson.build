project('imgui-demo', 'cpp', 'c',
    version : '0.1',
    default_options : [
        'warning_level=3',
        'cpp_std=c++17',
        'buildtype=debugoptimized',
        'b_ndebug=if-release',
])

data_models_inc = include_directories('.')
subdir('data_models')
subdir('nifti-reader')

executable('imgui-demo',
    sources: 'main.cpp',
    dependencies: [
        frame2d_dep,
        frame3d_dep,
        dependency('glfw3'),
        dependency('imgui'),
        nifti_reader_dep,
    ]
)
