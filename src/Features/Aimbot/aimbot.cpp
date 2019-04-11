#include "aimbot.hpp"
#include <chrono>
#include "../Utils/utils.hpp"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_custom.hpp"

namespace features
{
    void aimbot::create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        g.aim_fov = this->fov_max;

        if (this->enabled && (!this->on_key || GetAsyncKeyState(this->key)) && custom::is_gun(g.local_player_data.weapon.id))
        {
            vec3_t start = g.player_move->origin + g.player_move->view_ofs;
            vec3_t angles = cmd->viewangles;
            vec3_t forward = angles.to_vector() * 8192;

            if (this->prediction)
            {
                // Move players head in the direction the player is moving
                start += g.local_player_data.velocity * frametime;
            }

            // Get the best target
            auto best_target = this->find_best_target(start, angles, frametime);

            // Check if it's a valid target
            if (best_target.target_id != -1 && best_target.target_hitbox_id != -1)
            {
                // Transform hitbox position into world space
                auto& hitbox = g.player_data[best_target.target_id].hitboxes[best_target.target_hitbox_id];
                vec3_t center = best_target.center;


                auto needed_angles =    (center - start)        // Get target vector
                                        .normalize()            // Normalize to unit vector
                                        .to_angles()            // Get the necessary angle
                                        .normalize_angle();     // Normalize the angle

                //auto estimated_damage = get_estimated_damage(start, center, g.local_player_data.weapon.id, best_target.target_id, best_target.target_hitbox_id);
                //g.engine_funcs->Con_Printf("Estimated damage on target %i is %i\n", best_target.target_id, estimated_damage);

                if (this->smooth_enabled && !this->silent)
                {
                    // If the aimbot needs to follow a smooth path
                    // Get the angle difference
                    auto delta = (needed_angles - angles).normalize_angle();

                    // Smooth factor
                    float smooth = std::min(0.99f, 1 - (this->smooth_speed / 1000));
                    float coeff = (1.0f - smooth) / delta.length() * 4.0f;
                    coeff = std::min(1.0f, coeff);

                    // Needed angles = original angles + smoothing factor
                    // Thus we don't aim directly at a player, we move the crosshair slowly
                    needed_angles = angles + (delta * coeff);
                }

                // If we can fire or we smoothe the angles (eg: snap to target just when shooting or slowly move to the target)
                if ((((g.local_player_data.weapon.next_attack <= 0.0) && (g.local_player_data.weapon.next_primary_attack <= 0.0)) && (cmd->buttons & IN_ATTACK || this->auto_fire)) ||
                    this->smooth_enabled)
                {
                    // Normalize the angle
                    auto normalized = needed_angles.normalize_angle();
                    // Set the cmd viewangles (angles that are sent to the server)
                    cmd->viewangles = normalized;

                    // If we don't use silent aim, set the rendered angle as well (they will be a frame late tho)
                    if (!this->silent)
                        g.engine_funcs->SetViewAngles(normalized);

                    // If we should auto fire, check if we are aiming at a hitbox
                    if (this->auto_fire)
                    {
                        auto new_forward = cmd->viewangles.to_vector() * 8912;

                        if ((hitbox.visible || this->auto_wall) && (this->target_hitboxes[best_target.target_hitbox_id] || this->all_hitboxes) && key != hitbox_numbers::unknown)
                        {
                            // If the hitbox is visible and should trigger shooting, check if our crosshair intersects it
                            if (auto result = math::ray_hits_rbbox(start, new_forward, hitbox.box, hitbox.matrix); result.hit)
                            {
                                // Only fire if we have enough ammo
                                if (!g.local_player_data.weapon.in_reload)
                                {
                                    cmd->buttons |= IN_ATTACK;
                                }
                            }
                        }
                    }
                }
            }
        }

