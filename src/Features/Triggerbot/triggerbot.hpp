#pragma once
#include <cpptoml.h>
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
            void load_from_config(std::shared_ptr<cpptoml::table> config)
            {
                // Get correct section
                auto triggerbot_table = config->get_table("triggerbot");

                // If table doesn't exist, just create an empty one
                if (!triggerbot_table)
                {
                    triggerbot_table = cpptoml::make_table();
                }

                // Get all values or their respective defaults
                this->enabled = triggerbot_table->get_as<bool>("enabled").value_or(false);
                this->team = triggerbot_table->get_as<bool>("team").value_or(false);
                this->delay = triggerbot_table->get_as<int>("delay").value_or(0);
                this->on_key = triggerbot_table->get_as<bool>("on_key").value_or(false);
                this->key = triggerbot_table->get_as<int>("key").value_or(-1);

                // Get all target hitboxes
                auto hitboxes = triggerbot_table->get_array_of<int64_t>("target_hitboxes").value_or(std::vector<int64_t>{});

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
                config_stream << "[triggerbot]" << std::endl;

                // Write all relevant values
                config_stream
                    << "enabled = "                     << (this->enabled ? "true" : "false")               << std::endl
                    << "team = "                        << (this->team ? "true" : "false")                  << std::endl
                    << "delay = "                       << this->delay                                      << std::endl
                    << "on_key = "                      << (this->enabled ? "true" : "false")               << std::endl
                    << "key = "                         << this->key                                        << std::endl;

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