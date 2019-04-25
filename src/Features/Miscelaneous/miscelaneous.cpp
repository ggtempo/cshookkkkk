#include "miscelaneous.hpp"
#include <glad/gl.h>
#include <random>
#include "../Utils/utils.hpp"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_impl.h"

#include "../Removals/removals.hpp"

namespace features
{
    void miscelaneous::create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();

        if (cmd->buttons & IN_JUMP && !(g.player_move->flags & FL_ONGROUND) && this->bhop_enabled)
        {
            cmd->buttons &= ~IN_JUMP;
        }
    }

    void miscelaneous::calc_ref_def(ref_params_t* params)
    {
        static auto& g = globals::instance();

        if (this->third_person_enabled && this->can_show())
        {
            // We push the view origin by 15 units up
            params->vieworg += (math::vec3(params->right) * 0);
            params->vieworg += (math::vec3(params->up) * 15);

            // Get the backwards vector
            math::vec3 backwards = (math::vec3(params->forward) * - 1);

            // Cast a ray from our player to 100 units behind him
            pmtrace_t trace = {};
            g.engine_funcs->pEventAPI->EV_SetTraceHull(2);
            g.engine_funcs->pEventAPI->EV_PlayerTrace(params->vieworg, params->vieworg + (backwards * 100), PM_WORLD_ONLY, -1, &trace);

            // Scale third person distance (100 units) by lenght of the ray (eg: we hit a wall halfway through => 0.5, we hit nothing => 1.0)
            float distance = trace.fraction * 100;

            // Move the view origin by distance units back
            params->vieworg += backwards * distance;
        }
        
        if (this->mirror_cam_enabled)
        {
            if (params->nextView == 1)
            {
                if(g.local_player_data.alive && g.engine_funcs->GetLocalPlayer()->curstate.solid != SOLID_NOT)
                {
                    // Normal view
                    // The VGUI hud is drawn after all of the views
                    // Because of that, nextView 0 ends up with no HUD
                    // We instead render the view 1 into the main framebuffer
                    // and render view 0 (technically the main view) to the mirror_cam framebuffer
                    // TODO: Hook vgui and switch the framebuffer to the default one
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);check_gl_error();
                    glDrawBuffer(GL_BACK);check_gl_error();

                    params->nextView = 0;
                }
            }   
            else if (params->nextView == 0)
            {
                if (g.local_player_data.alive && g.engine_funcs->GetLocalPlayer()->curstate.solid != SOLID_NOT)
                {
                    // Mirror cam view
                    GLenum buffers[1] = {GL_COLOR_ATTACHMENT0};
                    glBindFramebuffer(GL_FRAMEBUFFER, g.mirrorcam_buffer);check_gl_error();
                    glDrawBuffers(1, buffers);check_gl_error();
                    
                    params->nextView = 1;

                    params->viewport[0] = 0;
                    params->viewport[1] = 0;
                    params->viewangles[1] += 180;
                }
            }
        }
    }

    int miscelaneous::initiate_game_connection(void *ecx, int *data, int max_data, long long steam_id, int server_ip, short server_port, int secure)
    {
        using initiate_game_connection_fn = int(__thiscall*)(void *ecx, int *data, int max_data, long long steam_id, int server_ip, short server_port, int secure);
        static auto& g = globals::instance();
        static auto original_func = g.steamapi_hook->get_original_vfunc<initiate_game_connection_fn>(3);

        if (!this->steam_id_changer_enabled)
            return original_func(ecx, data, max_data, steam_id, server_ip, server_port, secure);


        data[20] = -1;
        data[21] = this->steam_id;

        return 768;
    }

    bool miscelaneous::is_thirdperson()
    {
        return this->third_person_enabled && this->can_show();
    }

    void miscelaneous::show_menu()
    {
        static auto& g = globals::instance();

        if (ImGui::Begin("Miscellaneous"))
        {
            ImGui::Columns(2);
                ImGui::Checkbox("Bhop enabled", &this->bhop_enabled);
                ImGui::Checkbox("Mirror cam", &this->mirror_cam_enabled);
                ImGui::Checkbox("Third person", &this->third_person_enabled);
                ImGui::Checkbox("Hide on screenshots", &this->hide_on_screenshot);

                float float_backtrack_time = g.backtrack_amount;
                ImGui::SliderFloat("Backtrack time", &float_backtrack_time, 0, 10);
                g.backtrack_amount = float_backtrack_time;

                ImGui::Checkbox("### SteamID changer enabled", &this->steam_id_changer_enabled);
                ImGui::SameLine();
                if (ImGui::Button("Randomize"))
                {
                    static std::random_device dev;
                    static std::mt19937 gen(dev());
                    static std::uniform_int_distribution<int> dist(1000000,7000000);

                    this->steam_id = dist(gen);
                }
                ImGui::SameLine();
                ImGui::InputInt("Steam ID changer", &this->steam_id, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);

            ImGui::NextColumn();
                features::removals::instance().show_menu();
            ImGui::Columns(1);
        }
        ImGui::End();
    }

    bool miscelaneous::can_show()
    {
        return (!this->hide_on_screenshot || !(this->taking_screenshot || this->taking_snapshot));
    }

    void miscelaneous::swap_buffers()
    {
        static auto& g = globals::instance();

        if (this->mirror_cam_enabled && this->can_show())
        {
            ImGui::Begin("Mirrorcam");
                auto pos = ImGui::GetCursorScreenPos();
                auto size = ImGui::GetContentRegionMax();
                auto bottom_right = ImVec2(pos.x + size.x, pos.y + size.y);
                ImGui::GetWindowDrawList()->AddImage((void*)g.mirrorcam_texture, pos, bottom_right, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();
        }
    }

    void miscelaneous::swap_buffers_end()
    {
        static auto& g = globals::instance();

        if (this->taking_screenshot)
        {
            this->taking_screenshot = false;
            reinterpret_cast<command_t>(g.original_screenshot)();
        }

        if (this->taking_snapshot)
        {
            this->taking_snapshot = false;
            reinterpret_cast<command_t>(g.original_snapshot)();
        }
    }

    void miscelaneous::on_screenshot()
    {
        this->taking_screenshot = true;
    }

    void miscelaneous::on_snapshot()
    {
        this->taking_snapshot = true;
    }
}

