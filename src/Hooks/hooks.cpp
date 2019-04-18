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
#include "../Features/Menu/menu.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../Resources/stb_image.h"
#include "../Resources/gear.hpp"

#include "Hooks/studio.hpp"
#include "Hooks/messages.hpp"
#include "Hooks/commands.hpp"
#include "Hooks/netchan.hpp"
#include "Hooks/client.hpp"
#include "Hooks/opengl.hpp"
#include "Hooks/windows.hpp"

#include <iostream>

namespace hooks
{
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
        auto HUD_GetStudioModelInterface = reinterpret_cast<uintptr_t>(g.original_client_funcs->pStudioInterface);

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
    
        g.wgl_swap_buffers_hook = new memory::jump_hook(wgl_swap_buffers, reinterpret_cast<uintptr_t>(hk_wgl_swap_buffers), 5);
        g.original_wgl_swap_buffers = g.wgl_swap_buffers_hook->get_original_function();

        // Hook WindowProc
        g.original_window_proc = SetWindowLongPtr(g.main_window, GWL_WNDPROC, (LONG_PTR)&hk_wnd_proc);
    }

    typedef int(*message_fn)(const char *name, int size, void *buffer);
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
        *g.engine_time = g.engine_time_backup;
    }

    void hook_writepacket()
    {
        static auto& g = globals::instance();
        // Hook before and after writing packet
        
        auto cl_write_packet_loc = memory::find_location("hw.dll", {0x53, 0x55, 0x56, 0x57, 0x33, 0xff, 0x3b, 0xc7, 0x0f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x83, 0xf8, 0x01}, -11);
        auto cl_compute_packet_loss_loc = memory::find_location("hw.dll", {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8, 0x83, 0xec, 0x0c, 0xdd, 0x05, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x1d, 0x00, 0x00, 0x00, 0x00});
        
        // Host realtime
        //g.host_realtime = *(double**)(reinterpret_cast<uintptr_t>(cl_compute_packet_loss_loc) + 0xB);

        // Before
        // cl_write_packet_loc + 0x7A (6 bytes)
        // cl_compute_packet_loss
        g.cl_compute_packet_loss_hook = new memory::jump_hook(reinterpret_cast<uintptr_t>(cl_compute_packet_loss_loc), reinterpret_cast<uintptr_t>(cl_compute_packet_loss), 6);
        g.original_cl_compute_packet_loss = g.cl_compute_packet_loss_hook->get_original_function();

        // After
        // cl_write_packet_loc + 0xE1 (5 bytes)

        g.cl_write_packet_hook = new memory::call_hook(reinterpret_cast<uintptr_t>(cl_write_packet_loc) + 0xE1, reinterpret_cast<uintptr_t>(post_write_packet_time), 5);
        //memory::hook_func(reinterpret_cast<uintptr_t>(cl_write_packet_loc), reinterpret_cast<uintptr_t>(post_write_packet), 5);
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

        g.can_packet_hook = new memory::jump_hook(reinterpret_cast<uintptr_t>(can_packet_ptr), reinterpret_cast<uintptr_t>(hk_netchan_canpacket), 6);
        g.original_can_packet = g.can_packet_hook->get_original_function();

        // Hook messages
        g.original_team_info = hook_message("TeamInfo", hk_team_info);
        g.original_score_attrib = hook_message("ScoreAttrib", hk_score_attrib);
        g.original_set_fov = hook_message("SetFOV", hk_set_fov);

        // Hook commands
        g.original_screenshot = hook_command("screenshot", hk_screenshot);
        g.original_snapshot = hook_command("snapshot", hk_snapshot);

        features::config::instance().load_config();
    }

    // NOTE:    Can't be called from the game thread itself
    //          Should probably be called when a flag is set (from an non-game thread)
    void unload()
    {
        auto& g = globals::instance();

        // Restore client functions
        std::memcpy(g.client_funcs, g.original_client_funcs, sizeof(cldll_func_t));
        delete g.original_client_funcs;

        // Restore windows
        SetWindowLongPtr(g.main_window, GWL_WNDPROC, (LONG_PTR)g.original_window_proc);

        // Restore studio
        g.engine_studio->StudioEntityLight = reinterpret_cast<void(*)(alight_s*)>(g.original_studio_entity_light);
        g.engine_studio->StudioCheckBBox = reinterpret_cast<int(*)()>(g.original_studio_check_bbox);

        // Restore StudioModelRenderer
        delete g.studio_model_renderer_hook;

        // Restore various hooks
        delete g.wgl_swap_buffers_hook;
        delete g.cl_compute_packet_loss_hook;
        delete g.can_packet_hook;
        delete g.cl_write_packet_hook;

        // Unhook messages
        hook_message("TeamInfo", reinterpret_cast<message_fn>(g.original_team_info));
        hook_message("ScoreAttrib", reinterpret_cast<message_fn>(g.original_score_attrib));
        hook_message("SetFOV", reinterpret_cast<message_fn>(g.original_set_fov));

        // Unhook commands
        hook_command("screenshot", reinterpret_cast<command_t>(g.original_screenshot));
        hook_command("snapshot", reinterpret_cast<command_t>(g.original_snapshot));
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

