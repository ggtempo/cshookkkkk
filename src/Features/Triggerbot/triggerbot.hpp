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

                this->delay = 0;
                this->next_fire = -1;

                this->on_key = false;
                this->key = -1;

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
            int64_t delay;
            int64_t next_fire;
            bool on_key;
            int key;
            std::unordered_map<int, bool> target_hitboxes;
    };
}