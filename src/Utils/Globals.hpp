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
            this->mirror_cam_enabled = false;
            this->third_person_enabled = false;

            this->no_spread = false;

            this->taking_screenshot = false;
            this->taking_snapshot = false;
            this->hide_on_screenshot = false;

            this->studio_model_renderer_hook = nullptr;

            this->first = true;

            this->catch_keys = false;
            this->captured_key = -1;




            this->mirrorcam_buffer = 0;
            this->mirrorcam_texture = 0;
            this->mirrorcam_depth_buffer = 0;
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

        bool no_spread;

        bool mirror_cam_enabled;
        bool third_person_enabled;

        bool hide_on_screenshot;
        bool taking_screenshot;
        bool taking_snapshot;

        // Module base path
        std::string base_path;

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

        uintptr_t           original_screenshot;
        uintptr_t           original_snapshot;

        globalvars_t*       game_globals;
        globalvars_t**      game_globals_2;

        double*             engine_time;

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

        // Opengl
        unsigned int mirrorcam_buffer;
        unsigned int mirrorcam_texture;
        unsigned int mirrorcam_depth_buffer;
};

inline CBasePlayerWeapon* get_weapon_info(int id)
{
    typedef CBasePlayerWeapon*(*fnGetWeaponInfo)(int id);
    static auto& g = globals::instance();
    static auto original_func = reinterpret_cast<fnGetWeaponInfo>(g.get_weapon_info);


    return original_func(id);
}