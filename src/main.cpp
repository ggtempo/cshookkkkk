#include <iostream>
#include <memory>
#include <optional>
#include <cstdint>
#include <sstream>
#include <unordered_set>

#include <Windows.h>
#include <gl/GL.h>

#include <locale>
#include <codecvt>

#include "HLSDK/Studio.hpp"

#include "Memory/memory.hpp"
#include "Hooks/hooks.hpp"
#include "Utils/utils.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl.h"

#include "Utils/globals.hpp"

utils::module_info* module_info;

void thread_main()
{
    auto& g = globals::instance();

    g.module = module_info->instance;
    auto parent_process = GetModuleHandle(NULL);
    g.main_window = utils::get_main_window(module_info->pid);
    auto base_path = utils::get_base_path(module_info->instance);
    auto full_path = ((base_path) ? *base_path : std::wstring(L"%USERPROFILE%"));
    std::string base_path_str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(full_path);
    // Either the base path or Home path
    g.base_path = base_path_str;

    hooks::init();

    std::unique_lock<std::mutex> l(g.signal_mutex);
    g.exit_signal.wait(l, [](){return globals::instance().should_quit;});

    // Unload the dll itself
    hooks::unload();
    FreeLibraryAndExitThread(g.module, 0);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(instance);

        /*if (auto base_path = utils::get_base_path(instance); base_path)
        {
            SetDllDirectory((*base_path).c_str());
        }*/

        auto pid = GetCurrentProcessId();

        module_info = new utils::module_info();
        module_info->reason = reason;
        module_info->instance = instance;
        module_info->pid = pid;


        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(&thread_main), nullptr, NULL, NULL);

        return true;
    }
    else
    {
        return true;
    }
}