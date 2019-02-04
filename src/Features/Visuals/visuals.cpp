#include "visuals.hpp"
#include "../../ImGui/imgui_impl.h"
#include "../../ImGui/imgui.h"
#include <glad/gl.h>

namespace features
{
    void visuals::studio_render_model(CStudioModelRenderer* ecx)
    {
        typedef void(__thiscall* fnStudioRenderModel)(void* ecx);
        static auto& g = globals::instance();
        static auto original_func = g.studio_model_renderer_hook->get_original_vfunc<fnStudioRenderModel>(18);
        auto entity = g.engine_studio->GetCurrentEntity();
        auto local = g.engine_funcs->GetLocalPlayer();

        // If the target is valid and the player wants chams, render them
        if (this->chams                                                                             &&
            (g.player_data[entity->index].alive && !g.player_data[entity->index].dormant)           &&
            ((g.player_data[entity->index].team != g.local_player_data.team) || this->chams_team)   &&
            (entity->index != local->index) && (entity != local)                                    &&                    
            (!g.hide_on_screenshot || !(g.taking_screenshot || g.taking_snapshot)))
        {
            // If we should render the chams through the walls, use only the top portion of the depth buffer
            if (this->chams_through_walls)
            {
                glDepthRange(0.0, 0.1);check_gl_error();
            }
            else
            {
                glDepthRange(0.0, 1.0);check_gl_error();
            }

            // Disable/Enable textures based on the users choice
            if (this->chams_mode == chams_modes::CHAMS_NOTEXTURE)
            {
                glDisable(GL_TEXTURE_2D);check_gl_error();
            }
            else
            {
                glEnable(GL_TEXTURE_2D);check_gl_error();
            }
            
            // Render model once, either textured/untextured, behind the wall or not
            ecx->StudioRenderFinal();

            // Disable textures
            // Color rendering
            glDisable(GL_TEXTURE_2D);check_gl_error();
            g.engine_studio->SetForceFaceFlags( STUDIO_NF_CHROME );
            g.engine_funcs->pTriAPI->RenderMode( render_modes::kRenderNormal );

            // Get the apropriate color
            if (g.local_player_data.team != g.player_data[entity->index].team)
            {
                glColor4f(this->enemy_color.r, this->enemy_color.g, this->enemy_color.b, this->enemy_color.a);check_gl_error();
            }
            else
            {
                glColor4f(this->team_color.r, this->team_color.g, this->team_color.b, this->team_color.a);check_gl_error();
            }
            
            // Blend this color and the previous color (eg: untextured player model and just colors)
            glEnable(GL_BLEND);check_gl_error();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);check_gl_error();

            // Render just the colored overlay of the player
            ecx->StudioRenderFinal();

            // Disable blending
            glDisable(GL_BLEND);check_gl_error();


            // Enable depth test
            glEnable(GL_DEPTH_TEST);check_gl_error();
            // Restore texture rendering
            glEnable(GL_TEXTURE_2D);check_gl_error();
            // Restore depth range
            glDepthRange(0.0, 1.0);check_gl_error();
        }
        else if ((entity == local) && (entity->index == local->index) || (entity == g.engine_funcs->GetViewModel()))
        {
            // Draw our player on top of everything
            glDepthRange(0.0, 0.1);check_gl_error();
            ecx->StudioRenderFinal();
            glDepthRange(0.1, 1.0);check_gl_error();
        }
        else
        {
            // If invalid target, just let the game render it
            glDepthRange(0.0, 1.0);check_gl_error();
            original_func(ecx);
            glDepthRange(0.0, 1.0);check_gl_error();
        }
    }

    void draw_box_esp(ImDrawList* draw_list, const ImVec2& position, const ImVec2& size, const ImColor& color_inner = ImColor(1.0f, 0.0f, 0.0f, 1.0f), const ImColor& color_outer = ImColor(0.0f, 0.0f, 0.0f, 1.0f))
    {
        // Coordinates
        auto top_left_outer = ImVec2(position.x - 1, position.y - 1);
        auto top_left_inner = ImVec2(position.x + 1, position.y + 1);
        auto top_left_center = ImVec2(position.x, position.y);

        auto bottom_right_inner = ImVec2(position.x + size.x - 1, position.y + size.y - 1);
        auto bottom_right_outer = ImVec2(position.x + size.x + 1, position.y + size.y + 1);
        auto bottom_right_center = ImVec2(position.x + size.x, position.y + size.y);

        // Left outer border
        //draw_list->AddRectFilled(top_left_outer, bottom_right, color_outer);
        draw_list->AddRect(top_left_outer, bottom_right_outer, color_outer);
        draw_list->AddRect(top_left_inner, bottom_right_inner, color_outer);
        draw_list->AddRect(top_left_center, bottom_right_center, color_inner);
    }

    math::vec3 screen_project(const math::vec3& origin, const math::vec3& screen_size)
    {
            //#define XPROJECT(x)	( (1.0f+(x))*ScreenWidth*0.5f )
            //#define YPROJECT(y)   ( (1.0f-(y))*ScreenHeight*0.5f )
            
            math::vec3 result = {};
            result.x = (1.0f + origin.x) * screen_size.x * 0.5f;
            result.y = (1.0f - origin.y) * screen_size.y * 0.5f;
            result.z = 0.0f;
            return result;
    }

    void visuals::swap_buffers()
    {
        static auto& g = globals::instance();
        // Get local player
        auto local = g.engine_funcs->GetLocalPlayer();

        auto io = ImGui::GetIO();

        if (!(!g.hide_on_screenshot || !(g.taking_screenshot || g.taking_snapshot)))
            return; // Early exit when taking a screenshot

        // Creating a fullscreen overlay window with no inputs, title bar, etc..
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(io.DisplaySize);
        
        // Disable all padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0, 0.0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0, 0.0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        // Disable all input and decoration
        constexpr auto flags =  ImGuiWindowFlags_NoBackground   | ImGuiWindowFlags_NoBringToFrontOnFocus    |
                                ImGuiWindowFlags_NoCollapse     | ImGuiWindowFlags_NoDecoration             |
                                ImGuiWindowFlags_NoTitleBar     | ImGuiWindowFlags_NoInputs                 |
                                ImGuiWindowFlags_NoSavedSettings;
        
        // Only render the window while not taking a screenshot (if protecting against screenshots)
        if (!ImGui::Begin("Kek", NULL, flags))
        {
            // Early exit for when the window is hidden (Not really possible)
            ImGui::End();
            ImGui::PopStyleVar(4);
            return;
        }

        auto draw_list = ImGui::GetWindowDrawList();
        if (!draw_list || !local || !this->box_esp)
        {
            // Early exit for when the window is hidden (Not really possible)
            ImGui::End();
            ImGui::PopStyleVar(4);
            return;
        }

        // Go through each player
        for (size_t i = 1; i < g.engine_funcs->GetMaxClients(); i++)
        {
            auto entity = g.engine_funcs->GetEntityByIndex(i);

            // Check if entity is valid
            if (!entity || !entity->index)
                continue;

            if ((g.player_data[entity->index].alive && !g.player_data[entity->index].dormant)           &&
                ((g.player_data[entity->index].team != g.local_player_data.team) || this->box_esp_team) &&
                (entity->index != local->index) && (entity != local))
            {
                // Get head hitbox 
                auto& head_matrix = g.player_data[entity->index].hitboxes[hitbox_numbers::head].matrix;
                auto& head_box = g.player_data[entity->index].hitboxes[hitbox_numbers::head].box;

                // Find the center
                auto head_bbmax = head_matrix.transform_vec3(head_box.bbmax);
                auto head_bbmin = head_matrix.transform_vec3(head_box.bbmin);
                auto head = (head_bbmin + head_bbmax) * 0.5;
                head.z += 8;            // Offset by about 8 up, otherwise box will reach only halfway through the head

                bool ducking = ((entity->curstate.maxs[2] - entity->curstate.mins[2]) < 54 ? true : false);
                auto feet = entity->origin;
                feet.z -= ducking ? 14 : 34;

                math::vec3 head_screen = {};
                math::vec3 feet_screen = {};

                // Get screen coordinates (0 - 1)
                auto head_result = g.engine_funcs->pTriAPI->WorldToScreen(head, head_screen);
                auto feet_result = g.engine_funcs->pTriAPI->WorldToScreen(feet, feet_screen);
                
                // Project to actual screen coordinates (0 - resolution)
                auto head_pos = screen_project(head_screen, {io.DisplaySize.x, io.DisplaySize.y, 0.0f});
                auto feet_pos = screen_project(feet_screen, {io.DisplaySize.x, io.DisplaySize.y, 0.0f});

                // If projection is out of our screen, skip this player
                if (head_result || feet_result)
                    continue;

                // Find the difference between head and feet
                constexpr float width_to_height_ratio = 0.4f;
                auto height_difference = feet_pos.y - head_pos.y;
                auto width = height_difference * width_to_height_ratio; // Box is about 0.4 times as wide as it is high

                // Convert to screen coordinates
                auto pos = ImVec2(head_pos.x - (width / 2), head_pos.y);
                auto size = ImVec2(width, height_difference);

                // Get corresponding color
                ImColor color = ImColor(0.0f, 0.0f, 0.0f, 0.0f);

                if (g.player_data[entity->index].team != g.local_player_data.team)
                {
                    color = ImColor(this->enemy_color.r, this->enemy_color.g, this->enemy_color.b, 1.0f);
                }
                else
                {
                    color = ImColor(this->team_color.r, this->team_color.g, this->team_color.b, 1.0f);
                }

                // Draw the box itself
                draw_box_esp(draw_list, pos, size, color);

                // If we should draw name
                if (this->name_esp)
                {
                    // Get the size
                    auto text_size = ImGui::CalcTextSize(g.player_data[entity->index].name);

                    // Get correct position
                    auto bottom = ImVec2(pos.x + (size.x / 2), pos.y + size.y + 5);
                    auto text_pos = ImVec2(bottom.x - (text_size.x / 2), bottom.y);

                    // Render the text
                    draw_list->AddText(text_pos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), g.player_data[entity->index].name);
                }
            }
        }

        // Restore default style
        ImGui::End();
        ImGui::PopStyleVar(4);
    }

    void visuals::show_menu()
    {
        struct option
        {
            const char* name;
            const char* tooltip;
        };

        const static std::vector<option> modes = {
            {"Textured", "Players render with their texture"},
            {"Untextured", "Players render using flat color"},
        };

        if (ImGui::Begin("ESP"))
        {
            ImGui::Columns(2);
            {
                ImGui::Checkbox("Chams", &this->chams);
                ImGui::Checkbox("Chams through walls", &this->chams_through_walls);
                ImGui::Checkbox("Chams team", &this->chams_team);
                
                if (ImGui::BeginCombo("Chams mode", modes[(int)this->chams_mode].name))
                {
                    for (size_t i = 0; i < modes.size(); i++)
                    {
                        bool selected = (i == (size_t)this->chams_mode);
                        if (ImGui::Selectable(modes[i].name, selected))
                            this->chams_mode = (chams_modes)i;    // If we selected this mode, remember it

                        if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                            ImGui::SetTooltip(modes[i].tooltip);  // If we are hovering over the option, display it's tooltip

                        if (select)
                            ImGui::SetItemDefaultFocus();   // Scroll to the currently selected otion
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::NextColumn();
            {
                ImGui::Checkbox("Box ESP", &this->box_esp);
                ImGui::Checkbox("ESP team", &this->box_esp_team);
                ImGui::Checkbox("Name ESP", &this->name_esp);
            }

            ImGui::Columns(1);

            ImGui::ColorEdit4("Enemy color", this->enemy_color.clr, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Team color", this->team_color.clr, ImGuiColorEditFlags_NoInputs);
        }
        ImGui::End();
    }
}