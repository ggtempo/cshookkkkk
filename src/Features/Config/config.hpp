#pragma once
#include <cpptoml.h>
#include <fstream>
#include "../Aimbot/aimbot.hpp"
#include "../AntiAim/antiaim.hpp"
#include "../Removals/removals.hpp"
#include "../Triggerbot/triggerbot.hpp"
#include "../Visuals/visuals.hpp"

namespace config
{
    inline void load_config(const std::string& config_name)
    {
        static auto& g = globals::instance();

        // Get Full file path
        auto config_path = g.base_path + "configs\\" + config_name + ".toml";

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
    }

    inline void save_config(const std::string& config_name)
    {
        static auto& g = globals::instance();

        // Get Full file path
        auto config_path = g.base_path + "configs\\" + config_name + ".toml";
        
        if (auto file = std::ofstream(config_path); file.is_open())
        {
            features::aimbot::instance().save_to_config(file);
            features::triggerbot::instance().save_to_config(file);
            features::visuals::instance().save_to_config(file);
            features::anti_aim::instance().save_to_config(file);
            features::removals::instance().save_to_config(file);
        }
    }
}