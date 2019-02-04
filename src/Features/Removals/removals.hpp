#pragma once
#include "../../Utils/globals.hpp"

namespace features
{
    class removals
    {
        private:
            removals()
            {
                this->no_spread_enabled = false;
                this->no_recoil_enabled = false;
                this->no_visual_recoil = false;
                this->no_spread_method = no_spread_methods::nospread1;
            }
        
        public:
            static removals& instance()
            {
                static removals aa;
                return aa;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void calc_ref_def(ref_params_t* params);

            void show_menu();

        private:
            void no_recoil(float frametime, usercmd_t *cmd, int active);
            void no_spread(float frametime, usercmd_t *cmd, int active);
            void no_spread_2(float frametime, usercmd_t *cmd, int active);

            enum class no_spread_methods
            {
                nospread1,
                nospread2
            };

        private:
            bool no_spread_enabled;
            bool no_recoil_enabled;
            bool no_visual_recoil;
            no_spread_methods no_spread_method;
    };
}