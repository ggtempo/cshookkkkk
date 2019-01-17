#include <cstring>
#include "Hooks.hpp"
#include <gl/GL.h>

#include "../Utils/Globals.hpp"

#include <iostream>

namespace Hooks
{
	uint32_t FindClientFuncs()
	{
		DWORD dwExportPointer = Memory::FindPattern("hw.dll", { 0x68, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x83, 0xC4, 0x0C, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00 }, 1, false);
		return dwExportPointer;
	}

	void CL_CreateMove(float frametime, usercmd_t *cmd, int active)
	{
        static auto& g = Globals::Instance();
		g.original_client_funcs->pCL_CreateMove(frametime, cmd, active);

		auto lp = g.engine_funcs->GetLocalPlayer();

        if (!lp || !lp->player)
            return;

		if (cmd->buttons & IN_JUMP && !(g.player_move->flags & FL_ONGROUND) && g.bhop_enabled)
		{
			cmd->buttons &= ~IN_JUMP;
		}

        if (g.backtrack_enabled && GetAsyncKeyState(VK_LMENU))
        {
            cmd->lerp_msec -= g.backtrack_time;
        }

        // Triggerbot testing
        if (g.trigger_enabled)
        {
            vec3_t offset = {};
            g.engine_funcs->pEventAPI->EV_LocalPlayerViewheight(offset);
            vec3_t start = g.player_move->origin + offset;
            vec3_t angles = cmd->viewangles;
            vec3_t forward = {};
            vec3_t end = {};

            g.engine_funcs->pfnAngleVectors(angles, forward, nullptr, nullptr);
            end = start.multiply_add(2148, forward);

            //auto trace = g.engine_funcs->PM_TraceLine(start, end, PM_TRACELINE_PHYSENTSONLY, 1, lp->index);
            pmtrace_t trace = {};

            g.engine_funcs->pEventAPI->EV_SetTraceHull(g.trace_mode);
            g.engine_funcs->pEventAPI->EV_PlayerTrace(start, end, g.trace_flags, lp->index, &trace);
            //trace = TraceLine(start, end, forward);
            if (trace.fraction != 1.0f)
            {
                g.engine_funcs->Con_Printf("Trace hit: %f, entity: %i\n", trace.fraction, trace.ent);
                // We have a hit

                if (trace.ent > 0 && trace.ent <= g.player_move->numphysent)
                {
                    g.engine_funcs->Con_Printf("Trace hit a valid entity!\n");
                    // Get the actual player
                    // trace->ent is the physical entity number
                    // We need to get the normal one
                    // From playermove?
                    auto target = g.engine_funcs->GetEntityByIndex(g.player_move->physents[trace.ent].info);
                    
                    // If we have a valid target
                    if (target->player && g.trigger_enabled && ((target->curstate.team != lp->curstate.team) || g.trigger_team))
                    {
                        g.engine_funcs->Con_Printf("Entity is also a valid player, firing at hitgroup: %i\n", trace.hitgroup);
                        // Fire
                        cmd->buttons |= IN_ATTACK;
                    }
                }
            }
        }
	}

	void HUD_ClientMove(playermove_t* ppmove, int server)
	{
        static auto& g = Globals::Instance();

		g.original_client_funcs->pClientMove(ppmove, server);
		std::memcpy(g.player_move, ppmove, sizeof(playermove_t));
	}

	void Init()
	{
        static auto& g = Globals::Instance();
		auto client = GetModuleHandle(L"client.dll");

		auto cInitialize = reinterpret_cast<uint8_t*>(GetProcAddress(client, "Initialize"));
		g.engine_funcs = reinterpret_cast<cl_enginefunc_t*>(*(uint32_t*)(cInitialize + 0x1C));

		g.player_move = new playermove_t();

		cldll_func_t* funcs = reinterpret_cast<cldll_func_t*>(FindClientFuncs());

		g.original_client_funcs = new cldll_func_t();
		std::memcpy(g.original_client_funcs, funcs, sizeof(cldll_func_t));

		funcs->pCL_CreateMove = CL_CreateMove;
		funcs->pClientMove = HUD_ClientMove;
	}

    void Print(const char* text)
    {
        Globals::Instance().engine_funcs->pfnConsolePrint(text);
    }

    cl_enginefunc_t* GetEngineFuncs()
    {
        return Globals::Instance().engine_funcs;
    }

    cldll_func_t* GetClientFuncs()
    {
        return Globals::Instance().original_client_funcs;
    }
}

