#pragma once
#include <cstring>
#include "hooks.hpp"

#include "../Utils/globals.hpp"
#include "../Utils/math.hpp"

#include "../HLSDK/Parsemsg.hpp"
#include "../HLSDK/Weapons.hpp"
#include "../HLSDK/Textures.hpp"

#include "../Features/Utils/utils.hpp"

namespace hooks
{
    // For now useless
    int hk_cur_weapon(const char *name, int size, void *buffer )
    {
        using cur_weapon_fn = int(*)(const char *, int, void *);
        
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<cur_weapon_fn>(g.original_cur_weapon);

        return original_func(name, size, buffer);
    }

    int hk_score_attrib(const char* name, int size, void* buffer)
    {
        using score_attrib_fn = int(*)(const char*, int, void*);

        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<score_attrib_fn>(g.original_score_attrib);

        BEGIN_READ(buffer, size);

        uint8_t index = READ_BYTE();
        uint8_t status = READ_BYTE();

        if ((index >= 0) && (index < g.engine_funcs->GetMaxClients()) && (index != g.engine_funcs->GetLocalPlayer()->index))
        {           
            g.player_data[index].alive = !(status & 1);
        }
        else if (index == g.engine_funcs->GetLocalPlayer()->index)
        {
            g.local_player_data.alive = !(status & 1);
        }

        return original_func(name, size, buffer);
    }

    int hk_team_info(const char *name, int size, void *buffer)
    {
        using team_info_fn = int(*)(const char*, int, void*);

        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<team_info_fn>(g.original_team_info);

        BEGIN_READ(buffer, size);

        uint8_t index = READ_BYTE();
        char *team_str = READ_STRING();

        auto local = g.engine_funcs->GetLocalPlayer();

        if (index >= 0 && index < g.engine_funcs->GetMaxClients())
        {
            custom::player_team team;
            if (std::strcmp(team_str, "CT") == 0)
            {
                // Player is a ct
                team = custom::player_team::CT;
            }
            else if (std::strcmp(team_str, "TERRORIST") == 0)
            {
                // Player is a t
                team = custom::player_team::T;
            }
            else
            {
                // Player is a spectator/unknown
                team = custom::player_team::UNKNOWN;
            }

            if (index == local->index)
            {
                g.local_player_data.team = team;
            }
            else
            {
                g.player_data[index].team = team;
            }
        }

        return original_func(name, size, buffer);
    }

    int hk_set_fov(const char *name, int size, void *buffer)
    {
        using set_fov_fn = int(*)(const char*, int, void*);

        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<set_fov_fn>(g.original_set_fov);

        BEGIN_READ(buffer, size);

        auto fov = READ_BYTE();

        g.fov = fov;

        return original_func(name, size, buffer);
    }
}