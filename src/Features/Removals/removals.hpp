#pragma once
#include "../../Utils/globals.hpp"

namespace features
{
    class removals
    {
        private:
            removals()
            {
            }
        
        public:
            static removals& instance()
            {
                static removals aa;
                return aa;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void calc_ref_def(ref_params_t* params);

        private:
            void no_recoil(float frametime, usercmd_t *cmd, int active);
            void no_spread(float frametime, usercmd_t *cmd, int active);
    };
}