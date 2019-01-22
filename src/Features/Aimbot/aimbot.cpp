#include "aimbot.hpp"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_custom.hpp"
#include <chrono>

namespace features
{
    void aimbot::create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();

        if (this->enabled && (!this->on_key || GetAsyncKeyState(this->key)))
        {
            vec3_t start = g.player_move->origin + g.player_move->view_ofs;
            vec3_t angles = cmd->viewangles;

            auto best_target = this->find_best_target(start, angles);

            if (best_target.target_id != -1)
            {
                auto& hitbox = g.player_data[best_target.target_id].hitboxes[best_target.target_hitbox_id];
                vec3_t transformed_bbmin = hitbox.matrix.transform_vec3(hitbox.box.bbmin);
                vec3_t transformed_bbmax = hitbox.matrix.transform_vec3(hitbox.box.bbmax);
                vec3_t center = (transformed_bbmin + transformed_bbmax) * 0.5;

                auto angle = (center - start).normalize().to_angles().normalize_angle();
                cmd->viewangles = angle;
                //g.engine_funcs->SetViewAngles(angle);
            }
        }
    }

    void aimbot::show_menu()
    {
        static auto complex_hitboxes = false;

        ImGui::Begin("Aimbot");
            ImGui::Checkbox("Aimbot enabled", &this->enabled);
            ImGui::Checkbox("Aim team", &this->team);
            ImGui::DragInt("Delay", (int32_t*)&this->delay, 1.0, 0, 1000, "%f ms");
            ImGui::Checkbox("###Aim on key", &this->on_key);
            ImGui::SameLine();
            ImGui::Hotkey("Aim key", this->key);
            
            ImGui::Checkbox("Show complex hitboxes", &complex_hitboxes);

            if (complex_hitboxes)
            {
                this->all_hitboxes = false;
                ImGui::ListBoxHeader("Hitboxes");
                {
                    static auto& pelvis =           this->target_hitboxes[hitbox_numbers::pelvis];
                    static auto& left_top_leg =     this->target_hitboxes[hitbox_numbers::left_top_leg];
                    static auto& left_bottom_leg =  this->target_hitboxes[hitbox_numbers::left_bottom_leg];
                    static auto& left_foot =        this->target_hitboxes[hitbox_numbers::left_foot];
                    static auto& right_top_leg =    this->target_hitboxes[hitbox_numbers::right_top_leg];
                    static auto& right_bottom_leg = this->target_hitboxes[hitbox_numbers::right_bottom_leg];
                    static auto& right_foot =       this->target_hitboxes[hitbox_numbers::right_foot];
                    static auto& torso_bottom =     this->target_hitboxes[hitbox_numbers::torso_bottom];
                    static auto& torso_top =        this->target_hitboxes[hitbox_numbers::torso_top];
                    static auto& neck =             this->target_hitboxes[hitbox_numbers::neck];
                    static auto& heart =            this->target_hitboxes[hitbox_numbers::heart];
                    static auto& head =             this->target_hitboxes[hitbox_numbers::head];
                    static auto& left_shoulder =    this->target_hitboxes[hitbox_numbers::left_shoulder];
                    static auto& left_arm_top =     this->target_hitboxes[hitbox_numbers::left_arm_top];
                    static auto& left_arm_bottom =  this->target_hitboxes[hitbox_numbers::left_arm_bottom];
                    static auto& left_hand =        this->target_hitboxes[hitbox_numbers::left_hand];
                    static auto& right_shoulder =   this->target_hitboxes[hitbox_numbers::right_shoulder];
                    static auto& right_arm_top =    this->target_hitboxes[hitbox_numbers::right_arm_top];
                    static auto& right_arm_bottom = this->target_hitboxes[hitbox_numbers::right_arm_bottom];
                    static auto& right_hand =       this->target_hitboxes[hitbox_numbers::right_hand];

                    ImGui::Selectable("Head", &head);
                    ImGui::Selectable("Neck", &neck);
                    ImGui::Selectable("Torso top", &torso_top);
                    ImGui::Selectable("Torso bottom", &torso_bottom);
                    ImGui::Selectable("Pelvis", &pelvis);
                    ImGui::Selectable("Heart", &heart);
                    ImGui::Selectable("Left top leg", &left_top_leg);
                    ImGui::Selectable("Left bottom leg", &left_bottom_leg);
                    ImGui::Selectable("Left foot", &left_foot);
                    ImGui::Selectable("Right top leg", &right_top_leg);
                    ImGui::Selectable("Right bottom leg", &right_bottom_leg);
                    ImGui::Selectable("Right foot", &right_foot );
                    ImGui::Selectable("Left shoulder", &left_shoulder);
                    ImGui::Selectable("Left arm top", &left_arm_top);
                    ImGui::Selectable("Left arm bottom", &left_arm_bottom);
                    ImGui::Selectable("Left hand", &left_hand);
                    ImGui::Selectable("Right shoulder", &right_shoulder);
                    ImGui::Selectable("Right arm top", &right_arm_top);
                    ImGui::Selectable("Right arm bottom", &right_arm_bottom);
                    ImGui::Selectable("Right hand", &right_hand);
                }
                ImGui::ListBoxFooter();
                ImGui::SameLine();
                if (ImGui::Button("Select / Deselect all"))
                {
                    bool new_state = !this->target_hitboxes[0];
                    for (auto& [key, value] : this->target_hitboxes)
                    {
                        value = new_state;
                    }
                }
            }
            else
            {
                this->all_hitboxes = true;
            }
        ImGui::End();
    }

    aimbot::aim_target aimbot::find_best_target(const math::vec3& origin, const math::vec3& angles)
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();
        aim_target best_target = {-1, -1};
        float best_fov = 9999.0f;

        // For every player
        for (auto i = 0; i < g.engine_funcs->GetMaxClients(); i++)
        {
            auto entity = g.engine_funcs->GetEntityByIndex(i);

            // Check if its valid
            if (!entity || entity == local || entity->index == local->index || !entity->player)
                continue;

            if ((g.player_data[entity->index].team == g.local_player_data.team) && !this->team)
                continue;

            // Go through each hitbox
            for (auto& [key, hitbox] : g.player_data[entity->index].hitboxes)
            {
                // Check if it fits our criteria
                if (hitbox.visible && (this->target_hitboxes[key] || this->all_hitboxes) && key != hitbox_numbers::unknown)
                {
                    // Valid hitbox, get it's fov
                    vec3_t transformed_bbmin = hitbox.matrix.transform_vec3(hitbox.box.bbmin);
                    vec3_t transformed_bbmax = hitbox.matrix.transform_vec3(hitbox.box.bbmax);
                    vec3_t center = (transformed_bbmin + transformed_bbmax) * 0.5;

                    float distance = (center - origin).length();
                    vec3_t needed_angle = (center - origin).normalize().to_angles().normalize_angle();

                    float fov = this->get_fov_to_target(angles, needed_angle, distance);
                    if (fov < best_fov)
                    {
                        best_target.target_id = i;
                        best_target.target_hitbox_id = key;
                        best_fov = fov;
                    }

                    //g.engine_funcs->Con_Printf("Testing from %f %f %f, angles: %f %f %f\n", origin.x, origin.y, origin.z, angles.x, angles.y, angles.z);
                    //g.engine_funcs->Con_Printf("Testing target %i (hitbox: %i), fov: %f, distance: %f ,angles: %f %f %f\n", entity->index, key, fov, distance, needed_angle.x, needed_angle.y, needed_angle.z);
                }
            }
        }

        //g.engine_funcs->Con_Printf("Found the best target %i (hitbox %i), fov: %f\n", best_target.target_id, best_target.target_hitbox_id, best_fov);
        return best_target;
    }

    float aimbot::get_fov_to_target(const math::vec3& angles, const math::vec3& target_angles, float distance)
    {
        /*float pitch_difference = std::abs(target_angles.x - angles.x);
        float yaw_difference = std::abs(target_angles.y - angles.y);

        float pitch_distance = std::cos(math::to_rad(pitch_difference)) * distance;
        float yaw_distance = std::sin(math::to_rad(yaw_difference)) * distance;*/

        auto normal_angles = math::vec3(angles.x, angles.y, angles.z).normalize_angle();
        auto normal_target_angles = math::vec3(target_angles.x, angles.y, angles.z).normalize_angle();
        auto difference = normal_target_angles - normal_angles;
        difference.x = std::abs(difference.x);
        difference.y = std::abs(difference.y);
        difference.normalize_angle();

        //return std::sqrt((pitch_distance * pitch_distance) + (yaw_distance * yaw_distance));
        return std::sqrt((difference.x * difference.x) + (difference.y * difference.y));
    }
}