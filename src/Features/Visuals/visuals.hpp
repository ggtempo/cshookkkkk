#pragma once
#include <cpptoml.h>
#include "../../Utils/globals.hpp"

namespace features
{
    class visuals
    {
        private:
            visuals()
            {
                this->chams = false;
                this->chams_team = false;

                this->team_color = {
                    0.0, 0.0, 1.0, 0.3
                };

                this->enemy_color = {
                    1.0, 0.0, 0.0, 0.3
                };

                this->chams_mode = chams_modes::CHAMS_NORMAL;

                this->box_esp = false;
                this->box_esp_team = false;
                this->name_esp = false;
            }
        
        public:
            void load_from_config(std::shared_ptr<cpptoml::table> config)
            {
                // Get correct section
                auto visuals_table = config->get_table("visuals");

                // If table doesn't exist, just create an empty one
                if (!visuals_table)
                {
                    visuals_table = cpptoml::make_table();
                }

                // Get all values or their respective defaults
                this->chams = visuals_table->get_as<bool>("chams").value_or(false);
                this->chams_team = visuals_table->get_as<bool>("chams").value_or(false);
                this->chams_mode = static_cast<chams_modes>(visuals_table->get_as<bool>("chams_mode").value_or(0));
                this->box_esp = visuals_table->get_as<bool>("box_esp").value_or(false);
                this->box_esp_team = visuals_table->get_as<bool>("box_esp_team").value_or(false);
                this->name_esp = visuals_table->get_as<bool>("name_esp").value_or(false);

                // Get colors
                this->enemy_color.r = visuals_table->get_as<double>("enemy_color_r").value_or(1.0);
                this->enemy_color.g = visuals_table->get_as<double>("enemy_color_g").value_or(0.0);
                this->enemy_color.b = visuals_table->get_as<double>("enemy_color_b").value_or(0.0);
                this->enemy_color.a = visuals_table->get_as<double>("enemy_color_a").value_or(0.3);

                this->team_color.r = visuals_table->get_as<double>("team_color_r").value_or(0.0);
                this->team_color.g = visuals_table->get_as<double>("team_color_g").value_or(0.0);
                this->team_color.b = visuals_table->get_as<double>("team_color_b").value_or(1.0);
                this->team_color.a = visuals_table->get_as<double>("team_color_a").value_or(0.3);
            }

            void save_to_config(std::ofstream& config_stream)
            {
                // Create section
                config_stream << "[visuals]" << std::endl;

                // Write all relevant values
                config_stream
                    << "chams = "                       << (this->chams ? "true" : "false")             << std::endl
                    << "chams_team = "                  << (this->chams_team ? "true" : "false")        << std::endl
                    << "chams_mode = "                  << static_cast<int>(this->chams_mode)           << std::endl
                    << "box_esp = "                     << (this->box_esp ? "true" : "false")           << std::endl
                    << "box_esp_team = "                << (this->box_esp_team ? "true" : "false")      << std::endl
                    << "name_esp = "                    << (this->name_esp ? "true" : "false")          << std::endl
                    << "enemy_color_r = "               << this->enemy_color.r                          << std::endl
                    << "enemy_color_g = "               << this->enemy_color.g                          << std::endl
                    << "enemy_color_b = "               << this->enemy_color.b                          << std::endl
                    << "enemy_color_a = "               << this->enemy_color.a                          << std::endl
                    << "team_color_r = "                << this->team_color.r                           << std::endl
                    << "team_color_g = "                << this->team_color.g                           << std::endl
                    << "team_color_b = "                << this->team_color.b                           << std::endl
                    << "team_color_a = "                << this->team_color.a                           << std::endl;

                // Trailing newline
                config_stream << std::endl;
            }

        public:
            static visuals& instance()
            {
                static visuals aa;
                return aa;
            }

            void studio_render_model(CStudioModelRenderer* ecx);
            void swap_buffers();
            void show_menu();

        private:
            enum class chams_modes
            {
                CHAMS_NORMAL,
                CHAMS_NOTEXTURE
            };

        private:
            bool chams;
            bool chams_through_walls;
            bool chams_team;
            chams_modes chams_mode;

            bool box_esp;
            bool box_esp_team;
            bool name_esp;

            custom::color4f team_color;
            custom::color4f enemy_color;
    };
}