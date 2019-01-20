#pragma once
#include "../../Utils/globals.hpp"
#include <unordered_map>

namespace features
{
    class triggerbot
    {
        private:
            triggerbot()
            {
                this->enabled = false;
                this->team = false;


            }
        
        public:
            static triggerbot& instance()
            {
                static triggerbot aa;
                return aa;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void show_menu();

        private:
            bool enabled;
            bool team;
            bool all_hitboxes;
            std::unordered_map<int, bool> target_hitboxes;
    };
}