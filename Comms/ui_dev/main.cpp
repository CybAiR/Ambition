// Komsiki Station – UI development skeleton.
//
// This is a standalone copy of the station dashboard UI with ALL networking,
// camera streaming, OpenCV, GStreamer, and hardware dependencies removed.
// Camera feeds are replaced with colored placeholders so you can iterate on
// the layout, styling, and widgets without connecting to any hardware.
//
// Dependencies: Dear ImGui (GLFW + OpenGL3 backend), GLFW, OpenGL.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

// ---------------------------------------------------------------------------
// Simulated shared state (no real data – just for UI wiring)
// ---------------------------------------------------------------------------
struct UIState {
    // Keyboard
    std::mutex key_mtx;
    std::string last_key_sent;

    // Simulated latency (static value for display)
    double latency_ms = 12.3;

    // Camera enable flags
    bool cam_enabled[4] = {true, true, true, true};

    const char* mode_list[3] = {"SMART", "POPOUT", "PREMADE"};
    int wybrany = 0;
};

static UIState g_state;

// ---------------------------------------------------------------------------
// OpenGL texture helpers (used for placeholder textures)
// ---------------------------------------------------------------------------
static GLuint create_placeholder_texture(int width, int height,
                                         unsigned char r, unsigned char g,
                                         unsigned char b) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create a solid-color RGBA image
    const int channels = 4;
    unsigned char* pixels = new unsigned char[width * height * channels];
    for (int i = 0; i < width * height; ++i) {
        pixels[i * channels + 0] = r;
        pixels[i * channels + 1] = g;
        pixels[i * channels + 2] = b;
        pixels[i * channels + 3] = 255;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    delete[] pixels;
    return tex;
}

