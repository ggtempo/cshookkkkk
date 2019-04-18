#pragma once
#include <cstring>
#include "../hooks.hpp"

#include "../../Utils/utils.hpp"
#include "../../Utils/globals.hpp"
#include "../../Utils/math.hpp"

#include "../../Features/AntiAim/antiaim.hpp"
#include "../../Features/Triggerbot/triggerbot.hpp"
#include "../../Features/Visuals/visuals.hpp"
#include "../../Features/Aimbot/aimbot.hpp"
#include "../../Features/Removals/removals.hpp"

namespace hooks
{
    BOOL __stdcall hk_wgl_swap_buffers(HDC hDc)
    {
        using wgl_swap_buffers_fn = BOOL(__stdcall*)(HDC);

        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<wgl_swap_buffers_fn>(g.original_wgl_swap_buffers);

        if (!g.first)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);check_gl_error();
            glDrawBuffer(GL_BACK);check_gl_error();
        }

        if (g.first)
        {
            // Load GL
            ImGui_Impl_LoadGL();check_gl_error();

            // Debug OpenGL Stuff
            auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));check_gl_error();
            auto extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));check_gl_error();

            IMGUI_CHECKVERSION();check_gl_error();
            ImGui::CreateContext();check_gl_error();

            ImGuiIO& io = ImGui::GetIO();check_gl_error();
            auto& style = ImGui::GetStyle();check_gl_error();
            static std::string imgui_config = g.base_path + "CSHook.ini";
            io.IniFilename = imgui_config.c_str();

            io.MouseDrawCursor = false;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();check_gl_error();
            //ImGui::StyleColorsClassic();

            // Setup Platform/Renderer bindings
            ImGui_Impl_Init(g.main_window);check_gl_error();

            // Gear icon load
            int icon_width = 0, icon_height = 0;
            int n = 0;
            auto image = stbi_load_from_memory(gear_icon, gear_icon_length, &icon_width, &icon_height, &n, 0);

            glGenTextures(1, &g.gear_icon_id);check_gl_error();
            glBindTexture(GL_TEXTURE_2D, g.gear_icon_id);check_gl_error();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);check_gl_error();	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);check_gl_error();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);check_gl_error();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);check_gl_error();

            int32_t mode = GL_RGB;                                                                 // The surface mode
            if(n == 4) {
                mode = GL_RGBA;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, mode, icon_width, icon_height, 0, mode, GL_UNSIGNED_BYTE, image);check_gl_error();
            glGenerateMipmap(GL_TEXTURE_2D);check_gl_error();                                       // Not sure if generating mipmaps for a 2D game is necessary

            stbi_image_free(image);

            // Mirrorcam framebuffer / texture
            // Generate necessary buffer
            glGenFramebuffers(1, &g.mirrorcam_buffer);check_gl_error();
            glBindFramebuffer(GL_FRAMEBUFFER, g.mirrorcam_buffer);check_gl_error();

            // Generate texture
            glGenTextures(1, &g.mirrorcam_texture);check_gl_error();
            glBindTexture(GL_TEXTURE_2D, g.mirrorcam_texture);check_gl_error();

            // Texture settings
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 960, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);check_gl_error();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);check_gl_error();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);check_gl_error();

            // Generate depth buffer
            glGenRenderbuffers(1, &g.mirrorcam_depth_buffer);check_gl_error();
            glBindRenderbuffer(GL_RENDERBUFFER, g.mirrorcam_depth_buffer);check_gl_error();

            // Depth buffer settings
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 960);check_gl_error();

            // Attach texture and depth buffer to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g.mirrorcam_texture, 0);check_gl_error();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g.mirrorcam_depth_buffer);check_gl_error();

            // Check if complete
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                MessageBox(NULL, L"Frame buffer error!", L"", MB_OK);
                std::exit(0);
            }

            // Unbind buffer
            glBindTexture(GL_TEXTURE_2D, 0);check_gl_error();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);check_gl_error();

            g.first = false;
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);check_gl_error();
        
        glViewport(0, 0, viewport[2], viewport[3]);check_gl_error();

        glMatrixMode(GL_PROJECTION);check_gl_error();
        glPushMatrix();check_gl_error();
        glLoadIdentity();check_gl_error();

        // set the limits of our ortho projection
        glOrtho(0, viewport[2], viewport[3], 0, -1, 100);check_gl_error();

        glMatrixMode(GL_MODELVIEW);check_gl_error();
        glPushMatrix();check_gl_error();
        glLoadIdentity();check_gl_error();

        glMatrixMode(GL_TEXTURE);check_gl_error();
        glPushMatrix();check_gl_error();
        glLoadIdentity();check_gl_error();

        glDisable(GL_CULL_FACE);check_gl_error();
        glDisable(GL_DEPTH_TEST);check_gl_error();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);check_gl_error();
       
        ImGui_Impl_NewFrame();
        ImGui::NewFrame();

        // Render ESP
        features::visuals::instance().swap_buffers();
        features::miscelaneous::instance().swap_buffers();

        features::menu::instance().show_menu();

        ImGui::Render();
        ImGui_Impl_RenderDrawData(ImGui::GetDrawData());

        glEnable(GL_CULL_FACE);check_gl_error();
        glEnable(GL_DEPTH_TEST);check_gl_error();

        glMatrixMode(GL_TEXTURE);check_gl_error();
        glPopMatrix();check_gl_error();
        glMatrixMode(GL_MODELVIEW);check_gl_error();
        glPopMatrix();check_gl_error();
        glMatrixMode(GL_PROJECTION);check_gl_error();
        glPopMatrix();check_gl_error();

        features::miscelaneous::instance().swap_buffers_end();

        return original_func(hDc);
    }
}