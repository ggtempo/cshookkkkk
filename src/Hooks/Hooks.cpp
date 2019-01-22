#include <cstring>
#include <Windows.h>
#include "hooks.hpp"
#include <gl/GL.h>

#include "../Utils/globals.hpp"
#include "../Utils/math.hpp"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl.h"

#include "../HLSDK/Parsemsg.hpp"

#include "../Features/AntiAim/antiaim.hpp"
#include "../Features/Triggerbot/triggerbot.hpp"
#include "../Features/Visuals/visuals.hpp"
#include "../Features/Aimbot/aimbot.hpp"

#include <iostream>

namespace hooks
{
    inline bool is_valid_player(cl_entity_s* entity)
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();
        return  (entity) && (entity != local) && (entity->index != local->index) &&
                (g.player_data[entity->index].team == custom::player_team::CT || g.player_data[entity->index].team == custom::player_team::T);
    }

    typedef LRESULT(__stdcall*WNDPRC)(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK hk_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        static auto& g = globals::instance();

        if (!g.first)
            ImGui_Impl_WndProcHandler(hWnd, uMsg, wParam, lParam);

        // Key released
        if (uMsg == WM_KEYUP)
        {
            if (wParam == VK_INSERT)
            {
                // Toggle menu
                g.menu_enabled = !g.menu_enabled;
            }

            if (g.catch_keys)
            {
                g.captured_key = wParam;
            }                
        }
        else if (uMsg == WM_LBUTTONUP && g.catch_keys)
        {
            g.captured_key = MK_LBUTTON;
        }
        else if (uMsg == WM_RBUTTONUP && g.catch_keys)
        {
            g.captured_key = MK_RBUTTON;
        }
        else if (uMsg == WM_MBUTTONUP && g.catch_keys)
        {
            g.captured_key = MK_MBUTTON;
        }
        else if (uMsg == WM_XBUTTONUP && g.catch_keys)
        {
            g.captured_key = GET_XBUTTON_WPARAM(wParam) + 0x4;
        }

        auto& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard || ImGui::IsMouseHoveringAnyWindow())
        {
            // Dont pass keyboard/mouse input to the game
            //return DefWindowProc(hWnd, uMsg, wParam, lParam);

            io.MouseDrawCursor = true;
        }
        else
        {
            io.MouseDrawCursor = false;
        }

        

        return reinterpret_cast<WNDPRC>(g.original_window_proc)(hWnd, uMsg, wParam, lParam);
    }

    BOOL __stdcall hk_wgl_swap_buffers(HDC hDc)
    {
        typedef BOOL(__stdcall *wglSwapBuffersFn)(HDC);

        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<wglSwapBuffersFn>(g.original_wgl_swap_buffers);

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        glViewport(0, 0, viewport[2], viewport[3]);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        // set the limits of our ortho projection
        glOrtho(0, viewport[2], viewport[3], 0, -1, 100);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glPushMatrix();
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Do our custom rendering

        if (g.first) {
            g.first = false;

            // Debug OpenGL Stuff
            auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            auto extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
            

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            auto& style = ImGui::GetStyle();

            io.MouseDrawCursor = false;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsClassic();

            // Setup Platform/Renderer bindings
            ImGui_Impl_Init(g.main_window);
        }
        
        ImGui_Impl_NewFrame();
        ImGui::NewFrame();

        if (g.menu_enabled && ImGui::BeginMainMenuBar())
        {
            SYSTEMTIME time = {};
            GetLocalTime(&time);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0, 0.0));

            auto title_bar_size = ImGui::GetWindowSize();

            ImGui::SameLine();

            ImGui::SetCursorPos(ImVec2(0.0, 0.0));
            if (ImGui::Button("    Aimbot    "))
                g.aimbot_menu_enabled = !g.aimbot_menu_enabled;

            ImGui::SameLine();
            ImGui::Separator();

            if (ImGui::Button("    Trigger   "))
                g.trigger_menu_enabled = !g.trigger_menu_enabled;

            ImGui::SameLine();
            ImGui::Separator();

            if (ImGui::Button("      ESP     "))
                g.esp_menu_enabled = !g.esp_menu_enabled;

            ImGui::SameLine();
            ImGui::Separator();

            if (ImGui::Button("   Anti-Aim   "))
                g.anti_aim_menu_enabled = !g.anti_aim_menu_enabled;

            ImGui::SameLine();
            ImGui::Separator();

            if (ImGui::Button("     MISC     "))
                g.misc_menu_enabled = !g.misc_menu_enabled;

            ImGui::SameLine();
            ImGui::Separator();

            float right_offset = 10;

            {
                auto size = ImGui::CalcTextSize("00:00:00");
                right_offset += size.x;
                ImGui::SetCursorPos(ImVec2(title_bar_size.x - right_offset, 0));
                ImGui::Text("%02d:%02d:%0-2d", time.wHour, time.wMinute, time.wSecond);

                right_offset += 110;
            }

            {
                auto size = ImGui::CalcTextSize("CSHook by Dminik");
                
                ImGui::SetCursorPos(ImVec2((title_bar_size.x / 2) - (size.x / 2), 0));
                ImGui::Text("CSHook by Dminik");
            }

            ImGui::PopStyleVar();

            ImGui::EndMainMenuBar();
        }

        if (g.aimbot_menu_enabled && g.menu_enabled)
        {
            features::aimbot::instance().show_menu();
        }

        if (g.trigger_menu_enabled && g.menu_enabled)
        {
            features::triggerbot::instance().show_menu();
        }

        if (g.esp_menu_enabled && g.menu_enabled)
        {
            features::visuals::instance().show_menu();
        }

        if (g.anti_aim_menu_enabled && g.menu_enabled)
        {
            features::anti_aim::instance().show_menu();
        }

        if (g.misc_menu_enabled && g.menu_enabled)
        {
            ImGui::Begin("Miscellaneous");
                ImGui::Checkbox("Bhop enabled", &g.bhop_enabled);
                ImGui::Checkbox("Visual no recoil", &g.no_visual_recoil);
                ImGui::Checkbox("No recoil", &g.no_recoil);
            ImGui::End();
        }

        ImGui::Render();
        ImGui_Impl_RenderDrawData(ImGui::GetDrawData());

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        return original_func(hDc);
    }

    typedef void(__thiscall* fnStudioRenderModel)(void* ecx);
    void __fastcall hk_studio_render_model(CStudioModelRenderer* ecx, void* edx)
    {
        static auto& g = globals::instance();
        static auto original_func = g.studio_model_renderer_hook->get_original_vfunc<fnStudioRenderModel>(18);
        auto entity = g.engine_studio->GetCurrentEntity();
        auto local = g.engine_funcs->GetLocalPlayer();

        //g.engine_funcs->Con_Printf("Studio called from player %i\n", entity->index);

        g.engine_studio->SetChromeOrigin();
        g.engine_studio->SetForceFaceFlags( 0 );

        if ( entity->curstate.renderfx == render_effects::kRenderFxGlowShell )
        {
            original_func(ecx);
        }
        else
        {
            features::visuals::instance().studio_render_model(ecx);
        }
    }

	uint32_t find_client_functions()
	{
		DWORD dw_export_pointer = memory::find_pattern("hw.dll", { 0x68, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x83, 0xC4, 0x0C, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00 }, 1, false);
		return dw_export_pointer;
	}

    void setup_hitboxes()
    {
        static auto& g = globals::instance();
        auto local = g.engine_funcs->GetLocalPlayer();
        auto entity = g.engine_studio->GetCurrentEntity();

        if (entity && entity->model && entity->player && entity != local)
        {
            auto model = g.engine_studio->SetupPlayerModel(entity->index);
            auto header = g.engine_studio->Mod_Extradata(model);

            auto body_parts = header->numhitboxes;

            if (body_parts > 0)
            {
                using transform_matrix = float[128][3][4];
                // We have a valid model
                auto p_transform = (transform_matrix*)g.engine_studio->StudioGetBoneTransform();

                auto studio_box = reinterpret_cast<mstudiobbox_t*>((byte*)header + header->hitboxindex);

                for (int i = 0; i < body_parts; i++)
                {
                    auto bone = studio_box[i].bone;
                    auto transform = math::matrix3x4((*p_transform)[bone]);
                    auto box = math::bbox{studio_box[i].bbmin, studio_box[i].bbmax};
                    g.player_data[entity->index].hitboxes[i] = {bone, false, box, transform};

                    //Vector vMin, vMax;
                    //VectorTransform( m_pStudioTransform[11].bbmin, (*m_pBoneTransform)[m_pStudioTransform[11].bone], vMin );
                    //VectorTransform( m_pStudioTransform[11].bbmax, (*m_pBoneTransform)[m_pStudioTransform[11].bone], vMax );
                    //g_Player[ pEnt->index ].vHitbox=(vMin+vMax)*0.5f;

                    //g_Player[ pEnt->index ].bHitbox=true;
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
                math::vec3 start = g.player_move->origin + g.player_move->view_ofs;

                math::vec3 max_transformed = hitbox.matrix.transform_vec3(hitbox.box.bbmax);
                math::vec3 min_transformed = hitbox.matrix.transform_vec3(hitbox.box.bbmin);
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
        }
    }

    typedef void(__cdecl*fnStudioEntityLight)(alight_s* plight);
    void hk_studio_entity_light(alight_s* plight)
    {
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<fnStudioEntityLight>(g.original_studio_entity_light);
        setup_hitboxes();

        
        original_func(plight);
    }

    int hk_studio_check_bbox()
    {
        typedef int(*fnStudioCheckBBox)();
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<fnStudioCheckBBox>(g.original_studio_check_bbox);

        auto entity = g.engine_studio->GetCurrentEntity();


        if (!g.player_data[entity->index].dormant && g.player_data[entity->index].alive)
            return 1;

        return original_func();
    }

	void CL_CreateMove(float frametime, usercmd_t *cmd, int active)
	{
        static auto& g = globals::instance();
		g.original_client_funcs->pCL_CreateMove(frametime, cmd, active);

		auto lp = g.engine_funcs->GetLocalPlayer();

        if (!lp || !lp->player || !active)
            return;

        auto original_angles = cmd->viewangles;
        //auto original_move = vec3_s{cmd->forwardmove, cmd->sidemove, cmd->}

        update_visibility();
        update_status();

		if (cmd->buttons & IN_JUMP && !(g.player_move->flags & FL_ONGROUND) && g.bhop_enabled)
		{
			cmd->buttons &= ~IN_JUMP;
		}
        
        features::aimbot::instance().create_move(frametime, cmd, active);
        features::triggerbot::instance().create_move(frametime, cmd, active);
        features::anti_aim::instance().create_move(frametime, cmd, active);

        //g.engine_funcs->Con_Printf("Player with weapon id %i can fire in: %f, %f\n", g.local_player_data.weapon.id, g.local_player_data.weapon.next_attack, g.local_player_data.weapon.next_primary_attack);

        if (g.no_recoil && (cmd->buttons & IN_ATTACK))
        {
            cmd->viewangles -= g.punch_angles * 2;
            cmd->viewangles.z = 0.0f;

            cmd->viewangles.normalize_angle();
        }

        auto move = vec3_t{cmd->forwardmove, cmd->sidemove, cmd->upmove};
        auto new_move = math::correct_movement(original_angles, cmd->viewangles, move);
            
        // Reset movement bits
        cmd->buttons &= ~IN_FORWARD;
        cmd->buttons &= ~IN_BACK;
        cmd->buttons &= ~IN_LEFT;
        cmd->buttons &= ~IN_RIGHT;

        cmd->forwardmove = new_move.x;
        cmd->sidemove = new_move.y;
        cmd->upmove = new_move.z;
	}

	void HUD_ClientMove(playermove_t* ppmove, int server)
	{
        static auto& g = globals::instance();

		g.original_client_funcs->pClientMove(ppmove, server);
		std::memcpy(g.player_move, ppmove, sizeof(playermove_t));
	}

    typedef int(*fnTeamInfo)(const char*, int, void*);
    int hk_team_info(const char *name, int size, void *buffer)
    {
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<fnTeamInfo>(g.original_team_info);

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

    void hk_post_run_cmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed )
    {
        static auto& g = globals::instance();
        static auto original_func = g.original_client_funcs->pPostRunCmd;

        // Update local weapon
        g.local_player_data.weapon.id = to->client.m_iId;
        g.local_player_data.weapon.clip = to->weapondata[to->client.m_iId].m_iClip;
        g.local_player_data.weapon.next_primary_attack = to->weapondata[to->client.m_iId].m_flNextPrimaryAttack;
        g.local_player_data.weapon.next_secondary_attack = to->weapondata[to->client.m_iId].m_flNextSecondaryAttack;
        g.local_player_data.weapon.in_reload = to->weapondata[to->client.m_iId].m_fInReload || !to->weapondata[to->client.m_iId].m_iClip;
        g.local_player_data.weapon.next_attack = to->client.m_flNextAttack;

        return original_func(from, to, cmd, runfuncs, time, random_seed);
    }

    // For now useless
    int hk_cur_weapon(const char *name, int size, void *buffer )
    {
        typedef int(*fnCurWeapon)(const char *, int, void *);
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<fnCurWeapon>(g.original_cur_weapon);

        return original_func(name, size, buffer);
    }

    typedef void(*fnCalcRefDef)(ref_params_t*);
    void hk_calc_ref_def(ref_params_t* params)
    {
        static auto& g = globals::instance();
        static auto original_func = g.original_client_funcs->pCalcRefdef;//reinterpret_cast<fnCalcRefDef>(g.original_ref_def);

        g.punch_angles.x = params->punchangle[0];
        g.punch_angles.y = params->punchangle[1];
        g.punch_angles.z = params->punchangle[2];

        if (g.no_visual_recoil)
        {
            params->punchangle[0] = 0.0f;
            params->punchangle[1] = 0.0f;
            params->punchangle[2] = 0.0f;
        }

        original_func(params);
    }

    
    int hk_score_attrib(const char* name, int size, void* buffer)
    {
        typedef int(*fnScoreAttrib)(const char*, int, void*);
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<fnScoreAttrib>(g.original_score_attrib);

        BEGIN_READ(buffer, size);

        uint8_t index = READ_BYTE();
        uint8_t status = READ_BYTE();

        if ((index >= 0) && (index < g.engine_funcs->GetMaxClients()) && (index != g.engine_funcs->GetLocalPlayer()->index))
        {
            if (g.player_data[index].alive && (status & 1))
                g.engine_funcs->Con_Printf("Player %i just died!\n");
            
            g.player_data[index].alive = !(status & 1);
        }
        else if (index != g.engine_funcs->GetLocalPlayer()->index)
        {
            g.local_player_data.alive = !(status & 1);
        }

        return original_func(name, size, buffer);
    }

	void init()
	{
        static auto& g = globals::instance();
		auto client = GetModuleHandle(L"client.dll");
        auto opengl_dll = GetModuleHandle(L"opengl32.dll");
        auto hw_dll = GetModuleHandle(L"hw.dll");

		auto cInitialize = reinterpret_cast<uint8_t*>(GetProcAddress(client, "Initialize"));
		g.engine_funcs = reinterpret_cast<cl_enginefunc_t*>(*(uint32_t*)(cInitialize + 0x1C));

		g.player_move = new playermove_t();

        // Find and save original/hooked client funcs
		g.client_funcs = reinterpret_cast<cldll_func_t*>(find_client_functions());
		g.original_client_funcs = new cldll_func_t();
		std::memcpy(g.original_client_funcs, g.client_funcs, sizeof(cldll_func_t));

        // Hook client funcs
		g.client_funcs->pCL_CreateMove = CL_CreateMove;
		g.client_funcs->pClientMove = HUD_ClientMove;
        g.client_funcs->pCalcRefdef = hk_calc_ref_def;
        g.client_funcs->pPostRunCmd = hk_post_run_cmd;

        uintptr_t wgl_swap_buffers = reinterpret_cast<uintptr_t>(GetProcAddress(opengl_dll, "wglSwapBuffers"));
        g.original_wgl_swap_buffers = reinterpret_cast<uintptr_t>(memory::hook_func2(wgl_swap_buffers, reinterpret_cast<uintptr_t>(hk_wgl_swap_buffers), 5));
        g.original_window_proc = SetWindowLongPtr(g.main_window, GWL_WNDPROC, (LONG_PTR)&hk_wnd_proc);

        uint32_t offset = reinterpret_cast<uint32_t>(memory::get_module_info("client.dll").lpBaseOfDll);
        auto HUD_GetStudioModelInterface = (uint32_t)hooks::get_client_funcs()->pStudioInterface;

        g.engine_studio = *reinterpret_cast<engine_studio_api_s**>(HUD_GetStudioModelInterface + 0x1A);
        auto studio_model_renderer = *reinterpret_cast<CStudioModelRenderer**>(HUD_GetStudioModelInterface + 0x27);

        g.studio_model_renderer_hook = new memory::vmt_hook(studio_model_renderer);
        g.studio_model_renderer_hook->hook_vfunc(reinterpret_cast<void*>(hk_studio_render_model), 18);
        g.studio_model_renderer_hook->hook();

        g.original_studio_entity_light = reinterpret_cast<uintptr_t>(g.engine_studio->StudioEntityLight);
        g.engine_studio->StudioEntityLight = hk_studio_entity_light;

        g.original_studio_check_bbox = reinterpret_cast<uintptr_t>(g.engine_studio->StudioCheckBBox);
        g.engine_studio->StudioCheckBBox = hk_studio_check_bbox;
        //g.original_hook_usr_msg = reinterpret_cast<uintptr_t>(g.engine_funcs->pfnHookUserMsg);
        //g.engine_funcs->pfnHookUserMsg = reinterpret_cast<pfnEngSrc_pfnHookUserMsg_t>(hkHookUserMsg);

        

        auto hook_usr_msg = (uint32_t)g.engine_funcs->pfnHookUserMsg;
        uint32_t register_usr_msg = *(uint32_t*)(hook_usr_msg + 0x1B) + hook_usr_msg + (0x1F);
        uint32_t first_usr_msg_ptr = *(uint32_t*)(register_usr_msg + 0xD);
        usermsg_t* first_usr_msg_entry = *reinterpret_cast<usermsg_t**>(first_usr_msg_ptr);

        // Messages are a linked list
        auto element = first_usr_msg_entry;
        while (element)
        {
            if (std::strcmp(element->szMsg, "TeamInfo") == 0)
            {
               // We found the right one
               g.original_team_info = (uintptr_t)element->pfn;
               element->pfn = hk_team_info;
            }

            if (std::strcmp(element->szMsg, "ScoreAttrib") == 0)
            {
                g.original_score_attrib = (uintptr_t)element->pfn;
                element->pfn = hk_score_attrib;
            }
            // Go to the next one
            element = element->pNext;
        }
	}

    cl_enginefunc_t* get_engine_funcs()
    {
        return globals::instance().engine_funcs;
    }

    cldll_func_t* get_client_funcs()
    {
        return globals::instance().original_client_funcs;
    }
}

