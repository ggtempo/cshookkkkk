#pragma once
#include "../Memory/memory.hpp"
#include "../HLSDK/enginefuncs.hpp"
#include "../HLSDK/clientfuncs.hpp"
#include "../HLSDK/playermove.hpp"
#include "../HLSDK/Studio.hpp"
#include "../HLSDK/StudioStructures.hpp"
#include "../HLSDK/Globals.hpp"
#include "../HLSDK/Weapons.hpp"
#include "../HLSDK/Textures.hpp"
#include "custom.hpp"

#include <iostream>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

class globals
{
    private:
        globals()
        {
            this->studio_model_renderer_hook = nullptr;

            this->connected = false;
            this->send_packet = true;
            this->first = true;

            this->catch_keys = false;
            this->captured_key = -1;

            this->gear_icon_id = 0;
            this->mirrorcam_buffer = 0;
            this->mirrorcam_texture = 0;
            this->mirrorcam_depth_buffer = 0;

            this->fov = 90.0;
            this->backtrack_amount = 0.0;

            this->should_quit = false;
        }

    public:
        static globals& instance()
        {
            static globals instance;
            return instance;
        }

    public:
        // Connected
        bool connected;

        // SendPacket
        bool send_packet;

        // Backtrack amount
        double backtrack_amount;

        // Module base path
        std::string base_path;

        // Hooks
        memory::vmt_hook* studio_model_renderer_hook = nullptr;
        memory::jump_hook* wgl_swap_buffers_hook = nullptr;
        memory::jump_hook* cl_compute_packet_loss_hook = nullptr;
        memory::jump_hook* can_packet_hook = nullptr;
        memory::call_hook* cl_write_packet_hook = nullptr;

        // Pointers
        engine_studio_api_t* engine_studio = 0;
        cl_enginefunc_t*	engine_funcs = 0;
        cldll_func_t*		original_client_funcs = 0;
        cldll_func_t*		client_funcs = 0;
        playermove_t*		player_move = 0;
        usercmd_t*          last_cmd = 0;

        uintptr_t           original_studio_entity_light = 0;
        uintptr_t           original_team_info = 0;
        uintptr_t           original_cur_weapon = 0;
        uintptr_t           original_score_attrib = 0;
        uintptr_t           original_damage = 0;
        uintptr_t           original_studio_check_bbox = 0;
        uintptr_t           original_can_packet = 0;
        uintptr_t           original_set_fov = 0;

        uintptr_t           original_screenshot = 0;
        uintptr_t           original_snapshot = 0;

        uintptr_t           original_cl_compute_packet_loss = 0;

        globalvars_t*       game_globals = 0;
        globalvars_t**      game_globals_2 = 0;

        double*             engine_time = nullptr;
        double              engine_time_backup;
        double*             host_realtime = nullptr;

        uintptr_t           get_weapon_info = 0;

        uintptr_t           original_window_proc = 0;
        uintptr_t           original_wgl_swap_buffers = 0;
        HWND                main_window;

        // Helpers and useful data
        bool first;
        custom::player_data local_player_data;
        std::unordered_map<int, custom::player_data> player_data;
        math::vec3 punch_angles;
        float fov;

        // Key catching
        bool catch_keys;
        int captured_key;

        // Opengl
        unsigned int gear_icon_id;
        unsigned int mirrorcam_buffer;
        unsigned int mirrorcam_texture;
        unsigned int mirrorcam_depth_buffer;

        // Windows
        HMODULE module;

        // Exit variable
        std::mutex signal_mutex;
        std::condition_variable exit_signal;
        bool should_quit;
};