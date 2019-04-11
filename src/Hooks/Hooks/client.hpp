#pragma once
#include <cstring>
#include "../hooks.hpp"

#include "../../Utils/utils.hpp"
#include "../../Utils/globals.hpp"
#include "../../Utils/math.hpp"

#include "../../Features/AntiAim/antiaim.hpp"
#include "../../Features/Triggerbot/triggerbot.hpp"
#include "../../Features/Aimbot/aimbot.hpp"
#include "../../Features/Removals/removals.hpp"


namespace hooks
{
    void hk_cl_create_move(float frametime, usercmd_t *cmd, int active)
    {
        static auto& g = globals::instance();
        g.original_client_funcs->pCL_CreateMove(frametime, cmd, active);

        auto lp = g.engine_funcs->GetLocalPlayer();

        // Reset create move
        g.send_packet = true;

        if (!lp || !lp->player || !active)
            return;

        // Update entity alive/dormant status
        utils::update_status();

        // Update local player velocity
        g.local_player_data.velocity = g.player_move->velocity;

        // Only proceed if connected and alive
        if (!g.connected || !g.local_player_data.alive)
            return;

        // Update entity visibility status
        utils::update_visibility();

        auto original_angles = cmd->viewangles;        
        
        features::miscelaneous::instance().create_move(frametime, cmd, active);
        features::aimbot::instance().create_move(frametime, cmd, active);
        features::triggerbot::instance().create_move(frametime, cmd, active);
        features::anti_aim::instance().create_move(frametime, cmd, active);
        features::removals::instance().create_move(frametime, cmd, active);

        auto move = vec3_t{cmd->forwardmove, cmd->sidemove, cmd->upmove};
        auto new_move = math::correct_movement(original_angles, cmd->viewangles, move);

        features::anti_aim::instance().post_move_fix(cmd, new_move);

        // Reset movement bits
        cmd->buttons &= ~IN_FORWARD;
        cmd->buttons &= ~IN_BACK;
        cmd->buttons &= ~IN_LEFT;
        cmd->buttons &= ~IN_RIGHT;

        if (new_move.x > 0)
        {
            cmd->buttons |= IN_FORWARD;
        }
        else if (new_move.x < 0)
        {
            cmd->buttons |= IN_BACK;
        }

        if (new_move.y > 0)
        {
            cmd->buttons |= IN_LEFT;
        }
        else if (new_move.y < 0)
        {
            cmd->buttons |= IN_RIGHT;
        }

        cmd->forwardmove = new_move.x;
        cmd->sidemove = new_move.y;
        cmd->upmove = new_move.z;

        g.last_cmd = cmd;
    }

    void hk_hud_clientmove(playermove_t* ppmove, int server)
    {
        static auto& g = globals::instance();

        g.original_client_funcs->pClientMove(ppmove, server);
        std::memcpy(g.player_move, ppmove, sizeof(playermove_t));

        PM_InitTextureTypes(ppmove);
    }

    void hk_post_run_cmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed )
    {
        static auto& g = globals::instance();
        static auto original_func = g.original_client_funcs->pPostRunCmd;

        original_func(from, to, cmd, runfuncs, time, random_seed);

        if (!runfuncs)
            return;

        // Update local weapon
        auto info = utils::get_weapon_info((custom::weapon_id)to->client.m_iId);

        g.local_player_data.weapon.id = static_cast<custom::weapon_id>(to->client.m_iId);
        g.local_player_data.weapon.clip = to->weapondata[to->client.m_iId].m_iClip;
        g.local_player_data.weapon.next_primary_attack = to->weapondata[to->client.m_iId].m_flNextPrimaryAttack;
        g.local_player_data.weapon.next_secondary_attack = to->weapondata[to->client.m_iId].m_flNextSecondaryAttack;
        g.local_player_data.weapon.in_reload = to->weapondata[to->client.m_iId].m_fInReload || !to->weapondata[to->client.m_iId].m_iClip;
        g.local_player_data.weapon.next_attack = to->client.m_flNextAttack;

        if (info)
        {
            g.local_player_data.weapon.seed = random_seed;
            g.local_player_data.weapon.accuracy = info->m_flAccuracy;
        }
    }

    void hk_calc_ref_def(ref_params_t* params)
    {
        static auto& g = globals::instance();
        static auto original_func = g.original_client_funcs->pCalcRefdef;

        if (params->nextView == 0)
        {
            features::removals::instance().calc_ref_def(params);
        }

        original_func(params);

        features::miscelaneous::instance().calc_ref_def(params);
    }

    void hk_client_move_init(playermove_s* pmove)
    {
        static auto& g = globals::instance();
        static auto original_func = g.original_client_funcs->pClientMoveInit;

        PM_InitTextureTypes(pmove);

        original_func(pmove);
    }

    bool hk_is_third_person()
    {
        return features::miscelaneous::instance().is_thirdperson();
    }

    int hk_hud_redraw(float time, int intermission)
    {
        static auto& g = globals::instance();
        static auto original_func = g.original_client_funcs->pHudRedrawFunc;

        // Switch to the original buffer for drawing the HUD
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);check_gl_error();
        //glDrawBuffer(GL_BACK);check_gl_error();        

        features::removals::instance().hud_redraw(time, intermission);
        return original_func(time, intermission);
    }
}