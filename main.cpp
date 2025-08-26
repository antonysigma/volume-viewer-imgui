#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

#include "components/click_counter.hpp"
#include "components/image_viewer.hpp"
#include "components/volume_viewer.hpp"

namespace {

using view_models::Frame2D;
using view_models::Frame3D;
using data_models::Image;
using data_models::Volume;
using types::Orientation;
using types::Dimensions;
using types::Voxel;

template<int W=1024>
Image
mockImage(const uint8_t offset = 0) {
    Image image{W, W};

    for (int y = 0; y < W; ++y) {
        for (int x = 0; x < W; ++x) {
            image.raw[y * W + x] = x + y + offset;
        }
    }

    return image;
}

template <int W = 128>
Volume
mockVolume() {
    Volume volume{Dimensions{W, W, W}};

    constexpr float R = W / 3;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < W; k++) {
                const auto x = i - W / 2.0f;
                const auto y = j - W / 2.0f;
                const auto z = k - W / 2.0f;
                // volume.buffer[i + j*W + k*W*W] = (x * x + y * y + z * z <= R * R) ? Voxel{255} :
                // Voxel{0};
                const auto abs = [](float v) { return v >= 0 ? v : -v; };
                volume.buffer[i + j * W + k * W * W] = (abs(x) + abs(y) + abs(z) <= R) ? 255 : 0;
            }
        }
    }

    return volume;
}

// Our state
static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.00f, 1.00f);

void
MainLoopStep(GLFWwindow* window) {
    using namespace components;
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named
    // window.
    {
        ImGui::Begin(
            "Hello, world!");  // Create a window called "Hello, world!" and append into it.

        ImGui::Text(
            "This is some useful text.");  // Display some text (you can use a format strings too)

        ImGui::SliderFloat("Scale", &view_models::scale, 0.0f, 10.0f);
        ImGui::SliderFloat("alpha (coarse)", &VolumeViewer::alpha, 0.0f, 0.5f);
        {
            const float vmin = std::floor(VolumeViewer::alpha * 10.0f) / 10.0f;
            const float vmax =
                std::max(std::ceil(VolumeViewer::alpha * 10.0f) / 10.0f, vmin + 0.1f) - 0.01f;
            ImGui::SliderFloat("alpha (fine)", &VolumeViewer::alpha, vmin, vmax);
        }

        constexpr float step_size_min = std::sqrt(0.5f);
        ImGui::SliderFloat("Step size", &VolumeViewer::volume_step_size, step_size_min,
                           5.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::SliderInt("azimuth", &VolumeViewer::orientation.azimuth, 0, 360);
        ImGui::SliderInt("elevation", &VolumeViewer::orientation.elevation, -90, 90);
        ImGui::ColorEdit3("clear color",
                          (float*)&clear_color);  // Edit 3 floats representing a color

        ClickCounter::render();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
                    io.Framerate);
        ImGui::End();
    }

    ImageViewer::render();

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    {
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
    }

    VolumeViewer::render();
    VolumeViewer::orientation.azimuth += 1;
    VolumeViewer::orientation.normalize();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

struct GuiRuntime {
    GuiRuntime(GLFWwindow* window) {
        assert(window != nullptr);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        constexpr char glsl_version[]{"#version 130"};
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    ~GuiRuntime() {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
};

void
glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

enum fps_t { FPS60 = 1, FPS30 = 2 };

struct Window {
    GLFWwindow* fd;

    Window()
        : fd{glfwCreateWindow(1280, 1280, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr)} {
        glfwMakeContextCurrent(fd);
        glfwSwapInterval(FPS30);  // Enable vsync
    }

    ~Window() {
        glfwDestroyWindow(fd);
        glfwTerminate();
    }
};

}  // namespace

int
main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    Window window;
    if (window.fd == nullptr) {
        return 1;
    }

    GuiRuntime gui_runtime{window.fd};

    components::ImageViewer::frame.emplace(mockImage());
    components::VolumeViewer::volume.emplace(mockVolume());

    // Main loop
    while (!glfwWindowShouldClose(window.fd)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui
        // wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main
        // application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
        // application, or clear/overwrite your copy of the keyboard data. Generally you may always
        // pass all inputs to dear imgui, and hide them from your application based on those two
        // flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window.fd, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        MainLoopStep(window.fd);
    }

    return 0;
}
