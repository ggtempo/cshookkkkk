#pragma once
#include "../Memory/memory.hpp"
#include "../HLSDK/enginefuncs.hpp"
#include "../HLSDK/clientfuncs.hpp"
#include "../HLSDK/playermove.hpp"
#include "../HLSDK/Studio.hpp"
#include "../HLSDK/StudioStructures.hpp"
#include "../HLSDK/Globals.hpp"
#include "../HLSDK/Weapons.hpp"
#include "../HLSDK/Textures.hpp"
#include "custom.hpp"

#include <iostream>
#include <cstring>
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

            this->bhop_enabled = false;
            this->mirror_cam_enabled = false;
            this->third_person_enabled = false;

            this->taking_screenshot = false;
            this->taking_snapshot = false;
            this->hide_on_screenshot = false;

            this->studio_model_renderer_hook = nullptr;

            this->send_packet = true;
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
        bool mirror_cam_enabled;
        bool third_person_enabled;

        bool hide_on_screenshot;
        bool taking_screenshot;
        bool taking_snapshot;

        // SendPacket
        bool send_packet;

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
        uintptr_t           original_can_packet;

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

inline CBasePlayerWeapon* get_weapon_info(custom::weapon_id id)
{
    typedef CBasePlayerWeapon*(*fnGetWeaponInfo)(custom::weapon_id id);
    static auto& g = globals::instance();
    static auto original_func = reinterpret_cast<fnGetWeaponInfo>(g.get_weapon_info);


    return original_func(id);
}

inline pmtrace_t better_trace(math::vec3 start, math::vec3 end)
{   
    static auto& g = globals::instance();
    math::vec3 direction = (end - start).normalize();

    if (start == end)
    {
        pmtrace_t result = {};
        result.endpos = end;
        result.fraction = 1.0;
    }

    pmtrace_t result = {};
    g.engine_funcs->pEventAPI->EV_SetTraceHull(2);
    g.engine_funcs->pEventAPI->EV_PlayerTrace(start, end, PM_GLASS_IGNORE, -1, &result);

    if (result.fraction == 0.0f && result.startsolid && !result.allsolid)
    {
        math::vec3 temp = start;

        while (!result.allsolid && result.fraction == 0.0)
        {
            temp += direction;

            g.engine_funcs->pEventAPI->EV_SetTraceHull(2);
            g.engine_funcs->pEventAPI->EV_PlayerTrace(temp, end, PM_GLASS_IGNORE, -1, &result);
        }

        if (!result.allsolid && result.fraction != 1.0)
        {
            auto len1 = (end - start).length();
            auto len2 = (result.endpos - start).length();

            result.fraction = len2 / len1;
            result.startsolid = 1;
        }
    }
    
    if (result.allsolid)
        result.fraction = 1.0f;

    return result;
}

inline char get_texture_type(pmtrace_t& trace, math::vec3& start, math::vec3& end)
{
    static auto& g = globals::instance();

    auto index = g.engine_funcs->pEventAPI->EV_IndexFromTrace(&trace);

    auto texture_type = 'C';

    if (index >= 1 && index <= g.engine_funcs->GetMaxClients())
    {
        //Hit player
        texture_type = CHAR_TEX_FLESH;
    }
    else
    {
        // Hit a wall, time for texture trace
        auto texture_name = g.engine_funcs->pEventAPI->EV_TraceTexture(trace.ent, start, end);
        if (texture_name)
        {
            char texture[64];
            std::strncpy( texture, texture_name, sizeof( texture ) );
            texture_name = texture;

            bool sky = false;
            if( !std::strcmp( texture_name, "sky" ) )
            {
                sky = true;
            }

            // strip leading '-0' or '+0~' or '{' or '!'
            else if (*texture_name == '-' || *texture_name == '+')
            {
                texture_name += 2;
            }
            else if (*texture_name == '{' || *texture_name == '!' || *texture_name == '~' || *texture_name == ' ')
            {
                texture_name++;
            }

            // '}}'
            char texture_name_final[64];
            strncpy( texture_name_final, texture_name, sizeof(texture_name_final));
            texture_name_final[ 16 ] = 0;

            texture_type = PM_FindTextureType(texture_name_final);
        }
    }

    return texture_type;
}

inline int get_estimated_damage(math::vec3 start, math::vec3 end, custom::weapon_id weapon_id, int target_id, int hitbox_id)
{
    static auto& g = globals::instance();

    auto weapon = custom::get_weapon_params(weapon_id);

    // Testing with ak
    int penetration_power = weapon.bullet.penetration_power;
    float penetration_distance = weapon.bullet.penetration_range;
    int penetration_count = weapon.max_penetrations + 1;
    float max_distance = weapon.range;
    int estimated_damage = weapon.damage;

    // Auto wall testing
    vec3_t forward = (end - start).normalize();
    vec3_t max_end = start + (forward * penetration_distance);

    math::vec3 original_start = start;
    math::vec3 original_end = end;

    float damage_modifier = 0.5;

    float remaining_distance = max_distance;
    while (penetration_count != 0)
    {
        pmtrace_t trace = better_trace(start, max_end);
        auto texture_type = get_texture_type(trace, start, max_end);

        auto index = g.engine_funcs->pEventAPI->EV_IndexFromTrace(&trace);
        if ((index && index == target_id) ||                                                        // We hit the target player
            (trace.endpos - original_start).length() > (original_end - original_start).length())    // Or, we went behind him (if the server blocks player traces)
        {
            break;
        }

        float current_distance = trace.fraction * remaining_distance;

        if (current_distance > penetration_distance)
            return 0;                  // Bullet cannot go through
        else
            penetration_count--;       // Bullet made it through the object

        // We have a hit
        switch (texture_type)
        {
            case CHAR_TEX_METAL:
                penetration_power *= 0.15;
                damage_modifier = 0.2;
                break;
            case CHAR_TEX_CONCRETE:
                penetration_power *= 0.25;
                damage_modifier = 0.5;
                break;
            case CHAR_TEX_VENT:
                penetration_power *= 0.5;
                damage_modifier = 0.45;
                break;
            case CHAR_TEX_GRATE:
                penetration_power *= 0.5;
                damage_modifier = 0.4;
                break;
            case CHAR_TEX_TILE:
                penetration_power *= 0.65;
                damage_modifier = 0.3;
                break;
            case CHAR_TEX_COMPUTER:
                penetration_power *= 0.4;
                damage_modifier = 0.3;
                break;
            case CHAR_TEX_WOOD:
                penetration_power *= 1;
                damage_modifier = 0.6;
                break;
            default:
                break;
        }

        remaining_distance = (remaining_distance - current_distance) * 0.5;

        start = trace.endpos + (forward * penetration_power);
        max_end = start + (forward * remaining_distance);
        estimated_damage = (estimated_damage * damage_modifier);

        if (penetration_count <= 0)
        {
            estimated_damage = 0;
            break;
        }
    }

    estimated_damage = estimated_damage * (std::pow(weapon.damage_dropoff, (int)((end - start).length() / 500)));    // Account for range
    auto hitbox_damage_modifier = custom::get_hitbox_damage_modifier((hitbox_numbers)hitbox_id);
    estimated_damage = estimated_damage * hitbox_damage_modifier;

    // Getting armor/health values in this game is a bit retarder
    // For now, we will just assume that the enemy player always has full armor

    estimated_damage *= weapon.armor_penetration;

    return estimated_damage;                           // We didn't hit the player / got stuck in the wall
}