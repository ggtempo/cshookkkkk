#include "../Memory/memory.hpp"
#include "../HLSDK/enginefuncs.hpp"
#include "../HLSDK/clientfuncs.hpp"
#include "../HLSDK/playermove.hpp"
#include "../HLSDK/Studio.hpp"
#include "../HLSDK/StudioStructures.hpp"
#include "custom.hpp"

#include <unordered_set>
#include <unordered_map>

class globals
{
    private:
        globals()
        {
            this->menu_enabled = true;
            this->aimbot_menu_enabled = false;
            this->trigger_menu_enabled = false;
            this->esp_menu_enabled = false;
            this->anti_aim_menu_enabled = false;
            this->misc_menu_enabled = false;

            this->trigger_enabled = false;
            this->trigger_team = false;
            this->trigger_hitboxes_all = false;

            this->anti_aim_enabled = false;
            this->bhop_enabled = false;
            
            
            this->render_mode = render_modes::kRenderNormal;
            this->render_fx = render_effects::kRenderFxNone;
            this->s_nf = STUDIO_NF_CHROME;
            this->fx_amt = 255;
            this->clr = {};

            this->studio_model_renderer_hook = nullptr;

            this->first = true;
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
        // Menu settings
        bool menu_enabled;
        bool aimbot_menu_enabled;
        bool trigger_menu_enabled;
        bool esp_menu_enabled;
        bool anti_aim_menu_enabled;
        bool misc_menu_enabled;

        // Hack settings
        bool anti_aim_enabled;
        
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

        std::unordered_map<int, bool> trigger_hitboxes;
        bool trigger_hitboxes_all;
        int trigger_bone = 0;

        // Hooks
        memory::vmt_hook* studio_model_renderer_hook;

        // Pointers
        engine_studio_api_t* engine_studio;
        cl_enginefunc_t*	engine_funcs;
        cldll_func_t*		original_client_funcs;
        cldll_func_t*		client_funcs;
        playermove_t*		player_move;

        uintptr_t           original_studio_entity_light;
        uintptr_t           original_team_info;

        uint32_t original_window_proc;
        HWND main_window;

        // Helpers and useful data
        bool first;
        std::unordered_map<int, custom::player_data> player_data;
};