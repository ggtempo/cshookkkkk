#include "antiaim.hpp"
#include "../../ImGui/imgui.h"

namespace features
{
    void anti_aim::create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();

        if (this->enabled && !(cmd->buttons & IN_ATTACK) && g.player_move->movetype != MOVETYPE_FLY)
        {
            static float angle = 0.0;
            auto view = cmd->viewangles;
            auto new_view = view;
            auto move = vec3_t{cmd->forwardmove, cmd->sidemove, cmd->upmove};

            auto correct_view = true;

            // Determine the correct pitch angle
            switch (this->pitch_mode)
            {
                case aa_mode_pitch::down_emotion:
                    new_view.x = -89.0f;
                    break;

                case aa_mode_pitch::down_unsafe:
                    new_view.x = -179.0f;
                    correct_view = false;
                    break;

                case aa_mode_pitch::up_emotion:
                    new_view.x = 89.0f;
                    break;

                case aa_mode_pitch::up_unsafe:
                    new_view.x = 179.0f;
                    correct_view = false;
                    break;
            }

            // Determine the correct yaw angle
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

            // Wrap spinbot angle to 360 degrees
            if (angle > 360)
                angle -= 360;

            // If we should correct the angles (eg: safe angles)
            // Correct them
            if (correct_view)
                cmd->viewangles = new_view.normalize_angle();
            else
                cmd->viewangles = new_view;
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

        if (ImGui::Begin("Anti-Aim"))
        {
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

                    if (selected)
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

                    if (selected)
                        ImGui::SetItemDefaultFocus();   // Scroll to the currently selected otion
                }
                ImGui::EndCombo();
            }
        }
        ImGui::End();
    }
}