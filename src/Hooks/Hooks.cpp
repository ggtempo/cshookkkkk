#include <cstring>
#include <Windows.h>
#include "Hooks.hpp"
#include <gl/GL.h>

#include "../Utils/globals.hpp"
#include "../Utils/math.hpp"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl.h"

#include <iostream>

namespace Hooks
{
    typedef LRESULT(__stdcall*WNDPRC)(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        static auto& g = globals::instance();

        if (!g.first)
            ImGui_Impl_WndProcHandler(hWnd, uMsg, wParam, lParam);

        auto& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        {
            // Dont pass keyboard/mouse input to the game
            //return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

        return reinterpret_cast<WNDPRC>(g.original_window_proc)(hWnd, uMsg, wParam, lParam);
    }

    typedef BOOL(__stdcall *wglSwapBuffersFn)(HDC);
    wglSwapBuffersFn owglSwapBuffers;
    BOOL __stdcall hwglSwapBuffers(HDC hDc)
    {
        static auto& g = globals::instance();

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

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            auto& style = ImGui::GetStyle();

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsClassic();

            // Setup Platform/Renderer bindings
            ImGui_Impl_Init(g.main_window);
        }
        
        ImGui_Impl_NewFrame();
        ImGui::NewFrame();

        
        ImGui::Begin("Test Settings");
        {
            ImGui::Checkbox("Bhop", &g.bhop_enabled);
            ImGui::Checkbox("View angle manipulation", &g.backtrack_enabled);
            ImGui::InputFloat3("Aim angles test: ", g.angles2);
            ImGui::Text("(original) Pitch: %f, Yaw: %f, Roll: %f\n", g.angles.x, g.angles.y, g.angles.z);
            ImGui::Text("(new) Pitch: %f, Yaw: %f, Roll: %f\n", g.angles2.x, g.angles2.y, g.angles2.z);
            ImGui::Text("(original) Forward: %f, Side: %f, Up: %f\n", g.move.x, g.move.y, g.move.z);
            ImGui::Text("(new) Forward: %f, Side: %f, Up: %f\n", g.move2.x, g.move2.y, g.move2.z);

            ImGui::Checkbox("Triggerbot", &g.trigger_enabled);
            ImGui::Checkbox("Triggerbot teammates", &g.trigger_team);
            ImGui::InputInt("Triggerbot bone", &g.trigger_bone);
            ImGui::SliderFloat("Backtrack time", &g.backtrack_time, 0, 1000);

            if (g.studio_model_renderer_hook)
            {
                auto time = g.studio_model_renderer_hook->GetBase<CStudioModelRenderer*>()->m_clTime;
                ImGui::Text("Time is: %f\n", time);
            }

            ImGui::SliderInt("Render mode: ", &g.render_mode, 0, render_modes::kRenderCount);
            ImGui::SliderInt("Render effects: ", &g.render_fx, 0, render_effects::kRenderFxCount);
            //ImGui::SliderInt("Studio nf", &g.s_nf, 0, )
            ImGui::InputInt("S_NF", &g.s_nf);
            ImGui::SliderInt("FX Amount: ", &g.fx_amt, 0, 255);
            ImGui::ColorPicker3("FX Colors", reinterpret_cast<float*>(&g.clr));

            

            ImGui::InputInt("Trace mode: ", &g.trace_mode);
            ImGui::InputInt("Trace flags: ", &g.trace_flags);
        }
        ImGui::End(); 

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

        return owglSwapBuffers(hDc);
    }

