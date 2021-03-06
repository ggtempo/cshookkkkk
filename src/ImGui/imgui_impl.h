#pragma once
#include "imgui.h"
#include <cstdint>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>


#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
//#define check_gl_error()
void _check_gl_error(const char *file, int line);

IMGUI_IMPL_API void     ImGui_Impl_LoadGL();
IMGUI_IMPL_API bool     ImGui_Impl_Init(void* hwnd);
IMGUI_IMPL_API void     ImGui_Impl_Shutdown();
IMGUI_IMPL_API void     ImGui_Impl_NewFrame();
IMGUI_IMPL_API void     ImGui_Impl_RenderDrawData(ImDrawData* draw_data);

// Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool     ImGui_Impl_CreateFontsTexture();
IMGUI_IMPL_API void     ImGui_Impl_DestroyFontsTexture();
IMGUI_IMPL_API bool     ImGui_Impl_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_Impl_DestroyDeviceObjects();

IMGUI_IMPL_API uintptr_t ImGui_Impl_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);