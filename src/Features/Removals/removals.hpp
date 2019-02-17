#pragma once
#include <cpptoml.h>
#include "../../Utils/globals.hpp"

namespace features
{
    class removals
    {
        private:
            removals()
            {
                this->no_flash = false;
                this->no_flash_value = 1.0;
                this->fullbright = false;
                this->no_spread_enabled = false;
                this->no_recoil_enabled = false;
                this->no_visual_recoil = false;
                this->no_spread_method = no_spread_methods::nospread1;
            }
        
        public:
            void load_from_config(std::shared_ptr<cpptoml::table> config)
            {
                // Get correct section
                auto removals_table = config->get_table("removals");

                // If table doesn't exist, just create an empty one
                if (!removals_table)
                {
                    removals_table = cpptoml::make_table();
                }

                // Get all values or their respective defaults
                this->no_flash = removals_table->get_as<bool>("no_flash").value_or(false);;
                this->no_flash_value = removals_table->get_as<double>("no_flash_value").value_or(1.0);;
                this->fullbright = removals_table->get_as<bool>("fullbright").value_or(false);;
                this->no_spread_enabled = removals_table->get_as<bool>("no_spread_enabled").value_or(false);;
                this->no_recoil_enabled = removals_table->get_as<bool>("no_recoil_enabled").value_or(false);;
                this->no_visual_recoil = removals_table->get_as<bool>("no_visual_recoil").value_or(false);;
                this->no_spread_method = static_cast<no_spread_methods>(removals_table->get_as<int>("no_spread_method").value_or(0));
            }

            void save_to_config(std::ofstream& config_stream)
            {
                // Create section
                config_stream << "[removals]" << std::endl;

                // Write all relevant values
                config_stream
                    << "no_flash = "                    << (this->no_flash ? "true" : "false")              << std::endl
                    << "no_flash_value = "              << this->no_flash_value                             << std::endl
                    << "fullbright = "                  << (this->fullbright ? "true" : "false")            << std::endl
                    << "no_spread_enabled = "           << (this->no_spread_enabled ? "true" : "false")     << std::endl
                    << "no_recoil_enabled = "           << (this->no_recoil_enabled ? "true" : "false")     << std::endl
                    << "no_visual_recoil = "            << (this->no_visual_recoil ? "true" : "false")      << std::endl
                    << "no_spread_method = "            << static_cast<int>(this->no_spread_method)         << std::endl;

                // Trailing newline
                config_stream << std::endl;
            }

        public:
            static removals& instance()
            {
                static removals aa;
                return aa;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void calc_ref_def(ref_params_t* params);
            void hud_redraw(float time, int intermission);

            void show_menu();

        private:
            void no_recoil(float frametime, usercmd_t *cmd, int active);
            void no_spread(float frametime, usercmd_t *cmd, int active);
            void no_spread_2(float frametime, usercmd_t *cmd, int active);

            enum class no_spread_methods
            {
                nospread1,
                nospread2
            };

        private:
            bool no_flash;
            float no_flash_value;

            bool fullbright;

            bool no_spread_enabled;
            bool no_recoil_enabled;
            bool no_visual_recoil;
            no_spread_methods no_spread_method;
    };
}