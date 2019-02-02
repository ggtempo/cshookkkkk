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