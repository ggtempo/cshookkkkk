#include "../Memory/Memory.hpp"
#include "../HLSDK/enginefuncs.hpp"
#include "../HLSDK/clientfuncs.hpp"
#include "../HLSDK/playermove.hpp"
#include "../HLSDK/Studio.hpp"
#include "../HLSDK/StudioStructures.hpp"
#include "custom.hpp"

#include <unordered_map>

class globals
{
    private:
        globals()
        {
            this->backtrack_enabled = false;
            this->bhop_enabled = false;

            this->backtrack_time = 0.0;

            this->trigger_enabled = false;
            this->trigger_team = false;


            this->studio_model_renderer_hook = nullptr;

            this->render_mode = render_modes::kRenderNormal;
            this->render_fx = render_effects::kRenderFxNone;
            this->s_nf = STUDIO_NF_CHROME;
            this->fx_amt = 255;
            this->clr = {};

            this->first = true;

            this->angles = {0.0, 0.0, 0.0};
            this->angles2 = {0.0, 0.0, 0.0};
            this->move = {0.0, 0.0, 0.0};
            this->move2 = {0.0, 0.0, 0.0};
        }

    public:
        static globals& instance()
        {
            static globals g;
            return g;
        }

    public:
        // Settings
        // TODO: move somewhere else
        bool backtrack_enabled;
        float backtrack_time;
        
        bool bhop_enabled;


        bool trigger_enabled;
        bool trigger_team;

        int render_mode;
        int render_fx;
        int s_nf;
        int fx_amt;
        vec3_t clr;

        int trace_mode = 0;
        int trace_flags = 0;

        int trigger_bone = 0;

        vec3_t angles;
        vec3_t angles2;
        vec3_t move;
        vec3_t move2;

        // Hooks
        Memory::VMTHook* studio_model_renderer_hook;

        // Pointers
        engine_studio_api_t* engine_studio;
        cl_enginefunc_t*	engine_funcs;
        cldll_func_t*		original_client_funcs;
        cldll_func_t*		client_funcs;
        playermove_t*		player_move;

        uintptr_t           original_studio_entity_light;

        uint32_t original_window_proc;
        HWND main_window;

        // Helpers and useful data
        bool first;
        std::unordered_map<int, custom::player_data> player_data;
};