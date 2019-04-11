#include <cstring>
#include <Windows.h>
#include "hooks.hpp"
#include <glad/gl.h>

#include "../Utils/utils.hpp"
#include "../Utils/globals.hpp"
#include "../Utils/math.hpp"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl.h"

#include "../HLSDK/Parsemsg.hpp"

#include "../Features/AntiAim/antiaim.hpp"
#include "../Features/Triggerbot/triggerbot.hpp"
#include "../Features/Visuals/visuals.hpp"
#include "../Features/Aimbot/aimbot.hpp"
#include "../Features/Removals/removals.hpp"
#include "../HLSDK/Weapons.hpp"
#include "../HLSDK/Textures.hpp"
#include "../Features/Utils/utils.hpp"
#include "../Features/Config/config.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../Resources/stb_image.h"
#include "../Resources/gear.hpp"

#include "studio.hpp"
#include "messages.hpp"
#include "commands.hpp"
#include "netchan.hpp"

#include <iostream>

namespace hooks
{
    
    LRESULT CALLBACK hk_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        using window_proc_fn = LRESULT(__stdcall*)(HWND, UINT, WPARAM, LPARAM);

        static auto& g = globals::instance();

        if (!g.first)
            ImGui_Impl_WndProcHandler(hwnd, msg, wparam, lparam);

        // Key released
        if (msg == WM_KEYUP)
        {
            if (wparam == VK_INSERT)
            {
                // Toggle menu
                g.menu_enabled = !g.menu_enabled;
            }

            if (g.catch_keys)
            {
                g.captured_key = wparam;
            }                
        }
        else if (msg == WM_LBUTTONUP && g.catch_keys)
        {
            g.captured_key = MK_LBUTTON;
        }
        else if (msg == WM_RBUTTONUP && g.catch_keys)
        {
            g.captured_key = MK_RBUTTON;
        }
        else if (msg == WM_MBUTTONUP && g.catch_keys)
        {
            g.captured_key = MK_MBUTTON;
        }
        else if (msg == WM_XBUTTONUP && g.catch_keys)
        {
            g.captured_key = GET_XBUTTON_WPARAM(wparam) + 0x4;
        }

