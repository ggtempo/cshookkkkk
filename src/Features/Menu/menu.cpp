#include "menu.hpp"
#include <chrono>
#include "../Utils/utils.hpp"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_custom.hpp"

#include "../Aimbot/aimbot.hpp"
#include "../AntiAim/antiaim.hpp"
#include "../Removals/removals.hpp"
#include "../Triggerbot/triggerbot.hpp"
#include "../Visuals/visuals.hpp"
#include "../Config/config.hpp"
#include "../Miscelaneous/miscelaneous.hpp"
#include "../Playerlist/playerlist.hpp"

namespace features
{
    void menu::show_menu()
    {
        static auto& g = globals::instance();

        if (features::miscelaneous::instance().can_show())
        {
            if (this->menu_enabled && ImGui::BeginMainMenuBar())
            {
                SYSTEMTIME time = {};
                GetLocalTime(&time);

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0, 0.0));

                auto title_bar_size = ImGui::GetWindowSize();

                ImGui::SameLine();

                ImGui::SetCursorPos(ImVec2(0.0, 0.0));
                if (ImGui::Button("    Aimbot    "))
                    this->aimbot_menu_enabled = !this->aimbot_menu_enabled;

                ImGui::SameLine();
                ImGui::Separator();

                if (ImGui::Button("    Trigger   "))
                    this->trigger_menu_enabled = !this->trigger_menu_enabled;

                ImGui::SameLine();
                ImGui::Separator();

                if (ImGui::Button("      ESP     "))
                    this->esp_menu_enabled = !this->esp_menu_enabled;

                ImGui::SameLine();
                ImGui::Separator();

                if (ImGui::Button("   Anti-Aim   "))
                    this->anti_aim_menu_enabled = !this->anti_aim_menu_enabled;

                ImGui::SameLine();
                ImGui::Separator();

                if (ImGui::Button("     MISC     "))
                    this->misc_menu_enabled = !this->misc_menu_enabled;

                ImGui::SameLine();
                ImGui::Separator();

                float right_offset = 10 + 19 + 19;

                {
                    auto size = ImGui::CalcTextSize("00:00:00");
                    right_offset += size.x;
                    ImGui::SetCursorPos(ImVec2(title_bar_size.x - right_offset, 0));
                    ImGui::Text("%02d:%02d:%02d", time.wHour, time.wMinute, time.wSecond);

                    right_offset += 110;
                }

                {
                    auto size = ImGui::CalcTextSize("CSHook by DJ_Luigi");
                    
                    ImGui::SetCursorPos(ImVec2((title_bar_size.x / 2) - (size.x / 2), 0));
                    ImGui::Text("CSHook by DJ_Luigi");
                }

                // Settings button
                ImGui::SetCursorPos(ImVec2(title_bar_size.x - 19.0f, 0.0f));
                {
                    if (ImGui::Button("###Settings", ImVec2(19.0f, 19.0f)))
                    {
                        this->settings_menu_enabled = !this->settings_menu_enabled;
                    }

                    auto draw_list = ImGui::GetWindowDrawList();
                    if (draw_list)
                    {
                        draw_list->AddImage((void*)g.gear_icon_id, ImVec2(title_bar_size.x - 19.0f, 0.0f), ImVec2(title_bar_size.x, 19.0f));
                    }
                }
                
                // Playerlist button
                ImGui::SetCursorPos(ImVec2(title_bar_size.x - 38.0f, 0.0f));
                {
                    if (ImGui::Button("###Playerlist", ImVec2(19.0f, 19.0f)))
                    {
                        this->playerlist_menu_enabled = !this->playerlist_menu_enabled;
                    }

                    auto draw_list = ImGui::GetWindowDrawList();
                    if (draw_list)
                    {
                        draw_list->AddImage((void*)g.person_icon_id, ImVec2(title_bar_size.x - 38.0f, 0.0f), ImVec2(title_bar_size.x - 19.0f, 19.0f));
                    }
                }

                ImGui::PopStyleVar();

                ImGui::EndMainMenuBar();
            }

            if (this->aimbot_menu_enabled && this->menu_enabled)
            {
                features::aimbot::instance().show_menu();
            }

            if (this->trigger_menu_enabled && this->menu_enabled)
            {
                features::triggerbot::instance().show_menu();
            }

            if (this->esp_menu_enabled && this->menu_enabled)
            {
                features::visuals::instance().show_menu();
            }

            if (this->anti_aim_menu_enabled && this->menu_enabled)
            {
                features::anti_aim::instance().show_menu();
            }

            if (this->misc_menu_enabled && this->menu_enabled)
            {
                features::miscelaneous::instance().show_menu();
            }

            if (this->settings_menu_enabled && this->menu_enabled)
            {
                features::config::instance().show_menu();
            }

            if (this->playerlist_menu_enabled && this->menu_enabled)
            {
                features::playerlist::instance().show_menu();
            }
        }
    }

    void menu::toggle_menu()
    {
        this->menu_enabled = !this->menu_enabled;
    }
}