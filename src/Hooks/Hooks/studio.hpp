#pragma once
#include <cstring>
#include <Windows.h>
#include "../hooks.hpp"
#include <glad/gl.h>

#include "../../Utils/globals.hpp"
#include "../../Utils/math.hpp"

#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_impl.h"

#include "../../HLSDK/Parsemsg.hpp"

#include "../../Features/AntiAim/antiaim.hpp"
#include "../../Features/Triggerbot/triggerbot.hpp"
#include "../../Features/Visuals/visuals.hpp"
#include "../../Features/Aimbot/aimbot.hpp"
#include "../../Features/Removals/removals.hpp"
#include "../../HLSDK/Weapons.hpp"
#include "../../HLSDK/Textures.hpp"

#include "../../Features/Utils/utils.hpp"

namespace hooks
{
    void __fastcall hk_studio_render_model(CStudioModelRenderer* ecx, void* edx)
    {
        using studio_render_model_fn = void(__thiscall*)(void* ecx);

        static auto& g = globals::instance();
        static auto original_func = g.studio_model_renderer_hook->get_original_vfunc<studio_render_model_fn>(18);
        auto entity = g.engine_studio->GetCurrentEntity();
        auto local = g.engine_funcs->GetLocalPlayer();

        g.engine_studio->SetChromeOrigin();
        g.engine_studio->SetForceFaceFlags( 0 );

        if ( entity->curstate.renderfx == render_effects::kRenderFxGlowShell  || g.first)
        {
            original_func(ecx);
        }
        else
        {
            features::visuals::instance().studio_render_model(ecx);
        }
    }

    int __fastcall hk_StudioDrawPlayer(CStudioModelRenderer* ecx, void* edx, int flags, entity_state_s* pplayer)
    {
        using studio_draw_player_fn = int(__thiscall*)(CStudioModelRenderer* ecx, int flags, entity_state_s* pplayer);
        static auto& g = globals::instance();
        static auto original_func = g.studio_model_renderer_hook->get_original_vfunc<studio_draw_player_fn>(25);

        if (g.engine_studio->GetCurrentEntity() == g.engine_funcs->GetLocalPlayer())
        {
            auto original_return = original_func(ecx, flags, pplayer);

            auto entity = g.engine_studio->GetCurrentEntity();
            auto info = g.engine_studio->PlayerInfo(entity->index);
            auto original_entity = *entity;

            entity->angles.y += 180;
            info->gaityaw += 180;

            original_func(ecx, flags, pplayer);

            *entity = original_entity;
            return original_return;
        }

        //original_func(ecx, flags, pplayer);
        return original_func(ecx, flags, pplayer);
    }

    void hk_studio_entity_light(alight_s* plight)
    {
        using studio_entity_light_fn = void(*)(alight_s* plight);

        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<studio_entity_light_fn>(g.original_studio_entity_light);
        utils::setup_hitboxes();

        
        original_func(plight);
    }

    int hk_studio_check_bbox()
    {
        using studio_check_bbox_fn = int(*)();
        
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<studio_check_bbox_fn>(g.original_studio_check_bbox);

        auto entity = g.engine_studio->GetCurrentEntity();


        if (!g.player_data[entity->index].dormant && g.player_data[entity->index].alive)
            return 1;

        return original_func();
    }
}