    typedef void(__thiscall* fnStudioRenderModel)(void* ecx);
    void __fastcall hkStudioRenderModel(CStudioModelRenderer* ecx, void* edx)
    {
        static auto& g = globals::instance();
        static auto oFunc = g.studio_model_renderer_hook->GetOVFunc<fnStudioRenderModel>(18);
        auto entity = g.engine_studio->GetCurrentEntity();
        auto local = g.engine_funcs->GetLocalPlayer();

        //g.engine_funcs->Con_Printf("Studio called from player %i\n", entity->index);

        g.engine_studio->SetChromeOrigin();
        g.engine_studio->SetForceFaceFlags( 0 );

        if ( entity->curstate.renderfx == render_effects::kRenderFxGlowShell )
        {
            entity->curstate.renderfx = kRenderFxNone;
            ecx->StudioRenderFinal( );
            
            if ( !g.engine_studio->IsHardware() )
            {
                g.engine_funcs->pTriAPI->RenderMode( kRenderTransAdd );
            }

            g.engine_studio->SetForceFaceFlags( STUDIO_NF_CHROME );

            g.engine_funcs->pTriAPI->SpriteTexture( g.engine_studio->GetChromeSprite(), 0 );
            entity->curstate.renderfx = kRenderFxGlowShell;

            ecx->StudioRenderFinal( );
            if ( !g.engine_studio->IsHardware() )
            {
                g.engine_funcs->pTriAPI->RenderMode( kRenderNormal );
            }
        }
        else
        {
            if (entity && entity->model && entity->player && entity != local)
            {
                //glDepthFunc(GL_ALWAYS);
                //glDisable(GL_DEPTH_TEST);
                
                

                glDepthRange(0, 0.1);
                glEnable(GL_TEXTURE_2D);
                ecx->StudioRenderFinal();


                glDisable(GL_TEXTURE_2D);
                g.engine_studio->SetForceFaceFlags( g.s_nf );
                g.engine_funcs->pTriAPI->RenderMode( g.render_mode );
                entity->curstate.renderfx = g.render_fx;
                entity->curstate.renderamt = g.fx_amt;
                entity->curstate.rendercolor.r = g.clr.x;
                entity->curstate.rendercolor.g = g.clr.y;
                entity->curstate.rendercolor.b = g.clr.z;
                glColor4f(g.clr.x, g.clr.y, g.clr.z, (float)g.fx_amt / 255.0);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                ecx->StudioRenderFinal();

                glDisable(GL_BLEND);
                glDisable(GL_DEPTH_TEST);

                glPointSize(2.f);
                //for (auto& [key, bbox] : g.player_data[entity->index].hitboxes)
                auto key = g.trigger_bone;
                auto& bbox = g.player_data[entity->index].hitboxes[g.trigger_bone];
                {
                    //g.engine_funcs->Con_Printf("Rendering hitbox %i from player %i\n", key, entity->index);

                    auto transform_min = bbox.matrix.transform_vec3(bbox.box.bbmin);
                    auto transform_max = bbox.matrix.transform_vec3(bbox.box.bbmax);
                    auto center = (transform_min + transform_max) * 0.5;
                    //auto transform = bbox.matrix.transform_vec3(center);
                    if (g.trigger_bone == key)
                    {
                        glColor3f(1.0, 0.0, 0.0);
                    }
                    else
                    {
                        glColor3f(1.0, 1.0, 1.0);
                    }
                    //glBegin(GL_POINTS);
                    //    glVertex3f(center.x, center.y, center.z);
                    //glEnd();

                    glBegin(GL_LINE_LOOP);
                        glVertex3f(transform_min.x, transform_min.y, transform_min.z);
                        glVertex3f(transform_max.x, transform_min.y, transform_min.z);
                        glVertex3f(transform_max.x, transform_max.y, transform_min.z);
                        glVertex3f(transform_min.x, transform_max.y, transform_min.z);
                    glEnd();

                    glBegin(GL_LINE_LOOP);
                        glVertex3f(transform_min.x, transform_min.y, transform_max.z);
                        glVertex3f(transform_max.x, transform_min.y, transform_max.z);
                        glVertex3f(transform_max.x, transform_max.y, transform_max.z);
                        glVertex3f(transform_min.x, transform_max.y, transform_max.z);
                    glEnd();

                    glBegin(GL_LINES);
                        glVertex3f(transform_min.x, transform_min.y, transform_min.z);
                        glVertex3f(transform_min.x, transform_min.y, transform_max.z);
                        glVertex3f(transform_max.x, transform_min.y, transform_min.z);
                        glVertex3f(transform_max.x, transform_min.y, transform_max.z);
                        glVertex3f(transform_max.x, transform_max.y, transform_min.z);
                        glVertex3f(transform_max.x, transform_max.y, transform_max.z);
                        glVertex3f(transform_min.x, transform_max.y, transform_min.z);
                        glVertex3f(transform_min.x, transform_max.y, transform_max.z);
                    glEnd();
                }

                glEnable(GL_DEPTH_TEST);
                glEnable(GL_TEXTURE_2D);
                glDepthRange(0.1, 1);
            }
            else
            {
                ecx->StudioRenderFinal( );
            }
        }
    }

