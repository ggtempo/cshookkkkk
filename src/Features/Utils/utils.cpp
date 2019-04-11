#include "utils.hpp"

namespace utils
{
    bool is_valid_player(cl_entity_s* entity)
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        return  (entity) &&
                (entity != local) &&
                (entity->index != local->index) &&
                g.player_data[entity->index].alive &&
                !g.player_data[entity->index].dormant;
    }

    CBasePlayerWeapon* get_weapon_info(custom::weapon_id id)
    {
        typedef CBasePlayerWeapon*(*fnGetWeaponInfo)(custom::weapon_id id);
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<fnGetWeaponInfo>(g.get_weapon_info);


        return original_func(id);
    }

    void update_status()
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        net_status_s status = {};
        g.engine_funcs->pNetAPI->Status(&status);

        g.connected = status.connected;

        // Reset everyone as dormant
        for (auto& [key, value] : g.player_data)
        {
            value.dormant = true;
        }

        for (auto i = 1; i <= g.engine_funcs->GetMaxClients(); i++)
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

            if (!updated || entity->curstate.solid != 3 || (local->curstate.iuser1 == 4 && local->curstate.iuser2 == entity->index))
                updated = false;

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

    void update_visibility()
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();

        for (auto i = 1; i <= g.engine_funcs->GetMaxClients(); i++)
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

    pmtrace_t better_trace(math::vec3 start, math::vec3 end)
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

    char get_texture_type(pmtrace_t& trace, math::vec3& start, math::vec3& end)
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

    int get_estimated_damage(math::vec3 start, math::vec3 end, custom::weapon_id weapon_id, int target_id, int hitbox_id)
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
}