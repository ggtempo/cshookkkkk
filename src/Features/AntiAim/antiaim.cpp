#include "antiaim.hpp"
#include "../../ImGui/imgui.h"

namespace features
{
    void anti_aim::create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        static bool send_switch = true;

        if (this->enabled && !(cmd->buttons & IN_ATTACK) && (g.player_move->movetype != MOVETYPE_FLY))
        {
            static float angle = 0.0;
            auto view = cmd->viewangles;
            auto new_view = view;
            auto move = vec3_t{cmd->forwardmove, cmd->sidemove, cmd->upmove};

            auto correct_view = true;

            auto fake_angles = this->fake_yaw_mode != aa_mode_yaw::off;
            if (fake_angles && g.send_packet)
            {
                send_switch = !send_switch;
                g.send_packet = send_switch;
            }
            
            // No need to account for pitch using fake angles
            // Determine the correct pitch angle
            switch (this->pitch_mode)
            {
                case aa_mode_pitch::down_emotion:
                    new_view.x = -emotion_angle;
                    break;

                case aa_mode_pitch::down_unsafe:
                    new_view.x = unsafe_angle;
                    correct_view = false;
                    break;

                case aa_mode_pitch::down_lisp:
                    new_view.x = lisp_angle;
                    correct_view = false;
                    break;

                case aa_mode_pitch::up_emotion:
                    new_view.x = emotion_angle;
                    break;

                case aa_mode_pitch::up_unsafe:
                    new_view.x = -unsafe_angle;
                    correct_view = false;
                    break;

                case aa_mode_pitch::up_lisp:
                    new_view.x = -lisp_angle;
                    correct_view = false;
                    break;

                case aa_mode_pitch::user_defined:
                    new_view.x = this->user_pitch;
                    break;
            }


            if (fake_angles && !send_switch)
            {
                switch (this->fake_yaw_mode)
                {
                    case aa_mode_yaw::forwards:
                        new_view.y -= 0.0f;
                        break;

                    case aa_mode_yaw::backwards:
                        new_view.y -= 180.0f;
                        break;
                    
                    case aa_mode_yaw::left:
                        new_view.y += 90;
                        break;

                    case aa_mode_yaw::right:
                        new_view.y -= 90.0f;
                        break;

                    case aa_mode_yaw::spin:
                        new_view.y = angle - 180;
                        angle += 20;
                        break;

                    case aa_mode_yaw::user_defined:
                            new_view.y = this->user_fake_yaw;
                        break;
                }
            }
            else
            {
                // Determine the correct yaw angle
                switch (this->yaw_mode)
                {
                    case aa_mode_yaw::forwards:
                        new_view.y -= 0.0f;
                        break;

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

                    case aa_mode_yaw::user_defined:
                        new_view.y = this->user_yaw;
                        break;
                }
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

    void anti_aim::post_move_fix(usercmd_t* cmd, math::vec3& new_move)
    {
        static auto& g = globals::instance();

        if (this->enabled && !(cmd->buttons & IN_ATTACK) && (g.player_move->movetype != MOVETYPE_FLY) &&
            (this->pitch_mode == aa_mode_pitch::down_lisp || this->pitch_mode == aa_mode_pitch::up_lisp))
        {
            new_move.x *= -1;
        }
    }

    void anti_aim::show_menu()
    {
        static auto& g = globals::instance();

        struct option
        {
            const char* name;
            const char* tooltip;
        };

        const static std::vector<option> pitch_modes = {
            {"Off", "Default pitch"},
            {"Down (emotion)", "Pitch is set to point down (88.0f)"},
            {"Down (unsafe)", "Pitch is set to point down (overflows back to pointing up)"},
            {"Down (lisp)", "Pitch is set to point down (overflows back to 0)"},
            {"Up (emotion)", "Pitch is set to point up (-88.0f)"},
            {"Up (unsafe)", "Pitch is set to point up (overflows back to pointing down)"},
            {"Up (lisp)", "Pitch is set to point up  (overflows back to 0)"},
            {"Custom", "User defined"}
        };

        const static std::vector<option> yaw_modes = {
            {"Off", "Default yaw"},
            {"Forwards", "Yaw is offset by 0 (Player is walking forwards)"},
            {"Backwards", "Yaw is offset by 180 (Player is walking backwards)"},
            {"Left", "Yaw is offset by +90 (Player is walking sideways)"},
            {"Right", "Yaw is offset by -90 (Player is walking sideways)"},
            {"Spin", "Yaw is spinning"},
            {"Custom", "User defined"}
        };

        if (ImGui::Begin("Anti-Aim"))
        {
            ImGui::Checkbox("Enabled", &this->enabled);
            ImGui::Columns(2);
            
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

            if (ImGui::BeginCombo("Fake yaw mode", yaw_modes[(int)this->fake_yaw_mode].name))
            {
                for (size_t i = 0; i < yaw_modes.size(); i++)
                {
                    bool selected = (i == (size_t)this->fake_yaw_mode);

                    if (ImGui::Selectable(yaw_modes[i].name, selected))
                        this->fake_yaw_mode = (aa_mode_yaw)i;    // If we selected this mode, remember it

                    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                        ImGui::SetTooltip(yaw_modes[i].tooltip);  // If we are hovering over the option, display it's tooltip

                    if (selected)
                        ImGui::SetItemDefaultFocus();   // Scroll to the currently selected otion
                }
                ImGui::EndCombo();
            }

            ImGui::NextColumn();

            ImGui::SliderFloat("Custom pitch", &this->user_pitch, -89.0, 89.0);
            ImGui::SliderFloat("Custom yaw", &this->user_yaw, -180.0, 180.0);
            ImGui::SliderFloat("Custom fake yaw", &this->user_fake_yaw, -180, 180);
            //ImGui::SliderFloat("Custom roll", &this->user_roll, 0.0, 360.0); ... once i figure out how to modify the roll angle

            ImGui::Columns(1);
        }
        ImGui::End();
    }
}