	uint32_t FindClientFuncs()
	{
		DWORD dwExportPointer = Memory::FindPattern("hw.dll", { 0x68, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x83, 0xC4, 0x0C, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00 }, 1, false);
		return dwExportPointer;
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
                    g.player_data[entity->index].hitboxes[i] = {bone, box, transform};

                    //Vector vMin, vMax;
                    //VectorTransform( m_pStudioTransform[11].bbmin, (*m_pBoneTransform)[m_pStudioTransform[11].bone], vMin );
                    //VectorTransform( m_pStudioTransform[11].bbmax, (*m_pBoneTransform)[m_pStudioTransform[11].bone], vMax );
                    //g_Player[ pEnt->index ].vHitbox=(vMin+vMax)*0.5f;

                    //g_Player[ pEnt->index ].bHitbox=true;
                }
            }
        }
    }

    typedef void(__cdecl*fnStudioEntityLight)(alight_s* plight);
    void hkStudioEntityLight(alight_s* plight)
    {
        static auto& g = globals::instance();
        static auto oFunc = reinterpret_cast<fnStudioEntityLight>(g.original_studio_entity_light);
        setup_hitboxes();

        
        oFunc(plight);
    }

	void CL_CreateMove(float frametime, usercmd_t *cmd, int active)
	{
        static auto& g = globals::instance();
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

        if (g.backtrack_enabled)
        {
            auto view = cmd->viewangles;
            auto new_view = g.angles2;
            auto move = vec3_t{cmd->forwardmove, cmd->sidemove, cmd->upmove};
            cmd->viewangles = new_view;

            auto new_move = math::correct_movement(view, new_view, move);
            
            // Reset movement bits
            cmd->buttons &= ~IN_FORWARD;
            cmd->buttons &= ~IN_BACK;
            cmd->buttons &= ~IN_LEFT;
            cmd->buttons &= ~IN_RIGHT;

            cmd->forwardmove = new_move.x;
            cmd->sidemove = new_move.y;
            cmd->upmove = new_move.z;

            g.move2 = new_move;
            g.move = move;//new_move;
            g.angles = view;

            g.angles2.y += g.backtrack_time;

            while (g.angles2.y >= 360)
            {
                g.angles2.y -= 360;
            }

            while (g.angles2.y < 0)
            {
                g.angles2.y += 360;
            }
        }

        // Triggerbot testing
        if (g.trigger_enabled)
        {
            vec3_t start = g.player_move->origin + g.player_move->view_ofs;
            vec3_t angles = cmd->viewangles;
            vec3_t forward = {};
            vec3_t end = {};


            g.engine_funcs->pfnAngleVectors(angles, forward, nullptr, nullptr);
            end = start.multiply_add(2148, forward);


            //auto trace = *g.engine_funcs->PM_TraceLine(start, end, PM_TRACELINE_PHYSENTSONLY, 0, lp->index);
            pmtrace_t trace = {};

            g.engine_funcs->pEventAPI->EV_SetTraceHull(1);
            g.engine_funcs->pEventAPI->EV_PlayerTrace(start, end, PM_GLASS_IGNORE, -1, &trace);
            
            if (trace.fraction != 1.0f)
            {
                // We have a hit
                
                if (trace.ent > 0 && trace.ent <= g.player_move->numphysent)
                {
                    // Get the actual player
                    // trace->ent is the physical entity number
                    // We need to get the normal one
                    // From playermove?
                    auto target = g.engine_funcs->GetEntityByIndex(g.player_move->physents[trace.ent].info);
                    
                    auto& box = g.player_data[target->index].hitboxes[g.trigger_bone];
                    if (auto trace2 = math::ray_hits_rbbox(start, forward, box.box, box.matrix); trace2.hit)
                    {
                        // We hit with a second, more precise trace
                        if (target->player && g.trigger_enabled && ((target->curstate.team != lp->curstate.team) || g.trigger_team))
                        {
                            cmd->buttons |= IN_ATTACK;
                        }
                    }

                    // If we have a valid target
                    /*if (target->player && g.trigger_enabled && ((target->curstate.team != lp->curstate.team) || g.trigger_team))
                    {
                        g.engine_funcs->Con_Printf("Entity is also a valid player, firing at hitgroup: %i\n", trace.hitgroup);
                        // Fire
                        cmd->buttons |= IN_ATTACK;
                    }*/
                }
            }
        }
	}

	void HUD_ClientMove(playermove_t* ppmove, int server)
	{
        static auto& g = globals::instance();

		g.original_client_funcs->pClientMove(ppmove, server);
		std::memcpy(g.player_move, ppmove, sizeof(playermove_t));
	}

	void Init()
	{
        static auto& g = globals::instance();
		auto client = GetModuleHandle(L"client.dll");

		auto cInitialize = reinterpret_cast<uint8_t*>(GetProcAddress(client, "Initialize"));
		g.engine_funcs = reinterpret_cast<cl_enginefunc_t*>(*(uint32_t*)(cInitialize + 0x1C));

		g.player_move = new playermove_t();

		cldll_func_t* funcs = reinterpret_cast<cldll_func_t*>(FindClientFuncs());

		g.original_client_funcs = new cldll_func_t();
		std::memcpy(g.original_client_funcs, funcs, sizeof(cldll_func_t));

		funcs->pCL_CreateMove = CL_CreateMove;
		funcs->pClientMove = HUD_ClientMove;

        uintptr_t wglSwapBuffersLoc = reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers"));
        owglSwapBuffers = reinterpret_cast<wglSwapBuffersFn>(Memory::HookFunc2(wglSwapBuffersLoc, reinterpret_cast<uintptr_t>(hwglSwapBuffers), 5));
        g.original_window_proc = SetWindowLongPtr(g.main_window, GWL_WNDPROC, (LONG_PTR)&hWndProc);

        /*uintptr_t glBeginLoc = reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle(L"opengl32.dll"), "glBegin"));
        oglBegin = reinterpret_cast<glBeginFn>(Memory::HookFunc2(glBeginLoc, reinterpret_cast<uintptr_t>(hglBegin), 5));

        uintptr_t glEndLoc = reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle(L"opengl32.dll"), "glEnd"));
        oglEnd = reinterpret_cast<glEndFn>(Memory::HookFunc2(glEndLoc, reinterpret_cast<uintptr_t>(hglEnd), 5));*/

        uint32_t offset = reinterpret_cast<uint32_t>(Memory::GetModuleInfo("client.dll").lpBaseOfDll);
        auto HUD_GetStudioModelInterface = (uint32_t)Hooks::GetClientFuncs()->pStudioInterface;//reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle(L"client.dll"), "HUD_GetStudioModelInterface"));

        g.engine_studio = *reinterpret_cast<engine_studio_api_s**>(HUD_GetStudioModelInterface + 0x1A);
        auto studio_model_renderer = *reinterpret_cast<CStudioModelRenderer**>(HUD_GetStudioModelInterface + 0x27);

        g.studio_model_renderer_hook = new Memory::VMTHook(studio_model_renderer);
        g.studio_model_renderer_hook->HookVFunc(reinterpret_cast<void*>(hkStudioRenderModel), 18);
        g.studio_model_renderer_hook->Hook();

        g.original_studio_entity_light = reinterpret_cast<uintptr_t>(g.engine_studio->StudioEntityLight);
        g.engine_studio->StudioEntityLight = hkStudioEntityLight;
	}

    void Print(const char* text)
    {
        globals::instance().engine_funcs->pfnConsolePrint(text);
    }

    cl_enginefunc_t* GetEngineFuncs()
    {
        return globals::instance().engine_funcs;
    }

    cldll_func_t* GetClientFuncs()
    {
        return globals::instance().original_client_funcs;
    }
}