// ---------------------------------------------------------------------------
// GLFW key callback – stores key name locally (no network send)
// ---------------------------------------------------------------------------
static void key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/,
                          int action, int /*mods*/) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    const char* name = glfwGetKeyName(key, 0);
    std::string key_str;
    if (name) {
        key_str = name;
    } else {
        switch (key) {
            case GLFW_KEY_SPACE:        key_str = "SPACE"; break;
            case GLFW_KEY_ENTER:        key_str = "ENTER"; break;
            case GLFW_KEY_ESCAPE:       key_str = "ESC"; break;
            case GLFW_KEY_UP:           key_str = "UP"; break;
            case GLFW_KEY_DOWN:         key_str = "DOWN"; break;
            case GLFW_KEY_LEFT:         key_str = "LEFT"; break;
            case GLFW_KEY_RIGHT:        key_str = "RIGHT"; break;
            case GLFW_KEY_LEFT_SHIFT:   key_str = "LSHIFT"; break;
            case GLFW_KEY_RIGHT_SHIFT:  key_str = "RSHIFT"; break;
            case GLFW_KEY_LEFT_CONTROL: key_str = "LCTRL"; break;
            case GLFW_KEY_RIGHT_CONTROL:key_str = "RCTRL"; break;
            case GLFW_KEY_TAB:          key_str = "TAB"; break;
            case GLFW_KEY_BACKSPACE:    key_str = "BACKSPACE"; break;
            default:                    key_str = "KEY_" + std::to_string(key); break;
        }
    }

    {
        std::lock_guard<std::mutex> lk(g_state.key_mtx);
        g_state.last_key_sent = key_str;
    }
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int /*argc*/, char* /*argv*/[]) {
    // ---- GLFW + OpenGL init ----
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080,
        "KOMSIKI - UI Dev (offline)", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // vsync on for dev comfort
    glfwSetKeyCallback(window, key_callback);

    // ---- ImGui init ----
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    // When viewports are enabled, tweak WindowRounding/WindowBg so platform
    // windows look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Scale UI a bit
    ImGui::GetStyle().ScaleAllSizes(1.2f);
    io.FontGlobalScale = 1.2f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ---- Placeholder textures for camera feeds ----
    // Dark blue-ish for CAM1, dark green-ish for CAM2
    GLuint cam_tex[4] = {
        create_placeholder_texture(320, 240, 30, 40, 80),
        create_placeholder_texture(320, 240, 30, 70, 50),
        create_placeholder_texture(320, 240, 30, 100, 20),
        create_placeholder_texture(320, 240, 30, 130, 0),
    };
    // Placeholder dimensions (aspect ratio 16:9)
    const float placeholder_aspect = 16.0f / 9.0f;

    // Camera enable checkboxes
    bool cam_checkbox[4] = {true, true, true, true};

    const char* mode_list[3] = {"SMART", "POPOUT", "PREMADE"};
    int wybrany = 0;
    const char* cam_names[4] = {"CAM1", "CAM2", "CAM3", "CAM4"};

    // ---- Main render loop ----
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get window size for layout
        int win_w, win_h;
        glfwGetFramebufferSize(window, &win_w, &win_h);

        // Main viewport position (screen-space origin for window positioning)
        ImGuiViewport* main_vp = ImGui::GetMainViewport();
        ImVec2 vp_pos = main_vp->Pos;

        // ================================================================
        // Control Panel (left side) — always pinned to main viewport
        // ================================================================
        ImGui::SetNextWindowViewport(main_vp->ID);
        ImGui::SetNextWindowPos(ImVec2(vp_pos.x + 10, vp_pos.y + 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, static_cast<float>(win_h) - 20), ImGuiCond_Always);
        ImGui::Begin("Panel sterowania", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

        // Latency (simulated)
        ImGui::SeparatorText("Latencja");
        double lat = g_state.latency_ms;
        ImVec4 lat_color = (lat < 20.0) ? ImVec4(0, 1, 0, 1) :
                           (lat < 50.0) ? ImVec4(1, 1, 0, 1) :
                                          ImVec4(1, 0, 0, 1);
        ImGui::TextColored(lat_color, "RTT/2: %.1f ms", lat);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1), "");

        // Keyboard info
        ImGui::SeparatorText("Klawiatura");
        {
            std::lock_guard<std::mutex> lk(g_state.key_mtx);
            ImGui::Text("Wyslany:  %s", g_state.last_key_sent.empty()
                        ? "-" : g_state.last_key_sent.c_str());
        }
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1), "");

        // Camera controls
        ImGui::SeparatorText("Kamery");
        for (int i = 0; i < 4; ++i) {
            ImGui::Checkbox(cam_names[i], &cam_checkbox[i]);
            g_state.cam_enabled[i] = cam_checkbox[i];
            if (cam_checkbox[i]) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0, 1, 0, 1), " [WLACZONA]");
            }
        }

        ImGui::SeparatorText("Info");
        ImGui::Text("FPS: %.0f", io.Framerate);
        ImGui::Text("Rover IP: ");


       ImGui::SeparatorText("Tryb wyswietlania");
       ImGui::Combo("Opcje", &wybrany, mode_list, IM_ARRAYSIZE(mode_list));
       ImGui::Text("Wybrany tryb: %s", mode_list[wybrany]);



        ImGui::End();

        // ================================================================
        // Camera Feeds
        // ================================================================
        float feed_x = vp_pos.x + 320.0f;
        float feed_total_w = static_cast<float>(win_w) - 320.0f - 10.0f;
        float feed_total_h = static_cast<float>(win_h) - 20.0f;
        float feed_y0 = vp_pos.y + 10.0f;
        const float gap = 10.0f;

        // Collect enabled camera indices
        int active_cams[4];
        int active_count = 0;
        for (int i = 0; i < 4; ++i) {
            if (g_state.cam_enabled[i])
                active_cams[active_count++] = i;
        }

        // Shared: render camera image + overlay inside current window
        auto render_cam_content = [&](int cam_idx) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float disp_w = avail.x;
            float disp_h = disp_w / placeholder_aspect;
            if (disp_h > avail.y) {
                disp_h = avail.y;
                disp_w = disp_h * placeholder_aspect;
            }
            ImGui::SetCursorPosX((avail.x - disp_w) * 0.5f + ImGui::GetCursorPosX());
            ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(cam_tex[cam_idx])),
                         ImVec2(disp_w, disp_h));

            ImVec2 text_pos = ImGui::GetItemRectMin();
            char overlay[64];
            std::snprintf(overlay, sizeof(overlay), "", cam_idx + 1);
            ImGui::GetWindowDrawList()->AddText(
                ImVec2(text_pos.x + 10, text_pos.y + 10),
                IM_COL32(255, 255, 255, 200), overlay);
        };

        if (wybrany == 0) {
            // ---- SMART mode: fixed layout in the feed area ----
            auto render_cam_window = [&](int cam_idx, float x, float y, float w, float h) {
                ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
                ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);

                char title[64];
                std::snprintf(title, sizeof(title), "%s###cam%d", cam_names[cam_idx], cam_idx);
                ImGui::Begin(title, nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoDocking);
                render_cam_content(cam_idx);
                ImGui::End();
            };

            if (active_count == 1) {
                render_cam_window(active_cams[0], feed_x, feed_y0, feed_total_w, feed_total_h);
            } else if (active_count == 2) {
                float h = (feed_total_h - gap) / 2.0f;
                render_cam_window(active_cams[0], feed_x, feed_y0, feed_total_w, h);
                render_cam_window(active_cams[1], feed_x, feed_y0 + h + gap, feed_total_w, h);
            } else if (active_count == 3) {
                float row_h = (feed_total_h - gap) / 2.0f;
                float half_w = (feed_total_w - gap) / 2.0f;
                render_cam_window(active_cams[0], feed_x, feed_y0, half_w, row_h);
                render_cam_window(active_cams[1], feed_x + half_w + gap, feed_y0, half_w, row_h);
                render_cam_window(active_cams[2], feed_x, feed_y0 + row_h + gap, feed_total_w, row_h);
            } else if (active_count >= 4) {
                float row_h = (feed_total_h - gap) / 2.0f;
                float half_w = (feed_total_w - gap) / 2.0f;
                render_cam_window(active_cams[0], feed_x, feed_y0, half_w, row_h);
                render_cam_window(active_cams[1], feed_x + half_w + gap, feed_y0, half_w, row_h);
                render_cam_window(active_cams[2], feed_x, feed_y0 + row_h + gap, half_w, row_h);
                render_cam_window(active_cams[3], feed_x + half_w + gap, feed_y0 + row_h + gap, half_w, row_h);
            }
        } else if (wybrany == 1) {
            // ---- POPOUT mode: independent OS windows (via ImGui viewports) ----
            for (int j = 0; j < active_count; ++j) {
                int cam_idx = active_cams[j];

                float init_w = 640.0f;
                float init_h = init_w / placeholder_aspect + 40.0f;
                // Place initial position outside the main window area so it
                // spawns as a separate OS window immediately.
                ImVec2 main_pos = ImGui::GetMainViewport()->Pos;
                ImGui::SetNextWindowPos(
                    ImVec2(main_pos.x + static_cast<float>(win_w) + 20.0f + 40.0f * j,
                           main_pos.y + 40.0f * j),
                    ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(init_w, init_h), ImGuiCond_Once);

                char title[64];
                std::snprintf(title, sizeof(title), "%s###cam%d", cam_names[cam_idx], cam_idx);
                ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
                render_cam_content(cam_idx);
                ImGui::End();
            }
        } else if (wybrany == 2) {
            // ---- PREMADE mode: free-floating windows clamped inside the main viewport ----
            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImVec2 vp_min = vp->WorkPos;
            ImVec2 vp_max = ImVec2(vp_min.x + vp->WorkSize.x, vp_min.y + vp->WorkSize.y);

            for (int j = 0; j < active_count; ++j) {
                int cam_idx = active_cams[j];

                float init_w = 480.0f;
                float init_h = init_w / placeholder_aspect + 40.0f;
                ImGui::SetNextWindowPos(
                    ImVec2(feed_x + 30.0f * j, feed_y0 + 30.0f * j),
                    ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(init_w, init_h), ImGuiCond_Once);
                ImGui::SetNextWindowViewport(vp->ID);

                char title[64];
                std::snprintf(title, sizeof(title), "%s###cam%d", cam_names[cam_idx], cam_idx);
                ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

                // Clamp window position to stay inside the main viewport
                ImVec2 wpos = ImGui::GetWindowPos();
                ImVec2 wsz  = ImGui::GetWindowSize();
                float vp_w = vp_max.x - vp_min.x;
                float vp_h = vp_max.y - vp_min.y;
                if (wsz.x > vp_w) wsz.x = vp_w;
                if (wsz.y > vp_h) wsz.y = vp_h;
                bool clamped = false;
                if (wpos.x < vp_min.x) { wpos.x = vp_min.x; clamped = true; }
                if (wpos.y < vp_min.y) { wpos.y = vp_min.y; clamped = true; }
                if (wpos.x + wsz.x > vp_max.x) { wpos.x = vp_max.x - wsz.x; clamped = true; }
                if (wpos.y + wsz.y > vp_max.y) { wpos.y = vp_max.y - wsz.y; clamped = true; }
                if (clamped) ImGui::SetWindowPos(wpos);

                render_cam_content(cam_idx);
                ImGui::End();
            }
        }

        // ---- Render ----
        ImGui::Render();
        glViewport(0, 0, win_w, win_h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and render additional platform windows (multi-viewport)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // ---- Cleanup ----
    glDeleteTextures(4, cam_tex);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
