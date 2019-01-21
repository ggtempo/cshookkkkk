#pragma once
#include "../../Utils/globals.hpp"

/*
    Anti aim modes
    Pitch:
        Down (emotion)  -> 89
        Down (unsafe)   -> 179
        Up (emotion)    -> -89
        Up (unsafe)     -> -179


    Yaw:
        Backwards       -> Yaw + 180
        Left            -> Yaw + 90
        Right           -> Yaw - 90
        Spin            -> Angle + spin speed


*/

namespace features
{
    enum class aa_mode_pitch
    {
        off,
        down_emotion,
        down_unsafe,
        up_emotion,
        up_unsafe
    };

    enum class aa_mode_yaw
    {
        off,
        backwards,
        left,
        right,
        spin
    };

    class anti_aim
    {
        private:
            anti_aim()
            {
                this->enabled = false;
                this->pitch_mode = aa_mode_pitch::off;
                this->yaw_mode = aa_mode_yaw::off;
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
            bool            enabled;
            aa_mode_pitch   pitch_mode;
            aa_mode_yaw     yaw_mode;
    };
}