#include <iostream>
#include <memory>
#include <optional>
#include <cstdint>
#include <sstream>
#include <unordered_set>

#include <Windows.h>
#include <gl/GL.h>

#include "HLSDK/Studio.hpp"

#include "Memory/memory.hpp"
#include "Hooks/hooks.hpp"
#include "Utils/utils.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl.h"

#include "Utils/globals.hpp"

utils::module_info* module_info;

void ThreadMain()
{
    auto& g = globals::instance();

	auto parentProcess = GetModuleHandle(NULL);
	g.main_window = utils::get_main_window(module_info->pid);

    hooks::init();
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(instance);

		/*if (auto basePath = Utils::GetBasePath(hInstance); basePath)
		{
			SetDllDirectory((*basePath).c_str());
		}*/

		auto pid = GetCurrentProcessId();

		module_info = new utils::module_info();
		module_info->reason = reason;
		module_info->instance = instance;
		module_info->pid = pid;


		CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(&ThreadMain), nullptr, NULL, NULL);

		return true;
	}
	else
	{
		return true;
	}
}


/*uint32_t studio = Memory::FindPattern("hw.dll", { 
        0x85, 0xC0, 0x74, 0x40, 0x8B, 0x44, 0x24, 0x04, 0x68, 0x00, 0x00, 0x00, 0x00, 0x50, 0xFF, 0x15,
        0x00, 0x00, 0x00, 0x00, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x85, 0xC0, 0x74, 0x27, 0x68, 0x00, 0x00,
        0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 0x00, 0x6A, 0x01, 0xFF, 0xD0, 0x83, 0xC4, 0x0C, 0x85, 0xC0
     }, 30);
    g_Studio = reinterpret_cast<engine_studio_api_s*>(studio + offset);*/