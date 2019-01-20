#include <cstring>
#include <Windows.h>
#include "hooks.hpp"
#include <gl/GL.h>

#include "../Utils/globals.hpp"
#include "../Utils/math.hpp"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl.h"

#include <iostream>

namespace hooks
{
    typedef LRESULT(__stdcall*WNDPRC)(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

        
        /*ImGui::Begin("Test Settings");
        {
            ImGui::Checkbox("Bhop", &g.bhop_enabled);
            ImGui::Checkbox("Anti aim", &g.anti_aim_enabled);

            ImGui::Checkbox("Triggerbot", &g.trigger_enabled);
            ImGui::Checkbox("Triggerbot teammates", &g.trigger_team);
            ImGui::InputInt("Triggerbot bone", &g.trigger_bone);

            ImGui::SliderInt("Render mode: ", &g.render_mode, 0, render_modes::kRenderCount);
            ImGui::SliderInt("Render effects: ", &g.render_fx, 0, render_effects::kRenderFxCount);
            //ImGui::SliderInt("Studio nf", &g.s_nf, 0, )
            ImGui::InputInt("S_NF", &g.s_nf);
            ImGui::SliderInt("FX Amount: ", &g.fx_amt, 0, 255);
            ImGui::ColorPicker3("FX Colors", reinterpret_cast<float*>(&g.clr));

            

            ImGui::InputInt("Trace mode: ", &g.trace_mode);
            ImGui::InputInt("Trace flags: ", &g.trace_flags);
        }
        ImGui::End();*/

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
        

        if (g.aimbot_menu_enabled)
        {
            ImGui::Begin("Aimbot");
            ImGui::End();
        }

        if (g.trigger_menu_enabled)
        {
            static bool complex_hitboxes = false;

            ImGui::Begin("Triggerbot");
                ImGui::Checkbox("Triggerbot enabled", &g.trigger_enabled);
                ImGui::Checkbox("Shoot team", &g.trigger_team);
                ImGui::Checkbox("Show complex hitboxes", &complex_hitboxes);

                if (complex_hitboxes)
                {
                    g.trigger_hitboxes_all = false;
                    ImGui::ListBoxHeader("Hitboxes");
                    {
                        static auto& pelvis = g.trigger_hitboxes[hitbox_numbers::pelvis];
                        static auto& left_top_leg = g.trigger_hitboxes[hitbox_numbers::left_top_leg];
                        static auto& left_bottom_leg = g.trigger_hitboxes[hitbox_numbers::left_bottom_leg];
                        static auto& left_foot = g.trigger_hitboxes[hitbox_numbers::left_foot];
                        static auto& right_top_leg = g.trigger_hitboxes[hitbox_numbers::right_top_leg];
                        static auto& right_bottom_leg = g.trigger_hitboxes[hitbox_numbers::right_bottom_leg];
                        static auto& right_foot = g.trigger_hitboxes[hitbox_numbers::right_foot];
                        static auto& torso_bottom = g.trigger_hitboxes[hitbox_numbers::torso_bottom];
                        static auto& torso_top = g.trigger_hitboxes[hitbox_numbers::torso_top];
                        static auto& neck = g.trigger_hitboxes[hitbox_numbers::neck];
                        static auto& heart = g.trigger_hitboxes[hitbox_numbers::heart];
                        static auto& head = g.trigger_hitboxes[hitbox_numbers::head];
                        static auto& left_shoulder = g.trigger_hitboxes[hitbox_numbers::left_shoulder];
                        static auto& left_arm_top = g.trigger_hitboxes[hitbox_numbers::left_arm_top];
                        static auto& left_arm_bottom = g.trigger_hitboxes[hitbox_numbers::left_arm_bottom];
                        static auto& left_hand = g.trigger_hitboxes[hitbox_numbers::left_hand];
                        static auto& right_shoulder = g.trigger_hitboxes[hitbox_numbers::right_shoulder];
                        static auto& right_arm_top = g.trigger_hitboxes[hitbox_numbers::right_arm_top];
                        static auto& right_arm_bottom = g.trigger_hitboxes[hitbox_numbers::right_arm_bottom];
                        static auto& right_hand = g.trigger_hitboxes[hitbox_numbers::right_hand];

                        ImGui::Selectable("Head", &head);
                        ImGui::Selectable("Neck", &neck);
                        ImGui::Selectable("Torso top", &torso_top);
                        ImGui::Selectable("Torso bottom", &torso_bottom);
                        ImGui::Selectable("Pelvis", &pelvis);
                        ImGui::Selectable("Heart", &heart);
                        ImGui::Selectable("Left top leg", &left_top_leg);
                        ImGui::Selectable("Left bottom leg", &left_bottom_leg);
                        ImGui::Selectable("Left foot", &left_foot);
                        ImGui::Selectable("Right top leg", &right_top_leg);
                        ImGui::Selectable("Right bottom leg", &right_bottom_leg);
                        ImGui::Selectable("Right foot", &right_foot );
                        ImGui::Selectable("Left shoulder", &left_shoulder);
                        ImGui::Selectable("Left arm top", &left_arm_top);
                        ImGui::Selectable("Left arm bottom", &left_arm_bottom);
                        ImGui::Selectable("Left hand", &left_hand);
                        ImGui::Selectable("Right shoulder", &right_shoulder);
                        ImGui::Selectable("Right arm top", &right_arm_top);
                        ImGui::Selectable("Right arm bottom", &right_arm_bottom);
                        ImGui::Selectable("Right hand", &right_hand);
                    }
                    ImGui::ListBoxFooter();
                    ImGui::SameLine();
                    if (ImGui::Button("Select / Deselect all"))
                    {
                        bool new_state = !g.trigger_hitboxes[0];
                        for (auto& [key, value] : g.trigger_hitboxes)
                        {
                            value = new_state;
                        }
                    }
                }
                else
                {
                    g.trigger_hitboxes_all = true;
                }
                

                ImGui::InputInt("Trigger bone debug", &g.trigger_bone);
            ImGui::End();
        }

        if (g.esp_menu_enabled)
        {
            ImGui::Begin("ESP");
            ImGui::End();
        }

        if (g.anti_aim_enabled)
        {
            ImGui::Begin("Anti-Aim");
            ImGui::End();
        }

        if (g.misc_menu_enabled)
        {
            ImGui::Begin("Miscellaneous");
                ImGui::Checkbox("Bhop enabled", &g.bhop_enabled);
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

        return owglSwapBuffers(hDc);
    }

    typedef void(__thiscall* fnStudioRenderModel)(void* ecx);
    void __fastcall hkStudioRenderModel(CStudioModelRenderer* ecx, void* edx)
    {
        static auto& g = globals::instance();
        static auto oFunc = g.studio_model_renderer_hook->get_original_vfunc<fnStudioRenderModel>(18);
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

            g.engine_funcs->pEventAPI->EV_SetTraceHull(2);
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

                    bool hit = false;
                    for (auto& [key, value] : g.trigger_hitboxes)
                    {
                        if (value || g.trigger_hitboxes_all)
                        {
                            auto& box = g.player_data[target->index].hitboxes[key];
                            auto trace2 = math::ray_hits_rbbox(start, forward, box.box, box.matrix);

                            if (trace2.hit)
                            {
                                hit = true;
                                break;
                            }
                        }
                    }

                    if (hit)
                    {
                        // We hit with a second, more precise trace
                        if (target->player && g.trigger_enabled && ((target->curstate.team != lp->curstate.team) || g.trigger_team))
                        {
                            cmd->buttons |= IN_ATTACK;
                        }
                    }
                }
            }
        }

        if (g.anti_aim_enabled && !(cmd->buttons & IN_ATTACK))
        {
            static float angle = 0.0f;

            auto view = cmd->viewangles;
            auto new_view = math::vec3{-89.0f, angle, 0.0f};
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

            while (cmd->viewangles.y > 360)
            {
                cmd->viewangles.y -= 360;
                angle -= 360;
            }

            while (cmd->viewangles.y < 0)
            {
                cmd->viewangles.y += 360;
                angle += 360;
            }

            cmd->viewangles.z = 0;

            angle += 50;
        }
	}

	void HUD_ClientMove(playermove_t* ppmove, int server)
	{
        static auto& g = globals::instance();

		g.original_client_funcs->pClientMove(ppmove, server);
		std::memcpy(g.player_move, ppmove, sizeof(playermove_t));
	}

    typedef int(*fnTeamInfo)(const char*, int, void*);
    int hkTeamInfo(const char *pszName, int iSize, void *pbuf)
    {
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<fnTeamInfo>(g.original_team_info);

        g.engine_funcs->Con_Printf("Team Info called with: %s, iSize: %d, pbuf: 0x%X\n", pszName, iSize, pbuf);

        return original_func(pszName, iSize, pbuf);
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

		cldll_func_t* funcs = reinterpret_cast<cldll_func_t*>(find_client_functions());

		g.original_client_funcs = new cldll_func_t();
		std::memcpy(g.original_client_funcs, funcs, sizeof(cldll_func_t));

		funcs->pCL_CreateMove = CL_CreateMove;
		funcs->pClientMove = HUD_ClientMove;

        uintptr_t wglSwapBuffersLoc = reinterpret_cast<uintptr_t>(GetProcAddress(opengl_dll, "wglSwapBuffers"));
        owglSwapBuffers = reinterpret_cast<wglSwapBuffersFn>(memory::HookFunc2(wglSwapBuffersLoc, reinterpret_cast<uintptr_t>(hwglSwapBuffers), 5));
        g.original_window_proc = SetWindowLongPtr(g.main_window, GWL_WNDPROC, (LONG_PTR)&hWndProc);

        uint32_t offset = reinterpret_cast<uint32_t>(memory::get_module_info("client.dll").lpBaseOfDll);
        auto HUD_GetStudioModelInterface = (uint32_t)hooks::get_client_funcs()->pStudioInterface;

        g.engine_studio = *reinterpret_cast<engine_studio_api_s**>(HUD_GetStudioModelInterface + 0x1A);
        auto studio_model_renderer = *reinterpret_cast<CStudioModelRenderer**>(HUD_GetStudioModelInterface + 0x27);

        g.studio_model_renderer_hook = new memory::vmt_hook(studio_model_renderer);
        g.studio_model_renderer_hook->hook_vfunc(reinterpret_cast<void*>(hkStudioRenderModel), 18);
        g.studio_model_renderer_hook->hook();

        g.original_studio_entity_light = reinterpret_cast<uintptr_t>(g.engine_studio->StudioEntityLight);
        g.engine_studio->StudioEntityLight = hkStudioEntityLight;
        //g.original_hook_usr_msg = reinterpret_cast<uintptr_t>(g.engine_funcs->pfnHookUserMsg);
        //g.engine_funcs->pfnHookUserMsg = reinterpret_cast<pfnEngSrc_pfnHookUserMsg_t>(hkHookUserMsg);

        auto hook_usr_msg = (uint32_t)g.engine_funcs->pfnHookUserMsg;
        g.engine_funcs->Con_Printf("Hook: 0x%X\n", hook_usr_msg);
        uint32_t register_usr_msg = *(uint32_t*)(hook_usr_msg + 0x1B) + hook_usr_msg + (0x1F);
        g.engine_funcs->Con_Printf("Register at 0x%X\n", register_usr_msg);
        uint32_t first_usr_msg_ptr = *(uint32_t*)(register_usr_msg + 0xD);
        usermsg_t* first_usr_msg_entry = *reinterpret_cast<usermsg_t**>(first_usr_msg_ptr);

        g.engine_funcs->Con_Printf("Hook: 0x%X, Register: 0x%X, First PTR: 0x%X, First Entry: 0x%X\n", hook_usr_msg, register_usr_msg, first_usr_msg_ptr, first_usr_msg_entry);

        // Messages are a linked list
        auto element = first_usr_msg_entry;
        while (element)
        {
            if (std::strcmp(element->szMsg, "TeamInfo") == 0)
            {
               // We found the right one
               g.engine_funcs->Con_Printf("Found TeamInfo at 0x%X\n", element->pfn);
               g.original_team_info = (uintptr_t)element->pfn;
               element->pfn = hkTeamInfo;
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

