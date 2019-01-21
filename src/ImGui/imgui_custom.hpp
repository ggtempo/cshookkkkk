#pragma once
#include "imgui.h"
#include "../Utils/globals.hpp"
#include <string>
#include "../Utils/custom.hpp"

namespace ImGui
{
    inline bool Hotkey(const char* label, int& current_key)
    {
        static auto& g = globals::instance();

        g.catch_keys = true;
        auto custom_key_name = custom::get_key_name(current_key);
        if (custom_key_name == NULL)
        {
            custom_key_name = "Unassigned";
        }
            

        if (ImGui::Button(custom_key_name))
        {
            ImGui::OpenPopup("HotkeyModal");
        }

        if (ImGui::BeginPopupModal("HotkeyModal", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::Text("Press a key to set this hotkey!\n");
            ImGui::Text("%s", custom_key_name);

            if (ImGui::Button("OK"))
            {
                ImGui::CloseCurrentPopup();
                g.catch_keys = false;
                g.captured_key = -1;
            }
            else
            {
                current_key = g.captured_key;
            }

            ImGui::EndPopup();
        }

        ImGui::SameLine();

        std::string label_id = "###" + std::string(label);
        ImGui::LabelText(label_id.c_str(), label);
    }
}