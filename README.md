# Volume viewer for Z-stack monochrome images (ImGui + OpenGL)

Modern browser is very capable of 3D rendering (e.g. the [Allen Institute Volume
viewer](https://vole.allencell.org/)), but there are scenarios where low RAM/CPU
overhead is preferred. Sometimes, the target machine restricts the use of web
browsers, even with the native [libwebview
technology](https://developer.microsoft.com/en-us/Microsoft-edge/webview2) built
into the operating system.

Here, I experiment with real-time volume rendering using GLFW/OpenGL on NVIDIA
GPUs. It integrates [Dear ImGui](https://github.com/ocornut/imgui) for
interactive control of the volume's orientation, opacity, and 3D resolution. The
code is inspired by the reference implementation from
[neurolabusc/vx](https://github.com/neurolabusc/vx), which in turn targeted
legacy OpenGL v1 APIs.

![Preview](./preview.gif)

## Build instructions for Ubuntu/Linux

Install OpenGL drivers:
```bash
sudo apt install build-essentials libglfw3-dev
```

Install [UV, the Python pip accelerator](https://docs.astral.sh/uv/#installation).
Clone this project, then set up the build systems:
```bash
cd path/to/volume-viewer-imgui/
uv venv --python=3.12
source .venv/bin/activate
uv pip install meson ninja
```

Now, resolve the 3rd party dependencies:
```bash
cd path/to/volume-viewer-imgui/
source .venv/bin/activate
meson setup build/
```

Compile everything
```bash
ninja -C build/
```

Next, download a [test dataset](https://github.com/neurolabusc/vx/raw/refs/heads/master/dist/vx.nii.gz), then run the demo app.
```bash
wget https://github.com/neurolabusc/vx/raw/refs/heads/master/dist/vx.nii.gz
build/imgui-demo vx.nii.gz
```

