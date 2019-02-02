#pragma once
#include "../../Utils/globals.hpp"

namespace features
{
    class visuals
    {
        private:
            visuals()
            {
                this->chams = false;
                this->chams_team = false;

                this->team_color = {
                    0.0, 0.0, 1.0, 0.3
                };

                this->enemy_color = {
                    1.0, 0.0, 0.0, 0.3
                };

                this->chams_mode = chams_modes::CHAMS_NORMAL;
            }
        
        public:
            static visuals& instance()
            {
                static visuals aa;
                return aa;
            }

            void studio_render_model(CStudioModelRenderer* ecx);
            void show_menu();

        private:
            enum class chams_modes
            {
                CHAMS_NORMAL,
                CHAMS_NOTEXTURE
            };

        private:
            bool chams;
            bool chams_team;
            chams_modes chams_mode;

            custom::color4f team_color;
            custom::color4f enemy_color;
    };
}