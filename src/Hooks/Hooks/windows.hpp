#pragma once
#include <cstring>
#include <Windows.h>
#include "../hooks.hpp"
#include <glad/gl.h>

#include "../../Utils/utils.hpp"
#include "../../Utils/globals.hpp"
#include "../../Utils/math.hpp"

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
                features::menu::instance().toggle_menu();
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
}