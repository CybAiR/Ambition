#include <filesystem>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "panels/SidePanel.h"
#include "panels/TopPanel.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

#include "fonts/IconsFontAwesome6.h"
#include "views/MaintenanceView.h"
#include "views/ScienceView.h"

int main(int argc, char* argv[])
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    ImFont* main_font = io.Fonts->AddFontFromFileTTF("../misc/fonts/Roboto-Medium.ttf", 15.0f);

    // Merge FA into main_font immediately after
    ImFontConfig icon_config;
    icon_config.MergeMode = true;
    icon_config.PixelSnapH = true;
    icon_config.GlyphOffset = ImVec2(-3.0f, 1.0f); // nudge down, adjust value to taste
    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    io.Fonts->AddFontFromFileTTF("../misc/fonts/fa-solid-900.otf", 15.0f, &icon_config,
                                 icon_ranges);

    ImFont* bold_font = io.Fonts->AddFontFromFileTTF("../misc/fonts/Roboto-Medium.ttf", 15.0f);
    ImFont* logo_font = io.Fonts->AddFontFromFileTTF("../misc/fonts/Roboto-Medium.ttf", 32.0f);

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    SDL_Window* window = SDL_CreateWindow("App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");

    Views active_view = Views::Science;
    // ScienceView science_view;
    MaintenanceView maintenance_view;

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGuiWindowFlags side_panel_flags = ImGuiWindowFlags_NoTitleBar |
                                            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoCollapse;

        ImGuiWindowFlags main_view_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                           ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoCollapse;

        ImGuiWindowFlags top_panel_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

        TopPanel top_panel(viewport->WorkSize.x, top_panel_flags);
        top_panel.render(viewport);

        SidePanel side_panel(side_panel_flags);
        side_panel.render(viewport, active_view);

        ImVec2 main_view_pos(viewport->WorkPos.x + side_panel.get_width(),
                             viewport->WorkPos.y + 75.0f);
        ImVec2 main_view_size(viewport->WorkSize.x - side_panel.get_width(),
                              viewport->WorkSize.y - 75.0f);

        ImGui::SetNextWindowPos(main_view_pos);
        ImGui::SetNextWindowSize(main_view_size);

        ImGui::Begin("MainView", nullptr, main_view_flags);

        switch (active_view)
        {
        case Views::Science:
            ImGui::Text("Widok Science (w budowie)");
            break;
        case Views::Navigation:
            ImGui::Text("Widok Navigation (w budowie)");
            break;
        case Views::Maintenance:
            maintenance_view.render();
            break;
        case Views::Probing:
            ImGui::Text("Widok Probing (w budowie)");
            break;
        }

        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}