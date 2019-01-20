#pragma once
#include "../../Utils/globals.hpp"

namespace features
{
    class anti_aim
    {
        private:
            anti_aim()
            {
                this->enabled = false;
            }
        
        public:
            static anti_aim& instance()
            {
                static anti_aim aa;
                return aa;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void show_menu();

        private:
            bool enabled;
    };
}