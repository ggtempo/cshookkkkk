#pragma once
#include "../Memory/memory.hpp"
#include "../HLSDK/enginefuncs.hpp"
#include "../HLSDK/clientfuncs.hpp"
#include "../HLSDK/playermove.hpp"
#include "../HLSDK/Studio.hpp"
#include "../HLSDK/StudioStructures.hpp"
#include "../HLSDK/Globals.hpp"
#include "../HLSDK/Weapons.hpp"
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

            this->no_visual_recoil = false;
            this->no_recoil = false;
            this->bhop_enabled = false;
            
            
            this->render_mode = render_modes::kRenderNormal;
            this->render_fx = render_effects::kRenderFxNone;
            this->s_nf = STUDIO_NF_CHROME;
            this->fx_amt = 255;
            this->clr = {};

            this->studio_model_renderer_hook = nullptr;

            this->first = true;

            this->catch_keys = false;
            this->captured_key = -1;

            this->anti_aim_pitch_override = false;
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
        bool bhop_enabled;
        bool no_visual_recoil;
        bool no_recoil;

        int render_mode;
        int render_fx;
        int s_nf;
        int fx_amt;
        vec3_t clr;

        int trace_mode = 0;
        int trace_flags = 0;

        bool anti_aim_pitch_override;

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
        uintptr_t           original_cur_weapon;
        uintptr_t           original_score_attrib;
        uintptr_t           original_studio_check_bbox;

        globalvars_t*       game_globals;

        uintptr_t           get_weapon_info;

        uintptr_t           original_window_proc;
        uintptr_t           original_wgl_swap_buffers;
        HWND                main_window;

        // Helpers and useful data
        bool first;
        custom::player_data local_player_data;
        std::unordered_map<int, custom::player_data> player_data;
        math::vec3 punch_angles;

        // Key catching
        bool catch_keys;
        int captured_key;
};

inline CBasePlayerWeapon* get_weapon_info(int id)
{
    typedef CBasePlayerWeapon*(*fnGetWeaponInfo)(int id);
    static auto& g = globals::instance();
    static auto original_func = reinterpret_cast<fnGetWeaponInfo>(g.get_weapon_info);


    return original_func(id);
}