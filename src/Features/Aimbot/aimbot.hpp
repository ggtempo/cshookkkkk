#pragma once
#include "../../Utils/globals.hpp"
#include <unordered_map>

namespace features
{
    class aimbot
    {
        private:
            struct aim_target
            {
                int target_id;
                int target_hitbox_id;
            };

        private:
            aimbot()
            {
                this->enabled = false;
                this->team = false;

                this->delay = 0;
                this->next_fire = -1;

                this->on_key = false;
                this->key = -1;
            }
        
        public:
            static aimbot& instance()
            {
                static aimbot aa;
                return aa;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void show_menu();

        private:
            aim_target find_best_target(const math::vec3& origin, const math::vec3& angles);
            float get_fov_to_target(const math::vec3& angles, const math::vec3& target_angles, float distance);

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