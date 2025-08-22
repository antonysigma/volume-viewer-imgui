#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

namespace {

// Data model
struct Image {
    int width;
    int height;
    std::vector<uint8_t> raw;

    Image(int w, int h) : width{w}, height{h}, raw(w * h) {}

    inline bool isValid() const { return raw.size() == static_cast<size_t>(width) * height; }
};

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

// View models
struct Frame2D {
    int width;
    int height;
    GLuint texture;

    Frame2D(const Image& im) : width{im.width}, height{im.height}, texture{0} {
        assert(im.isValid());
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        constexpr bool interp_nearest = true;
        if constexpr(interp_nearest) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        update(im);

        constexpr std::array<GLint, 4> swizzleMask{GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask.data());
    }

    void update(const Image& im) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, im.width, im.height, 0, GL_RED, GL_UNSIGNED_BYTE,
                     im.raw.data());
    }
};

struct Dimensions {
    int x, y, z;

    constexpr int count() const { return x * y * z; }
};

struct Voxel {
    uint8_t r, g, b, a;

    constexpr Voxel(uint8_t v) : r{v}, g{v}, b{v}, a{1} {}
    constexpr Voxel() : r{0}, g{0}, b{0}, a{0} {}
};
static_assert(sizeof(Voxel) == 4);

struct Volume {
    Dimensions dim;
    std::vector<Voxel> buffer;

    Volume(Dimensions d) : dim{std::move(d)}, buffer(d.count()) {}

    bool isValid() const { return buffer.size() == static_cast<size_t>(dim.count()); }
};

template <int W = 128>
Volume
mockVolume() {
    Volume volume{Dimensions{W, W, W}};

    constexpr float R = 40.0f;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < W; k++) {
                const auto x = i - W / 2;
                const auto y = j - W / 2;
                const auto z = k - W / 2;
                volume.buffer[i + j + k] = (x * x + y * y + z * z <= R * R) ? Voxel{255} : Voxel{0};
            }
        }
    }

    return volume;
}

struct Frame3D {
    Dimensions dim;
    GLuint texture;

    Frame3D(const Volume& im) : dim{im.dim}, texture{0} {
        assert(im.isValid());
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_3D, texture);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

        constexpr bool interp_nearest = true;
        if constexpr (interp_nearest) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

#ifdef __sgi
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP);
#else
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
#endif

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        const auto [x, y, z] = dim;
        glTexImage3D(GL_TEXTURE_3D_EXT, 0, GL_RGBA8, x, y, z, 0, GL_RGBA8, GL_UNSIGNED_BYTE,
                     im.buffer.data());
    }
};

// Our state
static bool show_another_window = false;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static float f = 1.0f;
static int counter = 0;
static std::optional<Frame2D> frame{std::nullopt};
static std::optional<Frame3D> volume{std::nullopt};

void
render(Frame2D& frame, float factor = 1.0f) {
    ImGui::Begin("New image");
    ImGui::Image(frame.texture, ImVec2(frame.width * factor, frame.height * factor));
    ImGui::End();
}

struct Orientation {
    int azimuth;
    int elevation;

    constexpr void normalize() {
        azimuth = azimuth % 360;
        elevation = elevation % 90;
    }
};

void
drawGL3D(const Frame3D& volume, float scale, Orientation o,
         float quality) { /* volume render using a single 3D texture */
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(0.5f, 0.5f, 0.5f);
    glScalef(scale, scale, scale);
    glRotatef(90, 1, 0, 0);

    o.normalize();
    glRotatef(o.azimuth, 0, 1, 0);
    glRotatef(-o.elevation, 1, 0, 0);

    glTranslatef(-0.5f, -0.5f, -0.5f);

    glEnable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, volume.texture);
    const auto [x, y, z] = volume.dim;
    const auto render_quality = 1.7f * 1.0f / (x + y + z) * quality;
    for (auto fz = -0.5f; fz <= 0.5f; fz += render_quality) {
        const float tz = fz + 0.5f;
        const float vz = (fz * 2.0f) - 0.2;

        glBegin(GL_QUADS);
        glTexCoord3f(0.0f, 0.0f, tz);
        glVertex3f(-1.0f, -1.0f, vz);
        glTexCoord3f(1.0f, 0.0f, tz);
        glVertex3f(1.0f, -1.0f, vz);
        glTexCoord3f(1.0f, 1.0f, tz);
        glVertex3f(1.0f, 1.0f, vz);
        glTexCoord3f(0.0f, 1.0f, tz);
        glVertex3f(-1.0f, 1.0f, vz);
        glEnd();
    }
}

struct GLAlphaBlending {
    GLAlphaBlending(const float threshold = 0.03f) {
        constexpr bool is_clip_plane = true;
        if constexpr (is_clip_plane) {
            glEnable(GL_CLIP_PLANE0);
        } else {
            glDisable(GL_CLIP_PLANE0);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glAlphaFunc(GL_GREATER, threshold);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    //~GLAlphaBlending() {
    //    glDisable(GL_BLEND);
    //}
};

void
MainLoopStep(GLFWwindow* window) {
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
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f,
                           10.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color",
                          (float*)&clear_color);  // Edit 3 floats representing a color

        if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true
                                      // when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
                    io.Framerate);
        ImGui::End();
    }

    if (frame) {
        render(*frame, f);
    }

    // 3. Show another simple window.
    if (show_another_window) {
        ImGui::Begin(
            "Another Window",
            &show_another_window);  // Pass a pointer to our bool variable (the window will have a
                                    // closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) show_another_window = false;
        ImGui::End();
    }

    // ImGui::ShowMetricsWindow();

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    // if (volume) {
    //     GLAlphaBlending alpha_blending;
    //     // drawGL3D(*volume, f, Orientation{0, 40}, 0.8f);
    // }
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
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
        : fd{glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr)} {
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

    frame = Frame2D{mockImage()};
    volume = Frame3D{mockVolume()};

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
