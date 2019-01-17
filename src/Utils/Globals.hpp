#include "../Memory/Memory.hpp"
#include "../HLSDK/enginefuncs.hpp"
#include "../HLSDK/clientfuncs.hpp"
#include "../HLSDK/playermove.hpp"
#include "../HLSDK/Studio.hpp"
#include "../HLSDK/StudioStructures.hpp"

class Globals
{
    private:
        Globals()
        {
            this->backtrack_enabled = false;
            this->bhop_enabled = false;

            this->backtrack_time = 0.0;

            this->trigger_enabled = false;
            this->trigger_team = false;


            this->studioModelRendererHook = nullptr;

            this->render_mode = render_modes::kRenderNormal;
            this->render_fx = render_effects::kRenderFxNone;
            this->s_nf = STUDIO_NF_CHROME;
            this->fx_amt = 255;
            this->clr = {};
        }

    public:
        static Globals& Instance()
        {
            static Globals g;
            return g;
        }

    public:
        // Settings
        bool backtrack_enabled;
        float backtrack_time;
        
        bool bhop_enabled;


        bool trigger_enabled;
        bool trigger_team;

        // Hooks
        Memory::VMTHook* studioModelRendererHook;

        // Pointers
        engine_studio_api_t* engine_studio;
        cl_enginefunc_t*	engine_funcs;
        cldll_func_t*		original_client_funcs;
        cldll_func_t*		client_funcs;
        playermove_t*		player_move;

        int render_mode;
        int render_fx;
        int s_nf;
        int fx_amt;
        vec3_t clr;

        int trace_mode = 0;
        int trace_flags = 0;
};