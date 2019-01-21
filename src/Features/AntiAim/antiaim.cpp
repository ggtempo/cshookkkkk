#include "antiaim.hpp"
#include "../../ImGui/imgui.h"

namespace features
{
    void anti_aim::create_move(float frametime, usercmd_t *cmd, int active)
    {
        if (this->enabled && !(cmd->buttons & IN_ATTACK))
        {
            static float angle = 0.0;
            auto view = cmd->viewangles;
            auto new_view = view;
            auto move = vec3_t{cmd->forwardmove, cmd->sidemove, cmd->upmove};

            switch (this->pitch_mode)
            {
                case aa_mode_pitch::down_emotion:
                    new_view.x = -89.0f;
                    break;

                case aa_mode_pitch::down_unsafe:
                    new_view.x = -179.0f;
                    move.x *= -1;
                    break;

                case aa_mode_pitch::up_emotion:
                    new_view.x = 89.0f;
                    break;

                case aa_mode_pitch::up_unsafe:
                    new_view.x = 179.0f;
                    move.x *= -1;
                    break;
            }

            switch (this->yaw_mode)
            {
                case aa_mode_yaw::backwards:
                    new_view.y -= 180.0f;
                    break;
                
                case aa_mode_yaw::left:
                    new_view.y += 90.0f;
                    break;

                case aa_mode_yaw::right:
                    new_view.y -= 90.0f;
                    break;

                case aa_mode_yaw::spin:
                    new_view.y = angle;
                    angle += 20;
                    break;
            }

            while (angle >= 360.0f)
                angle -= 360.0f;

            while (angle < 0.0f)
                angle += 360.0f;

            
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

            cmd->viewangles.z = 0;
        }
    }

    void anti_aim::show_menu()
    {
        struct option
        {
            const char* name;
            const char* tooltip;
        };

        const static std::vector<option> pitch_modes = {
            {"Off", "Default pitch"},
            {"Down (emotion)", "Pitch is set to -89"},
            {"Down (unsafe)", "Pitch is set to -179 (outside the normal range)"},
            {"Up (emotion)", "Pitch is set to 89"},
            {"Up (unsafe)", "Pitch is set to 179 (outside the normal range)"}
        };

        const static std::vector<option> yaw_modes = {
            {"Off", "Default yaw"},
            {"Backwards", "Yaw is offset by 180 (Player is walking backwards)"},
            {"Left", "Yaw is offset by +90 (Player is walking sideways)"},
            {"Right", "Yaw is offset by -90 (Player is walking sideways)"},
            {"Spin", "Yaw is spinning"}
        };

        ImGui::Begin("Anti-Aim");
            ImGui::Checkbox("Enabled", &this->enabled);
            
            if (ImGui::BeginCombo("Pitch mode", pitch_modes[(int)this->pitch_mode].name))
            {
                for (size_t i = 0; i < pitch_modes.size(); i++)
                {
                    bool selected = (i == (size_t)this->pitch_mode);
                    if (ImGui::Selectable(pitch_modes[i].name, selected))
                        this->pitch_mode = (aa_mode_pitch)i;    // If we selected this mode, remember it

                    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                        ImGui::SetTooltip(pitch_modes[i].tooltip);  // If we are hovering over the option, display it's tooltip

                    if (select)
                        ImGui::SetItemDefaultFocus();   // Scroll to the currently selected otion
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo("Yaw mode", yaw_modes[(int)this->yaw_mode].name))
            {
                for (size_t i = 0; i < yaw_modes.size(); i++)
                {
                    bool selected = (i == (size_t)this->yaw_mode);

                    if (ImGui::Selectable(yaw_modes[i].name, selected))
                        this->yaw_mode = (aa_mode_yaw)i;    // If we selected this mode, remember it

                    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                        ImGui::SetTooltip(yaw_modes[i].tooltip);  // If we are hovering over the option, display it's tooltip

                    if (select)
                        ImGui::SetItemDefaultFocus();   // Scroll to the currently selected otion
                }
                ImGui::EndCombo();
            }
        ImGui::End();
    }
}