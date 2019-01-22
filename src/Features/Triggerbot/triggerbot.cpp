#include "triggerbot.hpp"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_custom.hpp"
#include <chrono>

namespace features
{
    void triggerbot::create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        if (this->enabled && (!this->on_key || GetAsyncKeyState(this->key)))
        {
            vec3_t start = g.player_move->origin + g.player_move->view_ofs;
            vec3_t angles = cmd->viewangles;
            vec3_t forward = {};
            vec3_t end = {};


            g.engine_funcs->pfnAngleVectors(angles, forward, nullptr, nullptr);
            end = start.multiply_add(2148, forward);


            for (auto i = 0; i < g.engine_funcs->GetMaxClients(); i++)
            {
                auto entity = g.engine_funcs->GetEntityByIndex(i);

                if (g.player_data[entity->index].dormant || !g.player_data[entity->index].alive)
                    continue;

                if ((g.player_data[entity->index].team == g.local_player_data.team) && !this->team)
                    continue;

                for (auto& [key, hitbox] : g.player_data[entity->index].hitboxes)
                {
                    if (hitbox.visible && (this->target_hitboxes[key] || this->all_hitboxes) && key != hitbox_numbers::unknown)
                    {
                        if (auto result = math::ray_hits_rbbox(start, forward, hitbox.box, hitbox.matrix); result.hit)
                        {
                            auto now = std::chrono::high_resolution_clock().now().time_since_epoch();
                            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

                            if (((this->next_fire != -1) && (this->next_fire <= ms)) || (this->delay == 0) &&
                                g.local_player_data.weapon.next_attack <= 0.0 && g.local_player_data.weapon.next_primary_attack <= 0.0)
                            {
                                cmd->buttons |= IN_ATTACK;
                                this->next_fire = -1;
                                g.engine_funcs->Con_Printf("[Triggerbot] fire!\n");
                            }
                            else if (this->next_fire == -1)
                            {
                                this->next_fire = ms + this->delay;
                                g.engine_funcs->Con_Printf("[Triggerbot] next_fire %i, current msec %i\n", this->next_fire, ms);
                            }
                        }
                    }
                }
            }

            //auto trace = *g.engine_funcs->PM_TraceLine(start, end, PM_TRACELINE_PHYSENTSONLY, 0, lp->index);
            /*pmtrace_t trace = {};

            g.engine_funcs->pEventAPI->EV_SetTraceHull(2);
            g.engine_funcs->pEventAPI->EV_PlayerTrace(start, end, PM_GLASS_IGNORE, -1, &trace);
            
            if (trace.fraction != 1.0f)
            {
                // We have a hit
                
                if (trace.ent > 0 && trace.ent <= g.player_move->numphysent)
                {
                    // Get the actual player
                    // trace->ent is the physical entity number
                    // We need to get the normal one
                    // From playermove?
                    auto target = g.engine_funcs->GetEntityByIndex(g.player_move->physents[trace.ent].info);

                    bool hit = false;
                    for (auto& [key, value] : this->target_hitboxes)
                    {
                        if (value || this->all_hitboxes)
                        {
                            auto& box = g.player_data[target->index].hitboxes[key];
                            auto trace2 = math::ray_hits_rbbox(start, forward, box.box, box.matrix);

                            if (trace2.hit)
                            {
                                hit = true;
                                break;
                            }
                        }
                    }

                    if (hit)
                    {
                        // We hit with a second, more precise trace
                        if (target->player && ((g.player_data[target->index].team != g.local_player_data.team) || this->team))
                        {
                            auto now = std::chrono::high_resolution_clock().now().time_since_epoch();
                            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();


                            if (((this->next_fire != -1) && (this->next_fire <= ms)) || (this->delay == 0))
                            {
                                cmd->buttons |= IN_ATTACK;
                                this->next_fire = -1;
                                g.engine_funcs->Con_Printf("[Triggerbot] fire!\n");
                            }
                            else if (this->next_fire == -1)
                            {
                                this->next_fire = ms + this->delay;
                                g.engine_funcs->Con_Printf("[Triggerbot] next_fire %i, current msec %i\n", this->next_fire, ms);
                            }
                        }
                    }
                }
            }*/
        }
    }

    void triggerbot::show_menu()
    {
        static auto complex_hitboxes = false;

        if (ImGui::Begin("Triggerbot"))
        {
            ImGui::Checkbox("Triggerbot enabled", &this->enabled);
            ImGui::Checkbox("Shoot team", &this->team);
            ImGui::DragInt("Delay", (int32_t*)&this->delay, 1.0, 0, 1000, "%f ms");
            ImGui::Checkbox("###Trigger on key", &this->on_key);
            ImGui::SameLine();
            ImGui::Hotkey("Trigger key", this->key);
            
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
    }
}