        // If psilent is enabled and we can fire, stop sending packets
        // Otherwise, continue normally
        if ((this->enabled && this->psilent && !this->smooth_enabled) &&
            ((g.local_player_data.weapon.next_attack <= 0.0) && (g.local_player_data.weapon.next_primary_attack <= 0.0)) && (cmd->buttons & IN_ATTACK))
        {
            g.send_packet = false;
        }
        else if (this->enabled && this->psilent)
        {
            g.send_packet = true;
        }
    }

    void aimbot::show_menu()
    {
        static auto& g = globals::instance();

        if (ImGui::Begin("Aimbot"))
        {
            ImGui::Checkbox("Aimbot enabled", &this->enabled);
            ImGui::Checkbox("Aim team", &this->team);
            ImGui::Checkbox("Prediction", &this->prediction);
            auto previous_silent = silent;
            auto previous_psilent = psilent;
            ImGui::Checkbox("Silent", &this->silent);
            ImGui::Checkbox("Perfect Silent", &this->psilent);

            // If user turned on psilent, turn on silent
            if (!this->silent && !previous_silent && this->psilent && !previous_psilent)
                this->silent = true;

            // If user turned off silent, turn off psilent
            if (!this->silent && previous_silent)
            {
                this->psilent = false;
                g.send_packet = true;
            }

            ImGui::Checkbox("###Auto wall enabled", &this->auto_wall);
            ImGui::SameLine();
            ImGui::DragInt("Auto wall", &this->auto_wall_min_damage, 1, 1, 100);
            ImGui::Checkbox("Auto fire", &this->auto_fire);

            ImGui::Checkbox("###FOV Enabled", &this->fov_enabled);
            ImGui::SameLine();
            ImGui::DragFloat("FOV", &this->fov_max, 1.0, 0.0, 360.0, "%f deg");

            ImGui::Checkbox("###Smooth Enabled", &this->smooth_enabled);
            ImGui::SameLine();
            ImGui::DragFloat("Smooth", &this->smooth_speed, 1.0, 0.0, 360.0);

            ImGui::Checkbox("###Aim on key", &this->on_key);
            ImGui::SameLine();
            ImGui::Hotkey("Aim key", this->key);
            

            auto complex_hitboxes = !this->all_hitboxes;
            ImGui::Checkbox("Show complex hitboxes", &complex_hitboxes);
            this->all_hitboxes = !complex_hitboxes;

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
        }
        ImGui::End();
    }

    aimbot::aim_target aimbot::find_best_target(const math::vec3& origin, const math::vec3& angles, float frame_time)
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        struct possible_target
        {
            int player_id;
            hitbox_numbers hitbox_id;
            
            float fov;
            float distance;

            math::vec3 center;
        };

        aimbot::aim_target best_target = {};
        std::vector<possible_target> possible_targets;

        // For every player
        for (auto i = 1; i <= g.engine_funcs->GetMaxClients(); i++)
        {
            auto entity = g.engine_funcs->GetEntityByIndex(i);

            // Check if its valid
            if (!utils::is_valid_player(entity))
                continue;

            if ((g.player_data[entity->index].team == g.local_player_data.team) && !this->team)
                continue;

            // Get player's velocity
            auto velocity = entity->curstate.origin - entity->prevstate.origin;

            // Go through each hitbox
            for (auto& [key, hitbox] : g.player_data[entity->index].hitboxes)
            {

                if ((this->target_hitboxes[key] || this->all_hitboxes) && key != hitbox_numbers::unknown)
                {
                    // Valid hitbox, get it's fov
                    auto matrix = hitbox.matrix;

                    // Test against rotated players
                    if (entity->curstate.angles.z != 0.0)
                    {
                        matrix = matrix.angle_matrix(math::vec3{0.0, 0.0, -entity->curstate.angles.z});
                    }

                    vec3_t transformed_bbmin = matrix.transform_vec3(hitbox.box.bbmin);
                    vec3_t transformed_bbmax = matrix.transform_vec3(hitbox.box.bbmax);
                    vec3_t center = (transformed_bbmin + transformed_bbmax) * 0.5;

                    // If the player cares about correcting for 1 tick old hitbox positions, move them by the velocity
                    if (this->prediction)
                    {
                        // Could possibly be incorrect
                        // The hitbox could be pushed into the wall
                        center += velocity * frame_time;
                    }

                    float distance = (center - origin).length();
                    vec3_t needed_angle = (center - origin).normalize().to_angles().normalize_angle();                    

                    auto fov_result = this->get_fov_to_target(angles, needed_angle, distance);

                    auto direction = angles.to_vector().normalize();
                    auto sphere_test = math::ray_hits_sphere(origin, direction, center, this->fov_max);

                    // If the players crosshair hits the virtual sphere, target is valid
                    if (sphere_test.hit || !this->fov_enabled)
                    {
                        // We found a useful target, push it to our possible target vector
                        possible_targets.push_back(possible_target{
                            entity->index, static_cast<hitbox_numbers>(key),
                            fov_result.fov, fov_result.real_distance,
                            center
                        });
                    }
                }
            }
        }

        // Sort, so that we get best target first
        std::sort(possible_targets.begin(), possible_targets.end(), [](const possible_target& target1, const possible_target& target2) -> bool {
            // Metric is the distance from crosshair + tenth of distance from head
            // In essence, we will get targets that are closer to our crosshair, or the ones closer to us
            // NOTE: This should probably be precalculated
            auto metric1 = (target1.fov) + (target1.distance / 10);
            auto metric2 = (target2.fov) + (target2.distance / 10);

            return metric1 < metric2;
        });

        // Go through each potential target
        for (auto target : possible_targets)
        {
            // Get the target hitbox
            auto& hitbox = g.player_data[target.player_id].hitboxes[target.hitbox_id];
            int hid = static_cast<int>(target.hitbox_id);

            if  (hitbox.visible ||          // Hitbox is visible
                (this->auto_wall &&         // Or we have auto-wall enabled and estimated damage is greater or equal to the required damage
                (utils::get_estimated_damage(origin, target.center, g.local_player_data.weapon.id, target.player_id, hid) >= this->auto_wall_min_damage)))
            {
                return {
                    target.player_id, target.hitbox_id,
                    target.center
                };
            }
        }

        return {-1, -1, math::vec3{0.0f, 0.0f, 0.0f}};
    }

    aimbot::fov_result aimbot::get_fov_to_target(const math::vec3& angles, const math::vec3& target_angles, float distance)
    {
        auto fov = math::get_fov(angles, target_angles);

        auto aiming_at = angles.to_vector();
        aiming_at *= distance;

        auto aim_at = target_angles.to_vector();
        aim_at *= distance;

        auto angle_distance = (aim_at - aiming_at).length();
        return {fov, angle_distance};
    }
}