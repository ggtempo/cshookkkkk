#pragma once
#include <cpptoml.h>
#include "../../Utils/globals.hpp"
#include <unordered_map>
#include <vector>

namespace features
{
    class menu
    {
        private:
            menu()
            {
                this->menu_enabled = true;
                this->aimbot_menu_enabled = false;
                this->trigger_menu_enabled = false;
                this->esp_menu_enabled = false;
                this->anti_aim_menu_enabled = false;
                this->misc_menu_enabled = false;
                this->settings_menu_enabled = false;
                this->playerlist_menu_enabled = false;
            }
        
        public:
            static menu& instance()
            {
                static menu instance;
                return instance;
            }

            void show_menu();
            void toggle_menu();

        private:
            bool menu_enabled;
            bool aimbot_menu_enabled;
            bool trigger_menu_enabled;
            bool esp_menu_enabled;
            bool anti_aim_menu_enabled;
            bool misc_menu_enabled;
            bool settings_menu_enabled;
            bool playerlist_menu_enabled;
    };
}