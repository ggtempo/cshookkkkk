#pragma once
#include <cpptoml.h>
#include "../../Utils/globals.hpp"

/*
    Anti aim modes
    Pitch:
        Down (emotion)  -> -88
        Down (unsafe)   -> 180
        Down (lisp)     -> max_short * 360
        Up (emotion)    -> 88
        Up (unsafe)     -> -180
        Up (lisp)       -> max_short * -360

    Yaw:
        Forwards        -> Yaw + 0 (Player is lookig where he is looking)
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
        down_lisp,
        up_emotion,
        up_unsafe,
        up_lisp,
        user_defined
    };

    enum class aa_mode_yaw
    {
        off,
        forwards,
        backwards,
        left,
        right,
        spin,
        edge,
        user_defined
    };

    class anti_aim
    {
        private:
            anti_aim()
            {
                this->enabled = false;

                this->pitch_mode = aa_mode_pitch::off;
                this->yaw_mode = aa_mode_yaw::off;
                this->fake_yaw_mode = aa_mode_yaw::off;

                this->user_pitch = 0.0f;
                this->user_yaw = 0.0f;
            }
        
        public:
            void load_from_config(std::shared_ptr<cpptoml::table> config)
            {
                // Get correct section
                auto antiaim_table = config->get_table("anti-aim");

                // If table doesn't exist, just create an empty one
                if (!antiaim_table)
                {
                    antiaim_table = cpptoml::make_table();
                }

                // Get all values or their respective defaults
                this->enabled = antiaim_table->get_as<bool>("enabled").value_or(false);
                this->at_target = antiaim_table->get_as<bool>("at_target").value_or(false);
                this->pitch_mode = static_cast<aa_mode_pitch>(antiaim_table->get_as<int>("pitch_mode").value_or(0));
                this->yaw_mode = static_cast<aa_mode_yaw>(antiaim_table->get_as<int>("yaw_mode").value_or(0));
                this->fake_yaw_mode = static_cast<aa_mode_yaw>(antiaim_table->get_as<int>("fake_yaw_mode").value_or(0));
                this->user_pitch = antiaim_table->get_as<double>("user_pitch").value_or(0.0);
                this->user_yaw = antiaim_table->get_as<double>("user_yaw").value_or(0.0);
                this->user_fake_yaw = antiaim_table->get_as<double>("user_fake_yaw").value_or(0.0);
            }

            void save_to_config(std::ofstream& config_stream)
            {
                // Create section
                config_stream << "[anti-aim]" << std::endl;

                // Write all relevant values
                config_stream
                    << "enabled = "                     << (this->enabled ? "true" : "false")           << std::endl
                    << "at_target = "                   << (this->at_target ? "true" : "false")         << std::endl
                    << "pitch_mode = "                  << static_cast<int>(this->pitch_mode)           << std::endl
                    << "yaw_mode = "                    << static_cast<int>(this->yaw_mode)             << std::endl
                    << "fake_yaw_mode = "               << static_cast<int>(this->fake_yaw_mode)        << std::endl
                    << "user_pitch = "                  << this->user_pitch                             << std::endl
                    << "user_yaw = "                    << this->user_yaw                               << std::endl
                    << "user_fake_yaw = "               << this->user_fake_yaw                          << std::endl;

                // Trailing newline
                config_stream << std::endl;
            }

        public:
            static anti_aim& instance()
            {
                static anti_aim aa;
                return aa;
            }

            void create_move(float frametime, usercmd_t *cmd, int active);
            void show_menu();

            void post_move_fix(usercmd_t* cmd, math::vec3& new_move);

        private:
            float find_angle_to_nearest_wall(float current_yaw);
            float find_angle_to_nearest_target(float current_yaw);

        private:
            bool            enabled;
            bool            at_target;

            aa_mode_pitch   pitch_mode;
            aa_mode_yaw     yaw_mode;
            aa_mode_yaw     fake_yaw_mode;

            float           user_pitch;
            float           user_yaw;
            float           user_fake_yaw;

        private:
            constexpr static auto emotion_angle = 88.0f;
            constexpr static auto unsafe_angle = 180.0f;
            constexpr static auto lisp_angle = std::numeric_limits<short>::max() * 360.0f;
    };
}