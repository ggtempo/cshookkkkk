#pragma once
#include "imgui.h"
#include "../Utils/globals.hpp"
#include <string>
#include "../Utils/custom.hpp"
#include <optional>

namespace ImGui
{
    inline bool Hotkey(const char* label, int& current_key)
    {
        static auto& g = globals::instance();

        bool found_key = false;

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
            ImGui::Text("Press a key to set this hotkey!");
            ImGui::Text("%s", custom_key_name);

            if (ImGui::Button("OK"))
            {
                ImGui::CloseCurrentPopup();
                g.catch_keys = false;
                g.captured_key = -1;
                found_key = true;
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

        return found_key;
    }

    inline std::optional<std::string> TextModalButton(const std::string& name, const std::string& helper_text)
    {
        if (ImGui::Button(name.c_str()))
            ImGui::OpenPopup("TextInputModal");

        if (ImGui::BeginPopupModal("TextInputModal", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
        {
            static char buffer[256] = "";
            ImGui::Text(helper_text.c_str());
            ImGui::InputText("### asdasfasfaf", buffer, IM_ARRAYSIZE(buffer));

            if (ImGui::Button("OK"))
            {
                ImGui::CloseCurrentPopup();

                std::string result(buffer);
                buffer[0] = '\0';
                return result;
            }

            ImGui::EndPopup();
        }

        return std::nullopt;
    }
}