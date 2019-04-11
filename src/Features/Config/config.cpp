#include "config.hpp"
#include <cpptoml.h>
#include <fstream>
#include "../Aimbot/aimbot.hpp"
#include "../AntiAim/antiaim.hpp"
#include "../Removals/removals.hpp"
#include "../Triggerbot/triggerbot.hpp"
#include "../Visuals/visuals.hpp"
#include "../Miscelaneous/miscelaneous.hpp"
#include "../../ImGui/imgui.h"
#include "../../Utils/utils.hpp"
#include "../../ImGui/imgui_custom.hpp"

namespace features
{
    void config::load_config()
    {
        static auto& g = globals::instance();

        // Get Full file path
        auto config_path = g.base_path + "configs\\" + this->current_config_name + ".toml";

        // Check if the file exists
        /*auto path = std::filesystem::path(config_path); 
        if (!std::filesystem::exists(path))
        {
            // Create empty config file if it doesn't
            std::ifstream file(config_path);
        }*/

        // Parse the config file
        auto config = cpptoml::parse_file(config_path);

        features::aimbot::instance().load_from_config(config);
        features::triggerbot::instance().load_from_config(config);
        features::visuals::instance().load_from_config(config);
        features::anti_aim::instance().load_from_config(config);
        features::removals::instance().load_from_config(config);
        features::miscelaneous::instance().load_from_config(config);
    }

    void config::save_config()
    {
        static auto& g = globals::instance();

        // Get Full file path
        auto config_path = g.base_path + "configs\\" + this->current_config_name + ".toml";
        
        if (auto file = std::ofstream(config_path); file.is_open())
        {
            features::aimbot::instance().save_to_config(file);
            features::triggerbot::instance().save_to_config(file);
            features::visuals::instance().save_to_config(file);
            features::anti_aim::instance().save_to_config(file);
            features::removals::instance().save_to_config(file);
            features::miscelaneous::instance().save_to_config(file);
        }
    }

    void config::show_menu()
    {
        static auto& g = globals::instance();

        if (ImGui::Begin("Settings"))
        {
            // Get all config files
            auto files = utils::get_files_in_directory(g.base_path + "configs\\");

            // Begin showing all files
            ImGui::ListBoxHeader("Configs");

            for (auto file : files)
            {
                // Get the file without extension
                auto extensionless = utils::remove_extension(file);
                // Check if this file is selected
                auto selected = (extensionless == this->current_config_name);

                // If the item is selected, switch the current config
                if (ImGui::Selectable(extensionless.c_str(), selected))
                {
                    this->current_config_name = extensionless;
                }

                // If the item is selected, bring it to focus
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::ListBoxFooter();

            // Save and load config
            if (ImGui::Button("Save"))
            {
                this->save_config();
            }
            ImGui::SameLine();

            if (ImGui::Button("Load"))
            {
                this->load_config();
            }
            ImGui::SameLine();

            // If the user wanted to create a new config
            auto result = ImGui::TextModalButton("New", "Enter the config name!");
            // And finished typing in the name
            if (result)
            {
                // Swtich to the new config, and also create it by saving
                this->current_config_name = utils::remove_extension(*result);
                this->save_config();
            }
            ImGui::SameLine();

            // If the user wants to delete a config (only allow it when the config isn't the default one)
            if (ImGui::Button("Delete") && !(this->current_config_name == "default"))
            {
                // Delete the config and set it to default one
                DeleteFileA((g.base_path + "configs/" + this->current_config_name + ".toml").c_str());
                this->current_config_name = "default";
            }

        }
        ImGui::End();
    }
}