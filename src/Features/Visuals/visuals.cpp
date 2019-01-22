#include "visuals.hpp"
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

        bool valid = false;

        valid = entity && entity->model && entity->player && entity != local && this->chams;
        valid = valid && ((g.local_player_data.team != g.player_data[entity->index].team) || this->chams_team);

        if (valid)
        {
            glDepthRange(0.05, 0.1);
            glEnable(GL_TEXTURE_2D);
            ecx->StudioRenderFinal();


            glDisable(GL_TEXTURE_2D);
            g.engine_studio->SetForceFaceFlags( g.s_nf );
            g.engine_funcs->pTriAPI->RenderMode( g.render_mode );

            if (g.local_player_data.team != g.player_data[entity->index].team)
            {
                glColor4f(this->enemy_color.r, this->enemy_color.g, this->enemy_color.b, this->enemy_color.a);
            }
            else
            {
                glColor4f(this->team_color.r, this->team_color.g, this->team_color.b, this->team_color.a);
            }
            

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            ecx->StudioRenderFinal();

            glDisable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);

            // Bone test
            glPointSize(6.0);
            
            for (auto& [key, hitbox] : g.player_data[entity->index].hitboxes)
            {
                vec3_t transformed_bbmin = hitbox.matrix.transform_vec3(hitbox.box.bbmin);
                vec3_t transformed_bbmax = hitbox.matrix.transform_vec3(hitbox.box.bbmax);
                vec3_t center = (transformed_bbmin + transformed_bbmax) * 0.5;

                if (hitbox.visible)
                {
                    glColor3f(0.0, 1.0, 0.0);
                }
                else
                {
                    glColor3f(1.0, 0.0, 0.0);
                }
                glBegin(GL_POINTS);
                glVertex3f(center.x, center.y, center.z);
                glEnd();
            }
            

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glDepthRange(0.1, 1);
        }
        else if ((entity == local) && (entity->index == local->index) || (entity == g.engine_funcs->GetViewModel()))
        {
            // Draw our player on top of everything
            glDepthRange(0, 0.05);
            ecx->StudioRenderFinal();
            glDepthRange(0.1, 1.0);
        }
        else
        {
            //ecx->StudioRenderFinal( );
            original_func(ecx);
        }
    }

    void visuals::show_menu()
    {
        ImGui::Begin("ESP");
            ImGui::Checkbox("Chams", &this->chams);
            ImGui::Checkbox("Chams team", &this->chams_team);

            ImGui::ColorEdit4("Enemy color", this->enemy_color.clr, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Team color", this->team_color.clr, ImGuiColorEditFlags_NoInputs);
        ImGui::End();
    }
}