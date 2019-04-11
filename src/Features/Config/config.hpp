#pragma once
#include <string>

namespace features
{
    class config
    {
        private:
            config()
            {
                this->current_config_name = "default";
            }

        public:
            static config& instance()
            {
                static config instance;
                return instance;
            }

        public:
            void load_config();
            void save_config();

            void show_menu();

        private:
            std::string current_config_name;

    };
}