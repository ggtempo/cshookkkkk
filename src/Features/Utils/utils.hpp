#pragma once
#include "../../Utils/globals.hpp"

namespace utils
{
    inline bool is_valid_player(cl_entity_s* entity)
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        return  (entity) && (entity != local) && (entity->index != local->index) &&
                g.player_data[entity->index].alive && !g.player_data[entity->index].dormant;
    }

    void setup_hitboxes()
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();
        auto entity = g.engine_studio->GetCurrentEntity();

        if (entity && entity->model && entity->player && entity != local)
        {
            // Get model info
            auto model = g.engine_studio->SetupPlayerModel(entity->index);
            auto header = g.engine_studio->Mod_Extradata(model);

            // How many hitboxes does the player have
            auto body_parts = header->numhitboxes;

            // If he has any
            if (body_parts > 0)
            {
                using transform_matrix = float[128][3][4];
                auto p_transform = (transform_matrix*)g.engine_studio->StudioGetBoneTransform();

                // Get hitbox info
                auto studio_box = reinterpret_cast<mstudiobbox_t*>((byte*)header + header->hitboxindex);

                // Go through every hitbox
                for (int i = 0; i < body_parts; i++)
                {
                    // Copy important data
                    auto bone = studio_box[i].bone;                                                 // Bone ID
                    auto transform = math::matrix3x4((*p_transform)[bone]);                         // Transform matrix
                    auto box = math::bbox{studio_box[i].bbmin, studio_box[i].bbmax};                // Hitbox coordinates (Top left corner + Bottom right corner)
                    g.player_data[entity->index].hitboxes[i] = {bone, false, box, transform};       // Store the data
                }
            }
        }
    }

    void update_visibility()
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        for (auto i = 0; i < g.engine_funcs->GetMaxClients(); i++)
        {
            auto entity = g.engine_funcs->GetEntityByIndex(i);

            if (!is_valid_player(entity))
                continue;

            for (auto& [key, hitbox] : g.player_data[entity->index].hitboxes)
            {
                auto matrix = hitbox.matrix;

                // Temporary fix agains people with roll antiaim
                if (entity->curstate.angles.z != 0.0)
                {
                    matrix = matrix.angle_matrix(math::vec3{0.0, 0.0, -entity->curstate.angles.z});
                }

                math::vec3 start = g.player_move->origin + g.player_move->view_ofs;

                math::vec3 max_transformed = matrix.transform_vec3(hitbox.box.bbmax);
                math::vec3 min_transformed = matrix.transform_vec3(hitbox.box.bbmin);
                math::vec3 center = (max_transformed + min_transformed) * 0.5;


                //auto trace = *g.engine_funcs->PM_TraceLine(start, end, PM_TRACELINE_PHYSENTSONLY, 0, lp->index);
                pmtrace_t world_trace = {};

                // For some reason, the trace functions are very imprecise when concerning players
                // However, they seem to be precise when tracing just against the world
                // So, we can check if the trace hit the world
                g.engine_funcs->pEventAPI->EV_SetTraceHull(2);
                g.engine_funcs->pEventAPI->EV_PlayerTrace(start, center, PM_WORLD_ONLY, -1, &world_trace);
                
                if (world_trace.fraction == 1.0f)
                {
                    // If not, check if we hit a non player with another trace
                    // To check against boxes/doors/whatever

                    pmtrace_t trace = {};
                    g.engine_funcs->pEventAPI->EV_SetTraceHull(2);
                    g.engine_funcs->pEventAPI->EV_PlayerTrace(start, center, PM_GLASS_IGNORE, -1, &trace);
                    auto trace_entity_index = g.engine_funcs->pEventAPI->EV_IndexFromTrace(&trace);
                    
                    if (!trace_entity_index)
                    {
                        // We didn't hit any entity, thus the hitbox is visible
                        hitbox.visible = true;
                    }
                    else
                    {
                        // We hit another entity
                        auto trace_entity = g.engine_funcs->GetEntityByIndex(trace_entity_index);

                        if (!trace_entity->player)
                        {
                            // Entity is not a player, thus is a map object
                            hitbox.visible = false;
                        }
                        else// if (((trace_entity->index != i) && (trace_entity != local) && (trace_entity->index != local->index)))
                        {
                            // We hit a different player, thus we consider hitbox visible
                            hitbox.visible = true;
                        }
                    }

                    //hitbox.visible = true;
                }
                else
                {
                    // We hit the world
                    hitbox.visible = false;
                }
            }
        }
    }

    void update_status()
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        // Reset everyone as dormant
        for (auto& [key, value] : g.player_data)
        {
            value.dormant = true;
        }

        for (size_t i = 1; i < g.engine_funcs->GetMaxClients(); i++)
        {
            auto entity = g.engine_funcs->GetEntityByIndex(i);

            auto updated = true;

            // Check if local player exists
            if (!updated || !(local))
                updated = false;

            // Check if exists or is a local player
            if (!updated || !(entity) || (entity == local) || (entity->index == local->index) || entity->index < 0 || entity->index > g.engine_funcs->GetMaxClients())
                updated = false;

            // Check if is a player or has a model/gun model
            if (!updated || !(entity->model) || !(entity->player) || !(entity->curstate.weaponmodel))
                updated = false;

            // Check if the player belongs to a team
            if (!updated || g.player_data[entity->index].team == custom::player_team::UNKNOWN)
                updated = false;

            // Mins and maxs
            if (!updated || entity->curstate.mins.is_zero() || entity->curstate.maxs.is_zero())
                updated = false;

            if (!updated || entity->curstate.messagenum < local->curstate.messagenum)
                updated = false;

            /*if (!updated || (local->curstate.iuser1 == 4 && local->curstate.iuser2 == entity->index))
                updated = false;*/

            g.player_data[entity->index].dormant = !updated;

            if (updated)
            {
                // Update position and velocity
                g.player_data[entity->index].origin = entity->origin;
                g.player_data[entity->index].velocity = entity->curstate.velocity;

                hud_player_info_t info = {};
                g.engine_funcs->pfnGetPlayerInfo(entity->index, &info);

                g.player_data[entity->index].name = info.name;
            }
            
        }
    }
}