#include "removals.hpp"
#include "../../ImGui/imgui.h"

namespace features
{
    void removals::create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto lp = g.engine_funcs->GetLocalPlayer();

        this->no_recoil(frametime, cmd, active);

        if (this->no_spread_enabled)
        {
            if (this->no_spread_method == no_spread_methods::nospread1)
            {
                this->no_spread(frametime, cmd, active);
            }
            else if (this->no_spread_method == no_spread_methods::nospread2)
            {
                this->no_spread_2(frametime, cmd, active);
            }
        }
        
    }

    void removals::calc_ref_def(ref_params_t* params)
    {
        static auto& g = globals::instance();

        g.punch_angles.x = params->punchangle[0];
        g.punch_angles.y = params->punchangle[1];
        g.punch_angles.z = params->punchangle[2];

        if (this->no_visual_recoil)
        {
            params->punchangle[0] = 0.0f;
            params->punchangle[1] = 0.0f;
            params->punchangle[2] = 0.0f;
        }
    }

    void removals::show_menu()
    {
        struct option
        {
            const char* name;
            const char* tooltip;
        };

        const static std::vector<option> spread_modes = {
            {"No spread 1", "Default method"},
            {"No spread 2", "More precise method"}
        };

        // No own window, belongs to the misc window
        ImGui::Checkbox("Visual no recoil", &this->no_visual_recoil);
        ImGui::Checkbox("No recoil", &this->no_recoil_enabled);
        ImGui::Checkbox("###No spread enabled", &this->no_spread_enabled);
        ImGui::SameLine();
        if (ImGui::BeginCombo("No spread mode", spread_modes[(int)this->no_spread_method].name))
        {
            for (size_t i = 0; i < spread_modes.size(); i++)
            {
                bool selected = (i == (size_t)this->no_spread_method);
                if (ImGui::Selectable(spread_modes[i].name, selected))
                    this->no_spread_method = (no_spread_methods)i;    // If we selected this mode, remember it

                if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                    ImGui::SetTooltip(spread_modes[i].tooltip);  // If we are hovering over the option, display it's tooltip

                if (selected)
                    ImGui::SetItemDefaultFocus();   // Scroll to the currently selected otion
            }
            ImGui::EndCombo();
        }
    }

    void removals::no_recoil(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto lp = g.engine_funcs->GetLocalPlayer();

        if (this->no_recoil_enabled && (cmd->buttons & IN_ATTACK) && custom::is_gun(g.local_player_data.weapon.id) &&
            (g.local_player_data.weapon.next_attack <= 0.0) && (g.local_player_data.weapon.next_primary_attack <= 0.0) &&
            !g.local_player_data.weapon.in_reload)
        {
            cmd->viewangles -= g.punch_angles * 2;
            cmd->viewangles.z = 0.0f;

            cmd->viewangles.normalize_angle();
        }
    }

    void removals::no_spread(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto lp = g.engine_funcs->GetLocalPlayer();

        if (this->no_spread_enabled && (cmd->buttons & IN_ATTACK) && custom::is_gun(g.local_player_data.weapon.id) &&
            (g.local_player_data.weapon.next_attack <= 0.0) && (g.local_player_data.weapon.next_primary_attack <= 0.0) &&
            !g.local_player_data.weapon.in_reload)
        {
            // Get necessary info
            auto info = get_weapon_info((custom::weapon_id)g.local_player_data.weapon.id);
            float velocity = math::vec3(g.local_player_data.velocity.x, g.local_player_data.velocity.y, 0.0f).length();
            auto spread = custom::get_spread(g.local_player_data.weapon.id, info->m_flAccuracy, velocity,
                            g.player_move->flags & FL_ONGROUND, g.player_move->flags & FL_DUCKING,
                            lp->curstate.fov, info->m_iWeaponState);
            unsigned int shared_rand = g.local_player_data.weapon.seed;

            math::vec3 view_angles = cmd->viewangles.normalize_angle();

            auto spread_vec = custom::get_spread_vec(shared_rand, spread);

            math::vec3 forward, right, up, direction;
            math::vec3 temp = {0.0, 0.0, 0.0};
            temp.to_vectors(forward, right, up);

            direction = (forward + (right * spread_vec.x) + (up * spread_vec.y)).normalize();
            auto angles = direction.to_angles();
            angles.x -= view_angles.x;

            angles.normalize_angle();
            angles.transpose(forward, right, up);
            direction = forward;

            angles = direction.to_angles(up);
            angles.normalize_angle();
            angles.y += view_angles.y;
            angles.normalize_angle();

            cmd->viewangles = angles;
        }
    }

    void removals::no_spread_2(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto lp = g.engine_funcs->GetLocalPlayer();

        if (this->no_spread_enabled && (cmd->buttons & IN_ATTACK) && custom::is_gun(g.local_player_data.weapon.id) &&
            (g.local_player_data.weapon.next_attack <= 0.0) && (g.local_player_data.weapon.next_primary_attack <= 0.0) &&
            !g.local_player_data.weapon.in_reload)
        {
            // Get necessary info
            auto info = get_weapon_info((custom::weapon_id)g.local_player_data.weapon.id);
            float velocity = math::vec3(g.local_player_data.velocity.x, g.local_player_data.velocity.y, 0.0f).length();
            auto spread = custom::get_spread(g.local_player_data.weapon.id, info->m_flAccuracy, velocity,
                            g.player_move->flags & FL_ONGROUND, g.player_move->flags & FL_DUCKING,
                            0.0f, info->m_iWeaponState);
            unsigned int shared_rand = g.local_player_data.weapon.seed;

            math::vec3 view_angles = cmd->viewangles.normalize_angle();

            auto spread_vec = custom::get_spread_vec(shared_rand, spread);
            
            math::vec3 forward, right, up;
            math::vec3 angle_zero = {0.0, 0.0, 0.0};

            angle_zero.to_vectors(forward, right, up);
            math::vec3 direction = forward + (right * -spread_vec.x ) + (up * -spread_vec.y);
            direction.normalize();

            math::vec3 angles = direction.to_angles();
            angles.normalize_euler_angle();

            angles.to_vectors(forward, right, up);

            double cosine_input = view_angles.x * (pi * 2 / 360);
            double cosine_pitch = std::cos(cosine_input);
            double yaw_temp = 0;

            if (cosine_pitch != 0)
                yaw_temp = 1.0 / cosine_pitch; // Inverse cosine pitch

            math::vec3 right_input = {};
            math::vec3 right_input_2 = {};

            yaw_temp *= direction.y;
            
            if (yaw_temp >= 1 || yaw_temp <= -1)
            {
                right_input.y = 1 / yaw_temp;
                right_input.x = 0;
            }
            else
            {
                right_input.y = yaw_temp;
                right_input.x = std::sqrt(1 - (right_input.y * right_input.y));
            }

            right_input_2.z = 0.0;
            math::vec3 adjuster_angles;
            adjuster_angles.y =  math::to_deg(std::atan2(right_input.y, right_input.x));
            adjuster_angles.normalize_euler_angle();

            if (direction.y >= 1 || direction.y <= -1)
            {
                right_input_2.y = 1 / direction.y;
                right_input_2.x = 0.0;
            }
            else
            {
                right_input_2.y = direction.y;
                right_input_2.x = std::sqrt(1 - (right_input_2.y * right_input_2.y));
            }

            double inverse_yaw = 0.0;
            double inverse_yaw_2 = 0.0;
            if (right_input.x != 0)
            {
                inverse_yaw = right_input.y / right_input.x;
            }

            if (right_input_2.x != 0)
            {
                inverse_yaw_2 = right_input_2.y / right_input_2.x;
            }

            double pitch_input = 0.0;

            if (view_angles.x != 0)
            {
                if (inverse_yaw != 0 && inverse_yaw_2 != 0)
                {
                    pitch_input = 1.0;
                    double abs_inverse_yaw = std::abs(inverse_yaw);
                    double abs_inverse_yaw_2 = std::abs(inverse_yaw_2);

                    if (abs_inverse_yaw < abs_inverse_yaw_2)
                        pitch_input = inverse_yaw / inverse_yaw_2;
                    else if (abs_inverse_yaw_2 < abs_inverse_yaw)
                        pitch_input = inverse_yaw_2 / inverse_yaw;
                }
            }
            else
            {
                pitch_input = 1.0;
            }

            double pitch_cosine = 0.0, pitch_sine = 0.0;
            math::vec3 input_angles = {};
            if (pitch_input > 1 && pitch_input < -1)
            {
                pitch_cosine = pitch_input;
                pitch_sine = std::sqrt(1 - (pitch_input * pitch_input));

                math::vec3 unrotated_pitch = {pitch_sine, pitch_cosine, 0.0};
                double temp_pitch = view_angles.x;

                if (temp_pitch < 0)
                    temp_pitch *= -1;

                temp_pitch = (45 - temp_pitch) * 2;

                math::matrix3x4 matrix = {};
                matrix.angle_matrix({0.0, temp_pitch, 0.0});
                math::vec3 rotated_pitch = matrix.rotate_vec3(rotated_pitch);

                if (view_angles.x < 0)
                    input_angles.x = math::to_deg(std::atan2(rotated_pitch.y, rotated_pitch.x));
                else
                    input_angles.x = math::to_deg(std::atan2(-rotated_pitch.y, rotated_pitch.x));
            }

            input_angles.normalize_euler_angle();
            input_angles.to_vectors(forward, right, up);

            direction = forward + (right * spread_vec.x) + (up * spread_vec.y);
            direction.normalize();

            adjuster_angles.x = view_angles.x + math::to_deg(std::atan2(direction.z, direction.x));
            adjuster_angles.normalize_euler_angle();

            math::vec3 new_angles = view_angles;
            new_angles.x = adjuster_angles.x;
            new_angles.y += adjuster_angles.y;
            new_angles.z = 0.0;

            new_angles.normalize_euler_angle();

            // Recover precision
            for (auto i = 0; i < 5; i++)
            {
                new_angles.to_vectors(forward, right, up);
                direction = forward + (right * spread_vec.x ) + (up * spread_vec.y);

                direction.normalize();

                math::vec3 test_angles = direction.to_angles();
                test_angles.normalize_euler_angle();

                new_angles.x += (view_angles.x - test_angles.x);
                new_angles.y += (view_angles.y - test_angles.y);
                
                new_angles.normalize_euler_angle();
            }

            cmd->viewangles = new_angles.normalize_angle();
        }
    }
}