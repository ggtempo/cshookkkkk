#include "playerlist.hpp"
#include <chrono>
#include "../Utils/utils.hpp"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_custom.hpp"

namespace features
{
    void playerlist::show_menu()
    {
        static auto& g = globals::instance();

        if (ImGui::Begin("Playerlist"))
        {
            static int selected = -1;

            ImGui::ListBoxHeader("###Players");
            {
                for (auto& [id, player] : g.player_data)
                {
                    if (!player.name)
                        continue;

                    if (ImGui::Selectable(player.name, selected == id))
                    {
                        selected = id;
                    }
                }
            }
            ImGui::ListBoxFooter();

            if (selected != -1)
            {
                auto& player = g.player_data[selected];                
                auto entity = g.engine_funcs->GetEntityByIndex(selected);
                auto local = g.engine_funcs->GetLocalPlayer();

                if (entity && local)
                {
                    ImGui::Text("Exists: %s", entity ? "true" : "false");
                    ImGui::Text("Alive: %s", player.alive ? "true" : "false");
                    ImGui::Text("Dormant: %s", player.dormant ? "true" : "false");
                    ImGui::Text("Same as local: %s", (entity == local || entity->index == local->index) ? "true" : "false");
                    ImGui::Text("ID: %i (> 0, < max clients), valid: %s", entity->index, (!(entity->index < 0 || entity->index > g.engine_funcs->GetMaxClients())) ? "true" : "false");
                    ImGui::Text("Has model: %s", entity->model ? "true" : "false");
                    ImGui::Text("Is player: %s", entity->player ? "true" : "false");
                    ImGui::Text("Has gun: %s", entity->curstate.weaponmodel ? "true" : "false");
                    ImGui::Text("Valid team: %s", (player.team != custom::player_team::UNKNOWN) ? "true" : "false");
                    ImGui::Text("Mins: %f %f %f, valid: %s", entity->curstate.mins.x, entity->curstate.mins.y, entity->curstate.mins.z, (!entity->curstate.mins.is_zero()) ? "true" : "false");
                    ImGui::Text("Maxs: %f %f %f, valid: %s", entity->curstate.maxs.x, entity->curstate.maxs.y, entity->curstate.maxs.z, (!entity->curstate.maxs.is_zero()) ? "true" : "false");
                    ImGui::Text("Valid messagenum: %s (%i < %i)", entity->curstate.messagenum >= local->curstate.messagenum ? "true" : "false", entity->curstate.messagenum, local->curstate.messagenum);
                    ImGui::Text("Solid: %s", entity->curstate.solid == 3 ? "true" : "false");
                    ImGui::Text("User test valid: %s", (!(local->curstate.iuser1 == 4 && local->curstate.iuser2 == entity->index)) ? "true" : "false");
                }
            }
        }
        ImGui::End();
    }
}