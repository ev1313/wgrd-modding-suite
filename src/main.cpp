
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <epoxy/gl.h>
// needs to be after epoxy include, else will include custom GL headers
#include <GLFW/glfw3.h>

#include "maingui.hpp"
#include "threadpool.hpp"

#include <libintl.h>
#include <locale.h>

static void glfw_error_callback(int error, const char *description) {
  std::cerr << std::vformat(gettext("GLFW Error {}: {}\n"),
                            std::make_format_args(error, description));
  spdlog::error(std::vformat(gettext("GLFW Error {}: {}\n"),
                             std::make_format_args(error, description)));
}

static bool exit_now = false;
static void window_close_callback(GLFWwindow *window) {
  spdlog::info(gettext("Window close callback"));
  return;
  if (!exit_now)
    glfwSetWindowShouldClose(window, GLFW_FALSE);
}

// Main code
int main(int argc, char *argv[]) {
  // initialize for localization with GNU gettext
  setlocale(LC_ALL, "");
  bindtextdomain("wgrd_mod_manager", ".");
  textdomain("wgrd_mod_manager");

  py::scoped_interpreter guard{};
  {
    py::gil_scoped_release release;
    // create the main gui inside this, so deinit is called before the python
    // interpreter is deinitialized
    maingui main_gui;
    main_gui.init(argc, argv);

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
      return 1;

      // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
    // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(
        1280, 720, gettext("WG: RD Modding Suite"), nullptr, nullptr);
    if (window == nullptr)
      return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;            // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigDockingWithShift = true;
    io.ConfigDockingAlwaysTabBar = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
    // need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr.
    // Please handle those errors in your application (e.g. use an assertion, or
    // display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and
    // stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame
    // below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use
    // Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at
    // runtime from the "fonts/" folder. See Makefile.emscripten for details.
    // io.Fonts->AddFontDefault();
    ImFontConfig config;
    config.MergeMode = true;
    io.Fonts->AddFontFromFileTTF(
        (fs::path("fonts") / fs::path("NotoSansMono-Regular.ttf"))
            .string()
            .c_str(),
        24.0f);
    io.Fonts->AddFontFromFileTTF(
        (fs::path("fonts") / fs::path("NotoSansMono-Regular.ttf"))
            .string()
            .c_str(),
        24.0f, &config, io.Fonts->GetGlyphRangesGreek());
    io.Fonts->AddFontFromFileTTF(
        (fs::path("fonts") / fs::path("NotoSansMono-Regular.ttf"))
            .string()
            .c_str(),
        24.0f, &config, io.Fonts->GetGlyphRangesCyrillic());
    io.Fonts->AddFontFromFileTTF(
        (fs::path("fonts") / fs::path("NotoSansMonoCJK-VF.ttf.ttc"))
            .string()
            .c_str(),
        24.0f, &config, io.Fonts->GetGlyphRangesJapanese());
    io.Fonts->AddFontFromFileTTF(
        (fs::path("fonts") / fs::path("NotoSansMonoCJK-VF.ttf.ttc"))
            .string()
            .c_str(),
        24.0f, &config, io.Fonts->GetGlyphRangesChineseFull());
    io.Fonts->AddFontFromFileTTF(
        (fs::path("fonts") / fs::path("NotoSansMonoCJK-VF.ttf.ttc"))
            .string()
            .c_str(),
        24.0f, &config, io.Fonts->GetGlyphRangesVietnamese());
    io.Fonts->AddFontFromFileTTF(
        (fs::path("fonts") / fs::path("NotoSansMonoCJK-VF.ttf.ttc"))
            .string()
            .c_str(),
        24.0f, &config, io.Fonts->GetGlyphRangesKorean());
    io.Fonts->AddFontFromFileTTF(
        (fs::path("fonts") / fs::path("NotoSansMonoCJK-VF.ttf.ttc"))
            .string()
            .c_str(),
        24.0f, &config, io.Fonts->GetGlyphRangesThai());
    io.Fonts->Build();
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);
    glfwSetWindowCloseCallback(window, window_close_callback);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!exit_now && !glfwWindowShouldClose(window)) {
      /*
      if(!exit_now && glfwWindowShouldClose(window)) {
        glfwDestroyWindow(window);
        GLFWwindow* window = glfwCreateWindow(1280, 720, gettext("WG: RD Modding
      Suite"), nullptr, nullptr); if (window == nullptr) throw
      std::runtime_error("could not recreate window");
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync
      }
      */
      // Poll and handle events (inputs, window resize, etc.)
      // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
      // tell if dear imgui wants to use your inputs.
      // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
      // your main application, or clear/overwrite your copy of the mouse data.
      // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
      // data to your main application, or clear/overwrite your copy of the
      // keyboard data. Generally you may always pass all inputs to dear imgui,
      // and hide them from your application based on those two flags.
      glfwPollEvents();

      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // render the main gui
      exit_now = main_gui.render();

      // Rendering
      ImGui::Render();
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);
      glViewport(0, 0, display_w, display_h);
      glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                   clear_color.z * clear_color.w, clear_color.w);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }

    ThreadPoolSingleton::get_instance().shutdown();

    py::gil_scoped_acquire acquire;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
  }
  py::gil_scoped_acquire acquire;

  return 0;
}
