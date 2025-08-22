project('imgui-demo', 'cpp',
    version : '0.1',
    default_options : [
        'warning_level=3',
        'cpp_std=c++17',
        'buildtype=debugoptimized',
        'b_ndebug=if-release',
])

executable('imgui-demo',
    sources: 'main.cpp',
    dependencies: [
        dependency('glfw3'),
        dependency('imgui'),
    ]
)