        auto& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard || ImGui::IsMouseHoveringAnyWindow())
        {
            // Dont pass keyboard/mouse input to the game
            //return DefWindowProc(hwnd, msg, wparam, lparam);

            io.MouseDrawCursor = true;
        }
        else
        {
            io.MouseDrawCursor = false;
        }

        return reinterpret_cast<window_proc_fn>(g.original_window_proc)(hwnd, msg, wparam, lparam);
    }

    BOOL __stdcall hk_wgl_swap_buffers(HDC hDc)
    {
        using wgl_swap_buffers_fn = BOOL(__stdcall*)(HDC);

        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<wgl_swap_buffers_fn>(g.original_wgl_swap_buffers);

        if (!g.first)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);check_gl_error();
            glDrawBuffer(GL_BACK);check_gl_error();
        }

        if (g.first)
        {
            g.first = false;

            // Load GL
            ImGui_Impl_LoadGL();check_gl_error();

            // Debug OpenGL Stuff
            auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));check_gl_error();
            auto extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));check_gl_error();

            IMGUI_CHECKVERSION();check_gl_error();
            ImGui::CreateContext();check_gl_error();

            ImGuiIO& io = ImGui::GetIO();check_gl_error();
            auto& style = ImGui::GetStyle();check_gl_error();
            static std::string imgui_config = g.base_path + "CSHook.ini";
            io.IniFilename = imgui_config.c_str();

            io.MouseDrawCursor = false;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();check_gl_error();
            //ImGui::StyleColorsClassic();

            // Setup Platform/Renderer bindings
            ImGui_Impl_Init(g.main_window);check_gl_error();

            // Gear icon load
            int icon_width = 0, icon_height = 0;
            int n = 0;
            auto image = stbi_load_from_memory(gear_icon, gear_icon_length, &icon_width, &icon_height, &n, 0);

            glGenTextures(1, &g.gear_icon_id);check_gl_error();
            glBindTexture(GL_TEXTURE_2D, g.gear_icon_id);check_gl_error();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);check_gl_error();	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);check_gl_error();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);check_gl_error();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);check_gl_error();

            int32_t mode = GL_RGB;                                                                 // The surface mode
            if(n == 4) {
                mode = GL_RGBA;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, mode, icon_width, icon_height, 0, mode, GL_UNSIGNED_BYTE, image);check_gl_error();
            glGenerateMipmap(GL_TEXTURE_2D);check_gl_error();                                       // Not sure if generating mipmaps for a 2D game is necessary

            stbi_image_free(image);

            // Mirrorcam framebuffer / texture
            // Generate necessary buffer
            glGenFramebuffers(1, &g.mirrorcam_buffer);check_gl_error();
            glBindFramebuffer(GL_FRAMEBUFFER, g.mirrorcam_buffer);check_gl_error();

            // Generate texture
            glGenTextures(1, &g.mirrorcam_texture);check_gl_error();
            glBindTexture(GL_TEXTURE_2D, g.mirrorcam_texture);check_gl_error();

            // Texture settings
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 960, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);check_gl_error();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);check_gl_error();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);check_gl_error();

            // Generate depth buffer
            glGenRenderbuffers(1, &g.mirrorcam_depth_buffer);check_gl_error();
            glBindRenderbuffer(GL_RENDERBUFFER, g.mirrorcam_depth_buffer);check_gl_error();

            // Depth buffer settings
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 960);check_gl_error();

            // Attach texture and depth buffer to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g.mirrorcam_texture, 0);check_gl_error();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g.mirrorcam_depth_buffer);check_gl_error();

            // Check if complete
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                MessageBox(NULL, L"Frame buffer error!", L"", MB_OK);
                std::exit(0);
            }

            // Unbind buffer
            glBindTexture(GL_TEXTURE_2D, 0);check_gl_error();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);check_gl_error();

        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);check_gl_error();
        
        glViewport(0, 0, viewport[2], viewport[3]);check_gl_error();

        glMatrixMode(GL_PROJECTION);check_gl_error();
        glPushMatrix();check_gl_error();
        glLoadIdentity();check_gl_error();

        // set the limits of our ortho projection
        glOrtho(0, viewport[2], viewport[3], 0, -1, 100);check_gl_error();

        glMatrixMode(GL_MODELVIEW);check_gl_error();
        glPushMatrix();check_gl_error();
        glLoadIdentity();check_gl_error();

        glMatrixMode(GL_TEXTURE);check_gl_error();
        glPushMatrix();check_gl_error();
        glLoadIdentity();check_gl_error();

        glDisable(GL_CULL_FACE);check_gl_error();
        glDisable(GL_DEPTH_TEST);check_gl_error();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);check_gl_error();
       
        ImGui_Impl_NewFrame();
        ImGui::NewFrame();

        // Render ESP
        features::visuals::instance().swap_buffers();

        if (!g.hide_on_screenshot || !(g.taking_screenshot || g.taking_snapshot))
        {
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

                float right_offset = 10 + 19;

                {
                    auto size = ImGui::CalcTextSize("00:00:00");
                    right_offset += size.x;
                    ImGui::SetCursorPos(ImVec2(title_bar_size.x - right_offset, 0));
                    ImGui::Text("%02d:%02d:%0-2d", time.wHour, time.wMinute, time.wSecond);

                    right_offset += 110;
                }

                {
                    auto size = ImGui::CalcTextSize("CSHook by DJ_Luigi");
                    
                    ImGui::SetCursorPos(ImVec2((title_bar_size.x / 2) - (size.x / 2), 0));
                    ImGui::Text("CSHook by DJ_Luigi");
                }

                ImGui::SetCursorPos(ImVec2(title_bar_size.x - 19.0f, 0.0f));
                {
                    if (ImGui::Button("###Settings", ImVec2(19.0f, 19.0f)))
                    {
                        g.settings_menu_enabled = !g.settings_menu_enabled;
                    }

                    auto draw_list = ImGui::GetWindowDrawList();
                    if (draw_list)
                    {
                        draw_list->AddImage((void*)g.gear_icon_id, ImVec2(title_bar_size.x - 19.0f, 0.0f), ImVec2(title_bar_size.x, 19.0f));
                    }
                    //ImGui::ImageButton((void*)g.gear_icon_id, ImVec2(19.0f, 19.0f));
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
                if (ImGui::Begin("Miscellaneous"))
                {
                    ImGui::Columns(2);
                        ImGui::Checkbox("Bhop enabled", &g.bhop_enabled);
                        ImGui::Checkbox("Mirror cam", &g.mirror_cam_enabled);
                        ImGui::Checkbox("Third person", &g.third_person_enabled);
                        ImGui::Checkbox("Hide on screenshots", &g.hide_on_screenshot);

                        float float_backtrack_time = g.backtrack_amount;
                        ImGui::SliderFloat("Backtrack time", &float_backtrack_time, 0, 99999);
                        g.backtrack_amount = float_backtrack_time;


                    ImGui::NextColumn();
                        features::removals::instance().show_menu();
                    ImGui::Columns(1);
                }
                ImGui::End();
            }

            if (g.mirror_cam_enabled)
            {
                ImGui::Begin("Mirrorcam");
                    auto pos = ImGui::GetCursorScreenPos();
                    auto size = ImGui::GetContentRegionMax();
                    auto bottom_right = ImVec2(pos.x + size.x, pos.y + size.y);
                    ImGui::GetWindowDrawList()->AddImage((void*)g.mirrorcam_texture, pos, bottom_right, ImVec2(0, 1), ImVec2(1, 0));
                ImGui::End();
            }

            if (g.settings_menu_enabled && g.menu_enabled)
            {
                features::config::instance().show_menu();
            }
        }

        ImGui::Render();
        ImGui_Impl_RenderDrawData(ImGui::GetDrawData());

        glEnable(GL_CULL_FACE);check_gl_error();
        glEnable(GL_DEPTH_TEST);check_gl_error();

        glMatrixMode(GL_TEXTURE);check_gl_error();
        glPopMatrix();check_gl_error();
        glMatrixMode(GL_MODELVIEW);check_gl_error();
        glPopMatrix();check_gl_error();
        glMatrixMode(GL_PROJECTION);check_gl_error();
        glPopMatrix();check_gl_error();

        if (g.taking_screenshot)
        {
            g.taking_screenshot = false;
            reinterpret_cast<command_t>(g.original_screenshot)();
        }

        if (g.taking_snapshot)
        {
            g.taking_snapshot = false;
            reinterpret_cast<command_t>(g.original_snapshot)();
        }

        return original_func(hDc);
    }

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

        if (cmd->buttons & IN_JUMP && !(g.player_move->flags & FL_ONGROUND) && g.bhop_enabled)
        {
            cmd->buttons &= ~IN_JUMP;
        }
        
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
        auto info = get_weapon_info((custom::weapon_id)to->client.m_iId);

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

        if (g.third_person_enabled && (!g.hide_on_screenshot || !(g.taking_screenshot || g.taking_snapshot)))
        {
            // We push the view origin by 15 units up
            params->vieworg += (math::vec3(params->right) * 0);
            params->vieworg += (math::vec3(params->up) * 15);

            // Get the backwards vector
            math::vec3 backwards = (math::vec3(params->forward) * - 1);

            // Cast a ray from our player to 100 units behind him
            pmtrace_t trace = {};
            g.engine_funcs->pEventAPI->EV_SetTraceHull(2);
            g.engine_funcs->pEventAPI->EV_PlayerTrace(params->vieworg, params->vieworg + (backwards * 100), PM_WORLD_ONLY, -1, &trace);

            // Scale third person distance (100 units) by lenght of the ray (eg: we hit a wall halfway through => 0.5, we hit nothing => 1.0)
            float distance = trace.fraction * 100;

            // Move the view origin by distance units back
            params->vieworg += backwards * distance;
        }
        
        if (g.mirror_cam_enabled)
        {
            if (params->nextView == 1)
            {
                if(g.local_player_data.alive && g.engine_funcs->GetLocalPlayer()->curstate.solid != SOLID_NOT)
                {
                    // Normal view
                    // The VGUI hud is drawn after all of the views
                    // Because of that, nextView 0 ends up with no HUD
                    // We instead render the view 1 into the main framebuffer
                    // and render view 0 (technically the main view) to the mirror_cam framebuffer
                    // TODO: Hook vgui and switch the framebuffer to the default one
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);check_gl_error();
                    glDrawBuffer(GL_BACK);check_gl_error();

                    params->nextView = 0;
                }
            }   
            else if (params->nextView == 0)
            {
                if (g.local_player_data.alive && g.engine_funcs->GetLocalPlayer()->curstate.solid != SOLID_NOT)
                {
                    // Mirror cam view
                    GLenum buffers[1] = {GL_COLOR_ATTACHMENT0};
                    glBindFramebuffer(GL_FRAMEBUFFER, g.mirrorcam_buffer);check_gl_error();
                    glDrawBuffers(1, buffers);check_gl_error();
                    
                    params->nextView = 1;

                    params->viewport[0] = 0;
                    params->viewport[1] = 0;
                    params->viewangles[1] += 180;
                }
            }
        }
    }

    bool hk_is_third_person()
    {
        static auto& g = globals::instance();
        return g.third_person_enabled && (!g.hide_on_screenshot || !(g.taking_screenshot || g.taking_snapshot));
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

    void hk_client_move_init(playermove_s* pmove)
    {
        static auto& g = globals::instance();
        static auto original_func = g.original_client_funcs->pClientMoveInit;

        PM_InitTextureTypes(pmove);

        original_func(pmove);
    }

    int __fastcall hk_StudioDrawPlayer(CStudioModelRenderer* ecx, void* edx, int flags, entity_state_s* pplayer)
    {
        using studio_draw_player_fn = int(__thiscall*)(CStudioModelRenderer* ecx, int flags, entity_state_s* pplayer);
        static auto& g = globals::instance();
        static auto original_func = g.studio_model_renderer_hook->get_original_vfunc<studio_draw_player_fn>(25);

        g.engine_funcs->Con_Printf("Draw player\n");

        //pplayer->angles.y = 0;

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

    uint32_t find_client_functions()
    {
        DWORD dw_export_pointer = memory::find_pattern("hw.dll", { 0x68, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x83, 0xC4, 0x0C, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00 }, 1, false);
        return dw_export_pointer;
    }

    void save_client()
    {
        static auto& g = globals::instance();

        // Find and save original/hooked client funcs
        g.client_funcs = reinterpret_cast<cldll_func_t*>(find_client_functions());
        g.original_client_funcs = new cldll_func_t();
        std::memcpy(g.original_client_funcs, g.client_funcs, sizeof(cldll_func_t));
    }

    void hook_globals()
    {
        static auto& g = globals::instance();

        // Steal WeaponsPostThink from PostRunCmd
        uintptr_t weapons_post_think_rel = *(uintptr_t*)(g.original_client_funcs->pPostRunCmd + 0x2A);
        uintptr_t weapons_post_think_abs = weapons_post_think_rel + ((uintptr_t)g.original_client_funcs->pPostRunCmd + 0x2E);

        // Steal InitClientWeapons from WeaponsPostThink
        uintptr_t init_client_weapons_rel = *(uintptr_t*)(weapons_post_think_abs + 0x3);
        uintptr_t init_client_weapons_abs = init_client_weapons_rel + ((uintptr_t)weapons_post_think_abs + 0x7);

        // Get global pointers
        g.game_globals = *(globalvars_t**)(init_client_weapons_abs + 0x23);
        g.game_globals_2 = *(globalvars_t***)(weapons_post_think_abs + 0xC);

        // Side effect, save get_weapon_info
        g.get_weapon_info = *(uintptr_t*)(weapons_post_think_abs + 0x1E) + weapons_post_think_abs + 0x22;
    }

    void hook_studio()
    {
        static auto& g = globals::instance();

        // Get StudioModelInterface
        uint32_t offset = reinterpret_cast<uint32_t>(memory::get_module_info("client.dll").lpBaseOfDll);
        auto HUD_GetStudioModelInterface = (uint32_t)hooks::get_client_funcs()->pStudioInterface;

        // Steal EngineStudio from StudioModelInterface
        g.engine_studio = *reinterpret_cast<engine_studio_api_s**>(HUD_GetStudioModelInterface + 0x1A);

        // Hook EngineStudio functions
        g.original_studio_entity_light = reinterpret_cast<uintptr_t>(g.engine_studio->StudioEntityLight);
        g.engine_studio->StudioEntityLight = hk_studio_entity_light;

        g.original_studio_check_bbox = reinterpret_cast<uintptr_t>(g.engine_studio->StudioCheckBBox);
        g.engine_studio->StudioCheckBBox = hk_studio_check_bbox;


        // Steam StudioModelRenderer from StudioModelInterface
        auto studio_model_renderer = *reinterpret_cast<CStudioModelRenderer**>(HUD_GetStudioModelInterface + 0x27);

        // Hook vtable functions
        g.studio_model_renderer_hook = new memory::vmt_hook(studio_model_renderer);
        g.studio_model_renderer_hook->hook_vfunc(reinterpret_cast<void*>(hk_studio_render_model), 18);
        //g.studio_model_renderer_hook->hook_vfunc(reinterpret_cast<void*>(hk_StudioDrawPlayer), 25);
        g.studio_model_renderer_hook->hook();
    }

    void hook_client()
    {
        static auto& g = globals::instance();

        // Hook client funcs
        g.client_funcs->pCL_CreateMove = hk_cl_create_move;
        g.client_funcs->pClientMove = hk_hud_clientmove;
        g.client_funcs->pCalcRefdef = hk_calc_ref_def;
        g.client_funcs->pPostRunCmd = hk_post_run_cmd;
        g.client_funcs->pHudRedrawFunc = hk_hud_redraw;
        g.client_funcs->pClientMoveInit = hk_client_move_init;
        g.client_funcs->pCL_IsThirdPerson = (void*)hk_is_third_person;
    }

    void hook_gl()
    {
        static auto& g = globals::instance();
        auto opengl_dll = GetModuleHandle(L"opengl32.dll");

        // Hook wglSwapBuffers
        uintptr_t wgl_swap_buffers = reinterpret_cast<uintptr_t>(GetProcAddress(opengl_dll, "wglSwapBuffers"));
        // Overwrite first 5 bytes
        g.original_wgl_swap_buffers = reinterpret_cast<uintptr_t>(memory::hook_func2(wgl_swap_buffers, reinterpret_cast<uintptr_t>(hk_wgl_swap_buffers), 5));

        // Hook WindowProc
        g.original_window_proc = SetWindowLongPtr(g.main_window, GWL_WNDPROC, (LONG_PTR)&hk_wnd_proc);
    }

    using message_fn = int(*)(const char *name, int size, void *buffer);
    uintptr_t hook_message(const char* message_name, message_fn new_function)
    {
        static auto& g = globals::instance();

        // Steal RegisterUserMessage from HookUserMessage
        auto hook_usr_msg = (uint32_t)g.engine_funcs->pfnHookUserMsg;
        uint32_t register_usr_msg = *(uint32_t*)(hook_usr_msg + 0x1B) + hook_usr_msg + (0x1F);

        // Find first Message Pointer in RegisterUserMessage
        uint32_t first_usr_msg_ptr = *(uint32_t*)(register_usr_msg + 0xD);
        usermsg_t* first_usr_msg_entry = *reinterpret_cast<usermsg_t**>(first_usr_msg_ptr);

        // Messages are a linked list
        auto element = first_usr_msg_entry;
        while (element)
        {
            if (std::strcmp(element->szMsg, message_name) == 0)
            {
                uintptr_t original_function = (uintptr_t)element->pfn;
                element->pfn = new_function;
                return original_function;
            }

            // Go to the next one
            element = element->pNext;
        }

        return 0;
    }

    uintptr_t hook_command(const char* command_name, command_t new_function)
    {
        static auto& g = globals::instance();
        auto cmd = g.engine_funcs->GetFirstCmdFunctionHandle();

        while (cmd)
        {
            if (std::strcmp(cmd->name, command_name) == 0)
            {
                uintptr_t original_function = reinterpret_cast<uintptr_t>(cmd->function);
                cmd->function = new_function;
                return original_function;
            }

            cmd = cmd->next;
        }

        return 0;
    }

    void pre_write_packet_time()
    {
        // Set host.realtime back to the original value
        static auto& g = globals::instance();

        g.engine_funcs->Con_Printf("Pre write packet time!\n");
    }

    void cl_compute_packet_loss()
    {
        using cl_compute_packet_loss_fn = void(*)(void);
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<cl_compute_packet_loss_fn>(g.original_cl_compute_packet_loss);

        // Call original func
        original_func();

        // Then modify host realtime
        g.engine_time_backup = *g.engine_time;

        if (GetAsyncKeyState(VK_MENU))
        {
            *g.engine_time -= g.backtrack_amount;
        }
    }

    void post_write_packet_time()
    {
        static auto& g = globals::instance();

        // Restore host realtime

        g.engine_funcs->Con_Printf("Engine time: %f, Backup: %f\n", *g.engine_time, g.engine_time_backup);
        *g.engine_time = g.engine_time_backup;
    }

    void hook_writepacket()
    {
        static auto& g = globals::instance();
        g.engine_funcs->Con_Printf("Hooking WritePacket\n");
        // Hook before and after writing packet
        
        auto cl_write_packet_loc = memory::find_location("hw.dll", {0x53, 0x55, 0x56, 0x57, 0x33, 0xff, 0x3b, 0xc7, 0x0f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x83, 0xf8, 0x01}, -11);
        auto cl_compute_packet_loss_loc = memory::find_location("hw.dll", {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8, 0x83, 0xec, 0x0c, 0xdd, 0x05, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x1d, 0x00, 0x00, 0x00, 0x00});
        
        // Host realtime
        //g.host_realtime = *(double**)(reinterpret_cast<uintptr_t>(cl_compute_packet_loss_loc) + 0xB);

        // Before
        // cl_write_packet_loc + 0x7A (6 bytes)
        // cl_compute_packet_loss
        g.original_cl_compute_packet_loss = memory::hook_func2(reinterpret_cast<uintptr_t>(cl_compute_packet_loss_loc), reinterpret_cast<uintptr_t>(cl_compute_packet_loss), 6);

        // After
        // cl_write_packet_loc + 0xE1 (5 bytes)
        memory::hook_func(reinterpret_cast<uintptr_t>(cl_write_packet_loc) + 0xE1, reinterpret_cast<uintptr_t>(post_write_packet_time), 5);

        g.engine_funcs->Con_Printf("Found CL_WritePacket @ 0x%X\n", cl_write_packet_loc);
        g.engine_funcs->Con_Printf("Found CL_ComputePacketLoss @ 0x%X\n", cl_compute_packet_loss_loc);

        //memory::hook_func(reinterpret_cast<uintptr_t>(ců_write_packet_loc), reinterpret_cast<uintptr_t>(post_write_packet), 5);
    }

    void init()
    {
        /*AllocConsole();
        freopen("CONOUT$", "w", stdout);*/

        static auto& g = globals::instance();
        auto client = GetModuleHandle(L"client.dll");
        auto hw_dll = GetModuleHandle(L"hw.dll");

        auto client_initialize = reinterpret_cast<uint8_t*>(GetProcAddress(client, "Initialize"));
        g.engine_funcs = reinterpret_cast<cl_enginefunc_t*>(*(uint32_t*)(client_initialize + 0x1C));

        g.player_move = new playermove_t();

        save_client();
        hook_globals();
        hook_client();
        hook_studio();
        hook_gl();
        hook_writepacket();

        g.engine_time = *(double**)(g.engine_funcs->pNetAPI->Status + 0x84);

        auto can_packet_ptr = memory::find_location("hw.dll",
        {
            0xD9, 0x05, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x1D, 0x00, 0x00, 0x00, 0x00, 0xDF, 0xE0, 0xF6, 0xC4, 0x00, 0x8B, 0x44, 0x24
        });

        g.original_can_packet = memory::hook_func2(reinterpret_cast<uintptr_t>(can_packet_ptr), reinterpret_cast<uintptr_t>(hk_netchan_canpacket), 6);
        g.engine_funcs->Con_Printf("Can_Packet at: 0x%X\n", can_packet_ptr);

        // Hook messages
        g.original_team_info = hook_message("TeamInfo", hk_team_info);
        g.original_score_attrib = hook_message("ScoreAttrib", hk_score_attrib);
        g.original_set_fov = hook_message("SetFOV", hk_set_fov);

        // Hook commands
        g.original_screenshot = hook_command("screenshot", hk_screenshot);
        g.original_snapshot = hook_command("snapshot", hk_snapshot);

        features::config::instance().load_config();
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

