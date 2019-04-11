#pragma once
#include <cpptoml.h>
#include "../../Utils/globals.hpp"
#include <unordered_map>
#include <vector>

namespace features
{
    class aimbot
    {
        private:
            struct aim_target
            {
                int target_id;
                int target_hitbox_id;
                math::vec3 center;

                bool operator==(const aim_target& other) const
                {
                    return  (this->target_id == other.target_id)                &&
                            (this->target_hitbox_id == other.target_hitbox_id)  &&
                            (this->center == other.center);
                }

                bool operator!=(const aim_target& other) const
                {
                    return !(*this == other);
                }
            };

            struct fov_result 
            {
                float fov;
                float real_distance;
            };

        private:
            aimbot()
            {
                this->enabled = false;
                this->team = false;
                this->silent = false;
                this->psilent = false;
                this->auto_wall = false;
                this->auto_wall_min_damage = 30;
                this->auto_fire = false;
                this->fov_enabled = false;
                this->fov_max = 360.0f;
                this->smooth_enabled = false;
                this->smooth_speed = 7.0f;

                this->prediction = false;

                this->delay = 0;
                this->next_fire = -1;

                this->on_key = false;
                this->key = -1;
            }

        public:
            void load_from_config(std::shared_ptr<cpptoml::table> config)
            {
                // Get correct section
                auto aimbot_table = config->get_table("aimbot");

                // If table doesn't exist, just create an empty one
                if (!aimbot_table)
                {
                    aimbot_table = cpptoml::make_table();
                }

                // Get all values or their respective defaults
                this->enabled = aimbot_table->get_as<bool>("enabled").value_or(false);
                this->team = aimbot_table->get_as<bool>("team").value_or(false);
                this->silent = aimbot_table->get_as<bool>("silent").value_or(false);
                this->psilent = aimbot_table->get_as<bool>("psilent").value_or(false);
                this->auto_wall = aimbot_table->get_as<bool>("auto_wall").value_or(false);
                this->auto_wall_min_damage = aimbot_table->get_as<int>("auto_wall_min_damage").value_or(30);
                this->auto_fire = aimbot_table->get_as<bool>("auto_fire").value_or(false);
                this->fov_enabled = aimbot_table->get_as<bool>("fov_enabled").value_or(false);
                this->fov_max = aimbot_table->get_as<double>("fov_max").value_or(360.0);
                this->smooth_enabled = aimbot_table->get_as<bool>("smooth_enabled").value_or(false);
                this->smooth_speed = aimbot_table->get_as<double>("smooth_speed").value_or(7.0);
                this->prediction = aimbot_table->get_as<bool>("prediction").value_or(false);
                this->on_key = aimbot_table->get_as<bool>("on_key").value_or(false);
                this->key = aimbot_table->get_as<int>("key").value_or(-1);

                // Get all target hitboxes
                auto hitboxes = aimbot_table->get_array_of<int64_t>("target_hitboxes").value_or(std::vector<int64_t>{});

                // Reset all of the hitboxes
                for (auto& hitbox : this->target_hitboxes)
                {
                    hitbox.second = false;
                }

                // Set each target hitbox
                for (auto hitbox : hitboxes)
                {
                    this->target_hitboxes[hitbox] = true;
                }

                // Aim at all hitboxes only if we have no target_hitboxes
                this->all_hitboxes = (hitboxes.size() == 0);
            }

            void save_to_config(std::ofstream& config_stream)
            {
                // Create section
                config_stream << "[aimbot]" << std::endl;

                // Write all relevant values
                config_stream 
                    << "enabled = "                << (this->enabled ? "true" : "false")        << std::endl
                    << "team = "                   << (this->team ? "true" : "false")           << std::endl
                    << "silent = "                 << (this->silent ? "true" : "false")         << std::endl
                    << "psilent = "                << (this->psilent ? "true" : "false")        << std::endl
                    << "auto_wall = "              << (this->auto_wall ? "true" : "false")      << std::endl
                    << "auto_wall_min_damage = "   << this->auto_wall_min_damage                << std::endl
                    << "auto_fire = "              << (this->auto_fire ? "true" : "false")      << std::endl
                    << "fov_enabled = "            << (this->fov_enabled ? "true" : "false")    << std::endl
                    << "fov_max = "                << this->fov_max                             << std::endl
                    << "smooth_enabled = "         << (this->smooth_enabled ? "true" : "false") << std::endl
                    << "smooth_speed = "           << this->smooth_speed                        << std::endl
                    << "prediction = "             << (this->prediction ? "true" : "false")     << std::endl
                    << "on_key = "                 << (this->on_key ? "true" : "false")         << std::endl
                    << "key = "                    << this->key                                 << std::endl;

                // Write all hitboxes
                config_stream << "target_hitboxes = [";
                for (auto& [hitbox, active] : this->target_hitboxes)
                {
                    if (active)
                    {
                        config_stream << " " << hitbox << ",";
                    }
                }
                config_stream << "]" << std::endl;
                // Trailing newline
                config_stream << std::endl;
            }
        
        public:
            static aimbot& instance()
            {
                static aimbot instance;
                return instance;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void show_menu();

        private:
            aim_target find_best_target(const math::vec3& origin, const math::vec3& angles, float frame_time);
            fov_result get_fov_to_target(const math::vec3& angles, const math::vec3& target_angles, float distance);

        private:
            bool enabled;
            bool team;
            bool silent;
            bool psilent;
            bool auto_wall;
            int auto_wall_min_damage;

            bool auto_fire;

            bool fov_enabled;
            float fov_max;
            bool smooth_enabled;
            float smooth_speed;

            bool prediction;

            int64_t delay;
            int64_t next_fire;

            bool on_key;
            int key;

            bool all_hitboxes;
            std::unordered_map<int, bool> target_hitboxes;
    };
}