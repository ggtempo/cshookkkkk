#include "removals.hpp"

namespace features
{
    void removals::create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto lp = g.engine_funcs->GetLocalPlayer();

        this->no_recoil(frametime, cmd, active);
        this->no_spread(frametime, cmd, active);
    }

    void removals::calc_ref_def(ref_params_t* params)
    {
        static auto& g = globals::instance();

        g.punch_angles.x = params->punchangle[0];
        g.punch_angles.y = params->punchangle[1];
        g.punch_angles.z = params->punchangle[2];

        if (g.no_visual_recoil)
        {
            params->punchangle[0] = 0.0f;
            params->punchangle[1] = 0.0f;
            params->punchangle[2] = 0.0f;
        }
    }

    void removals::no_recoil(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto lp = g.engine_funcs->GetLocalPlayer();

        if (g.no_recoil && (cmd->buttons & IN_ATTACK) && custom::is_gun(g.local_player_data.weapon.id) &&
            (g.local_player_data.weapon.next_attack <= 0.0) && (g.local_player_data.weapon.next_primary_attack <= 0.0) &&
            !g.local_player_data.weapon.in_reload)
        {
            cmd->viewangles -= g.punch_angles * 2;
            cmd->viewangles.z = 0.0f;

            cmd->viewangles.normalize_angle();
        }
    }

    void removals::no_spread(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        auto lp = g.engine_funcs->GetLocalPlayer();

        if (g.no_spread && (cmd->buttons & IN_ATTACK) && custom::is_gun(g.local_player_data.weapon.id) &&
            (g.local_player_data.weapon.next_attack <= 0.0) && (g.local_player_data.weapon.next_primary_attack <= 0.0) &&
            !g.local_player_data.weapon.in_reload)
        {
            // Get necessary info
            auto info = get_weapon_info(g.local_player_data.weapon.id);
            float velocity = math::vec3(g.local_player_data.velocity.x, g.local_player_data.velocity.y, 0.0f).length();
            auto spread = custom::get_spread(g.local_player_data.weapon.id, info->m_flAccuracy, velocity,
                            g.player_move->flags & FL_ONGROUND, g.player_move->flags & FL_DUCKING,
                            0.0f, info->m_iWeaponState);
            unsigned int shared_rand = g.local_player_data.weapon.seed;

            math::vec3 view_angles = cmd->viewangles.normalize_angle();

            auto spread_vec = custom::get_spread_vec(shared_rand, spread);

            math::vec3 forward, right, up, direction;
            math::vec3 temp = {0.0, 0.0, 0.0};
            temp.to_vectors(forward, right, up);

            direction = (forward + (right * spread_vec.x) + (up * spread_vec.y)).normalize();
            auto angles = direction.to_angles();
            angles.x -= view_angles.x;

            angles.normalize_angle();
            angles.transpose(forward, right, up);
            direction = forward;

            angles = direction.to_angles(up);
            angles.normalize_angle();
            angles.y += view_angles.y;
            angles.normalize_angle();

            cmd->viewangles = angles;
        }
    }
}