#pragma once
#include <cpptoml.h>
#include "../../Utils/globals.hpp"
#include <unordered_map>
#include <vector>
#include <iterator>

namespace features
{
    class playerlist
    {
        private:
            playerlist()
            {
            }

        public:
            void load_from_config(std::shared_ptr<cpptoml::table> config)
            {
                // Get correct section
                auto playerlist_table = config->get_table("playerlist");

                // If table doesn't exist, just create an empty one
                if (!playerlist_table)
                {
                    playerlist_table = cpptoml::make_table();
                }

                // Get all values or their respective defaults
            }

            void save_to_config(std::ofstream& config_stream)
            {
                // Create section
                config_stream << "[playerlist]" << std::endl;

                // Write all relevant values


                // Trailing newline
                config_stream << std::endl;
            }
        
        public:
            static playerlist& instance()
            {
                static playerlist instance;
                return instance;
            }

            void show_menu();

        private:

    };
}