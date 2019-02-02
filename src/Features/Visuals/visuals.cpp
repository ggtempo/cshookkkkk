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

        if (this->chams                                                                             &&
            (g.player_data[entity->index].alive && !g.player_data[entity->index].dormant)           &&
            ((g.player_data[entity->index].team != g.local_player_data.team) || this->chams_team)   &&
            (entity->index != local->index) && (entity != local))
        {
            glDepthRange(0.05, 0.1);check_gl_error();

            if (this->chams_mode == chams_modes::CHAMS_NOTEXTURE)
            {
                glDisable(GL_TEXTURE_2D);check_gl_error();
            }
            else
            {
                glEnable(GL_TEXTURE_2D);check_gl_error();
            }
            
            ecx->StudioRenderFinal();


            glDisable(GL_TEXTURE_2D);check_gl_error();
            //glEnable(GL_COLORS);check_gl_error();
            g.engine_studio->SetForceFaceFlags( STUDIO_NF_CHROME );
            g.engine_funcs->pTriAPI->RenderMode( render_modes::kRenderNormal );

            if (g.local_player_data.team != g.player_data[entity->index].team)
            {
                glColor4f(this->enemy_color.r, this->enemy_color.g, this->enemy_color.b, this->enemy_color.a);
            }
            else
            {
                glColor4f(this->team_color.r, this->team_color.g, this->team_color.b, this->team_color.a);
            }
            

            glEnable(GL_BLEND);check_gl_error();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);check_gl_error();

            ecx->StudioRenderFinal();

            //glDisable(GL_COLORS);check_gl_error();
            glDisable(GL_BLEND);check_gl_error();
            glDisable(GL_DEPTH_TEST);check_gl_error();



            glEnable(GL_DEPTH_TEST);check_gl_error();
            glEnable(GL_TEXTURE_2D);check_gl_error();
            glDepthRange(0.1, 1);check_gl_error();
        }
        else if ((entity == local) && (entity->index == local->index) || (entity == g.engine_funcs->GetViewModel()))
        {
            // Draw our player on top of everything
            glDepthRange(0, 0.05);check_gl_error();
            ecx->StudioRenderFinal();
            glDepthRange(0.1, 1.0);check_gl_error();
        }
        else
        {
            //ecx->StudioRenderFinal( );
            original_func(ecx);
        }
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
            ImGui::Checkbox("Chams", &this->chams);
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

            ImGui::ColorEdit4("Enemy color", this->enemy_color.clr, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Team color", this->team_color.clr, ImGuiColorEditFlags_NoInputs);
        }
        ImGui::End();
    }
}