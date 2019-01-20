#include "antiaim.hpp"
#include "../../ImGui/imgui.h"

namespace features
{
    void anti_aim::create_move(float frametime, usercmd_t *cmd, int active)
    {
        if (this->enabled && !(cmd->buttons & IN_ATTACK))
        {
            static float angle = 0.0f;

            auto view = cmd->viewangles;
            auto new_view = math::vec3{-89.0f, angle, 0.0f};
            auto move = vec3_t{cmd->forwardmove, cmd->sidemove, cmd->upmove};
            cmd->viewangles = new_view;

            auto new_move = math::correct_movement(view, new_view, move);
            
            // Reset movement bits
            cmd->buttons &= ~IN_FORWARD;
            cmd->buttons &= ~IN_BACK;
            cmd->buttons &= ~IN_LEFT;
            cmd->buttons &= ~IN_RIGHT;

            cmd->forwardmove = new_move.x;
            cmd->sidemove = new_move.y;
            cmd->upmove = new_move.z;

            while (cmd->viewangles.y > 360)
            {
                cmd->viewangles.y -= 360;
                angle -= 360;
            }

            while (cmd->viewangles.y < 0)
            {
                cmd->viewangles.y += 360;
                angle += 360;
            }

            cmd->viewangles.z = 0;

            angle += 50;
        }
    }

    void anti_aim::show_menu()
    {
        ImGui::Begin("Anti-Aim");
        ImGui::End();
    }
}