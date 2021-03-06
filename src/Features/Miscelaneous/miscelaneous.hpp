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

                this->steam_id_changer_enabled = false;
                this->steam_id = 1337;
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
                this->steam_id_changer_enabled = misc_table->get_as<bool>("steam_id_changer_enabled").value_or(false);
                this->steam_id = misc_table->get_as<int>("steam_id").value_or(1337);
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
                    << "hide_on_screenshot = "          << (this->hide_on_screenshot ? "true" : "false")        << std::endl
                    << "steam_id_changer_enabled = "    << (this->steam_id_changer_enabled ? "true" : "false")  << std::endl
                    << "steam_id = "                    << this->steam_id                                       << std::endl;
                    

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
            int  initiate_game_connection(void *ecx, int *data, int max_data, long long steam_id, int server_ip, short server_port, int secure);
            bool is_thirdperson();
            void show_menu();
            bool can_show();

            void swap_buffers();
            void swap_buffers_end();
            void on_screenshot();
            void on_snapshot();

        private:
            bool bhop_enabled;
            bool mirror_cam_enabled;
            bool third_person_enabled;

            bool hide_on_screenshot;
            bool taking_screenshot;
            bool taking_snapshot;

            bool steam_id_changer_enabled;
            int  steam_id;
    };
}