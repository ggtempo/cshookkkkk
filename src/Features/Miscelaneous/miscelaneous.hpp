#pragma once
#include <cpptoml.h>
#include "../../Utils/globals.hpp"

namespace features
{
    class miscelaneous
    {
        private:
            miscelaneous()
            {
                this->bhop_enabled = false;
                this->mirror_cam_enabled = false;
                this->third_person_enabled = false;

                this->hide_on_screenshot = false;
                this->taking_screenshot = false;
                this->taking_snapshot = false;
            }
        
        public:
            void load_from_config(std::shared_ptr<cpptoml::table> config)
            {
                // Get correct section
                auto misc_table = config->get_table("misc");

                // If table doesn't exist, just create an empty one
                if (!misc_table)
                {
                    misc_table = cpptoml::make_table();
                }

                // Get all values or their respective defaults
                this->bhop_enabled = misc_table->get_as<bool>("bhop_enabled").value_or(false);
                this->mirror_cam_enabled = misc_table->get_as<bool>("mirror_cam_enabled").value_or(false);
                this->third_person_enabled = misc_table->get_as<bool>("third_person_enabled").value_or(false);
                this->hide_on_screenshot = misc_table->get_as<bool>("hide_on_screenshot").value_or(false);
            }

            void save_to_config(std::ofstream& config_stream)
            {
                // Create section
                config_stream << "[misc]" << std::endl;

                // Write all relevant values
                config_stream
                    << "bhop_enabled = "                << (this->bhop_enabled ? "true" : "false")              << std::endl
                    << "mirror_cam_enabled = "          << (this->mirror_cam_enabled ? "true" : "false")        << std::endl
                    << "third_person_enabled = "        << (this->third_person_enabled ? "true" : "false")      << std::endl
                    << "hide_on_screenshot = "          << (this->hide_on_screenshot ? "true" : "false")        << std::endl;
                    

                // Trailing newline
                config_stream << std::endl;
            }

        public:
            static miscelaneous& instance()
            {
                static miscelaneous instance;
                return instance;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void calc_ref_def(ref_params_t* params);
            bool is_thirdperson();
            void show_menu();
            bool can_show();

            void swap_buffers();
            void on_screenshot();
            void on_snapshot();

        private:
            bool bhop_enabled;
            bool mirror_cam_enabled;
            bool third_person_enabled;

            bool hide_on_screenshot;
            bool taking_screenshot;
            bool taking_snapshot;
    };
}