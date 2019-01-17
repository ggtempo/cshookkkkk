#include <iostream>
#include <memory>
#include <optional>
#include <cstdint>
#include <sstream>
#include <unordered_set>

#include <Windows.h>
#include <gl/GL.h>

#include "HLSDK/Studio.hpp"

#include "Memory/Memory.hpp"
#include "Hooks/Hooks.hpp"
#include "Utils/utils.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl.h"

#include "Utils/globals.hpp"

utils::ModuleInfo* moduleInfo;
bool first = true;



void ThreadMain()
{
    auto& g = globals::instance();


	auto parentProcess = GetModuleHandle(NULL);
	g.main_window = utils::GetMainWindow(moduleInfo->pid);

    Hooks::Init();
    
    /*std::stringstream ss;
    ss << "HUD_...: " << std::hex << (uint32_t)GetProcAddress(GetModuleHandle(L"client.dll"), "HUD_GetStudioModelInterface") << "\n";
    ss << "g_Studio: " << std::hex << (uint32_t)g.engine_studio << "\n";
    ss << "Studio: " << std::hex << (uint32_t)studioModelRenderer << "\n";
    ss << "Studio2: " << std::hex << (uint32_t)studioModelRenderer2 << "\n";
    ss << "Offset: " << std::hex << offset << "\n";
    Hooks::Print(ss.str().c_str());*/
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fwdReason, LPVOID reserved)
{
	if (fwdReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);

		/*if (auto basePath = Utils::GetBasePath(hInstance); basePath)
		{
			SetDllDirectory((*basePath).c_str());
		}*/

		auto pid = GetCurrentProcessId();

		moduleInfo = new utils::ModuleInfo();
		moduleInfo->fwdReason = fwdReason;
		moduleInfo->hInstance = hInstance;
		moduleInfo->pid = pid;


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
    g_Studio = reinterpret_cast<engine_studio_api_s*>(studio + offset);

	

    std::stringstream ss;
    ss << "g_Studio: " << std::hex << g_Studio << "\n";
    ss << "Studio: " << std::hex << (uint32_t)studio << "\n";
    ss << "Offset: " << std::hex << offset << "\n";
    Hooks::Print(ss.str().c_str());*/