#pragma once
#include "../../Utils/globals.hpp"

namespace features
{
    class visuals
    {
        private:
            visuals()
            {
                this->enabled = false;
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
            bool enabled;
    };
}