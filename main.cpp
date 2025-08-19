#include <array>
#include <cassert>
#include <optional>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

#define GL_SILENCE_DEPRECATION
#include <GL/freeglut.h>

namespace {

// Data model
struct Image {
    int width;
    int height;
    std::vector<uint8_t> raw;

    Image(int w, int h) : width{w}, height{h}, raw(w * h) {}

    inline bool isValid() const { return raw.size() == static_cast<size_t>(width) * height; }
};

Image
mockImage() {
    constexpr int W = 256;
    Image image{W, W};

    for (int y = 0; y < W; ++y) {
        for (int x = 0; x < W; ++x) {
            image.raw[y * W + x] = x + y;
        }
    }

    return image;
}

// View models
struct Frame2D {
    int width;
    int height;
    GLuint texture;

    Frame2D(const Image im) : width{im.width}, height{im.height}, texture{0} {
        assert(im.isValid());
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, im.width, im.height, 0,
        //              GL_RGBA, GL_UNSIGNED_BYTE, im.raw.data());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, im.width, im.height, 0, GL_RED, GL_UNSIGNED_BYTE,
                     im.raw.data());

        constexpr std::array<GLint, 4> swizzleMask{GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask.data());
    }
};

// Our state
static bool show_another_window = false;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static float f = 0.0f;
static int counter = 0;
static std::optional<Frame2D> frame{std::nullopt};

void
render(Frame2D& frame, float factor = 1.0f) {
    ImGui::Begin("New image");
    ImGui::Image(frame.texture, ImVec2(frame.width * factor, frame.height * factor));
    ImGui::End();
}

void
MainLoopStep() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGLUT_NewFrame();
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
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
    glutPostRedisplay();
}

struct GuiRuntime {
    GuiRuntime() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

#ifdef __FREEGLUT_EXT_H__
        glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
        glutInitWindowSize(1280, 720);
        glutCreateWindow("Dear ImGui GLUT+OpenGL2 Example");

        // Setup GLUT display function
        // We will also call ImGui_ImplGLUT_InstallFuncs() to get all the other functions installed
        // for us, otherwise it is possible to install our own functions and call the
        // imgui_impl_glut.h functions ourselves.
        glutDisplayFunc(MainLoopStep);

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        // FIXME: Consider reworking this example to install our own GLUT funcs + forward calls
        // ImGui_ImplGLUT_XXX ones, instead of using ImGui_ImplGLUT_InstallFuncs().
        ImGui_ImplGLUT_Init();
        ImGui_ImplOpenGL2_Init();

        // Install GLUT handlers (glutReshapeFunc(), glutMotionFunc(), glutPassiveMotionFunc(),
        // glutMouseFunc(), glutKeyboardFunc() etc.) You can read the io.WantCaptureMouse,
        // io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main
        // application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
        // application, or clear/overwrite your copy of the keyboard data. Generally you may always
        // pass all inputs to dear imgui, and hide them from your application based on those two
        // flags.
        ImGui_ImplGLUT_InstallFuncs();
    }

    ~GuiRuntime() {
        // Cleanup
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGLUT_Shutdown();
        ImGui::DestroyContext();
    }
};
}  // namespace

int
main(int argc, char** argv) {
    // Create GLUT window
    glutInit(&argc, argv);
    GuiRuntime gui_runtime;

    frame = Frame2D{mockImage()};

    // Main loop
    glutMainLoop();

    return 0;
}
