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
#include "Utils/Utils.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl.h"

#include "Utils/Globals.hpp"

Utils::ModuleInfo* moduleInfo;
HWND mainWindow;
uintptr_t origWndProc;
bool first = true;
std::unordered_set<std::string> models;

engine_studio_api_s* g_Studio;

typedef LRESULT(__stdcall*WNDPRC)(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!first)
        ImGui_Impl_WndProcHandler(hWnd, uMsg, wParam, lParam);

    auto& io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
    {
        // Dont pass keyboard/mouse input to the game
        //return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

	return reinterpret_cast<WNDPRC>(origWndProc)(hWnd, uMsg, wParam, lParam);
}

typedef BOOL(__stdcall *wglSwapBuffersFn)(HDC);
wglSwapBuffersFn owglSwapBuffers;
BOOL __stdcall hwglSwapBuffers(HDC hDc)
{
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

    if (first) {
        first = false;

        // Debug OpenGL Stuff
        auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        auto extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

        /*Hooks::Print("GL Version: ");
        Hooks::Print(version);
        Hooks::Print("\n");

        Hooks::Print("GL Extensions: ");
        Hooks::Print(extensions);
        Hooks::Print("\n");*/

        

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        auto& style = ImGui::GetStyle();
        //style.AntiAliasedFill = false;
        //style.AntiAliasedLines = false;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_Impl_Init(mainWindow);
    }
    
    ImGui_Impl_NewFrame();
    ImGui::NewFrame();
    //{
    //    ImGui::Begin("Models");
    //
    //    ImGui::Text("Registered models: \n\n");
    //
    //    for (const auto& key : models)
    //        ImGui::Text(key.c_str());
    //}

    auto& g = Globals::Instance();
    ImGui::Begin("Test Settings");
    {
        ImGui::Checkbox("Bhop", &g.bhop_enabled);
        ImGui::Checkbox("Backtrack", &g.backtrack_enabled);
        ImGui::Checkbox("Triggerbot", &g.trigger_enabled);
        ImGui::Checkbox("Triggerbot teammates", &g.trigger_team);
        ImGui::SliderFloat("Backtrack time", &g.backtrack_time, 0, 1000);

        if (g.studioModelRendererHook)
        {
            auto time = g.studioModelRendererHook->GetBase<CStudioModelRenderer*>()->m_clTime;
            ImGui::Text("Time is: %f\n", time);
        }

        ImGui::SliderInt("Render mode: ", &g.render_mode, 0, render_modes::kRenderCount);
        ImGui::SliderInt("Render effects: ", &g.render_fx, 0, render_effects::kRenderFxCount);
        //ImGui::SliderInt("Studio nf", &g.s_nf, 0, )
        ImGui::InputInt("S_NF", &g.s_nf);
        ImGui::SliderInt("FX Amount: ", &g.fx_amt, 0, 255);
        ImGui::ColorPicker3("FX Colors", reinterpret_cast<float*>(&g.clr));


        ImGui::InputInt("Trace mode: ", &g.trace_mode);
        ImGui::InputInt("Trace flags: ", &g.trace_flags);
    }
    ImGui::End();


    /*if (g.engine_funcs)
    {
        auto lp = g.engine_funcs->GetLocalPlayer();
        if (lp)
        {
            ImGui::Begin("Player info");
            {
                ImGui::Text("Is player: %i", lp->player);
                ImGui::Text("Health: %i", lp->curstate.health);
                ImGui::Text("Origin: %f %f %f", lp->origin.x, lp->origin.y, lp->origin.z);
                ImGui::Text("Origin (curstate): %f %f %f", lp->curstate.origin.x, lp->curstate.origin.y, lp->curstate.origin.z);
                //ImGui::Text("Position: %f %f %f", lp->current_position.x, lp->current_position.y, lp->current_position.z);
                

            }
            ImGui::End();
        }
    }*/
    

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

typedef void(__stdcall *glBeginFn)(GLenum);
glBeginFn oglBegin;

void hhglBegin(GLenum mode)
{
    /*auto entity = g_Studio->GetCurrentEntity();
    if (entity && entity->model && entity->player)
    {
        std::string modelStr = entity->model->name;
        if (models.find(modelStr) == models.end())
            models.insert(modelStr);
        
        Hooks::Print("Valid entity found!\n");
        if((mode == GL_TRIANGLE_STRIP) || (mode == GL_TRIANGLE_FAN))
        {
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST); 
            glColor3f(1, 0, 0);
        }
    }*/

    return oglBegin(mode);
}

void __stdcall hglBegin(GLenum mode)
{
    return hhglBegin(mode);
}

typedef void (__stdcall *glEndFn)();
glEndFn oglEnd;
void __stdcall hglEnd()
{
    return oglEnd();
}


typedef void(__thiscall* fnStudioRenderModel)(void* ecx);
void __fastcall hkStudioRenderModel(CStudioModelRenderer* ecx, void* edx)
{
    static auto& g = Globals::Instance();
    static auto oFunc = g.studioModelRendererHook->GetOVFunc<fnStudioRenderModel>(18);

    auto entity = g.engine_studio->GetCurrentEntity();
    auto local = g.engine_funcs->GetLocalPlayer();

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

		//g.engine_funcs->pTriAPI->SpriteTexture( m_pChromeSprite, 0 );
		entity->curstate.renderfx = kRenderFxGlowShell;

		ecx->StudioRenderFinal( );
		if ( !g.engine_studio->IsHardware() )
		{
			g.engine_funcs->pTriAPI->RenderMode( kRenderNormal );
		}

        oFunc(ecx);
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
            glEnable(GL_TEXTURE_2D);
            glDepthRange(0.1, 1);
        }
		else
        {
            ecx->StudioRenderFinal( );
        }
	}
}

void ThreadMain()
{
    auto& g = Globals::Instance();


	auto parentProcess = GetModuleHandle(NULL);
	mainWindow = Utils::GetMainWindow(moduleInfo->pid);

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    std::cout << "This works" << std::endl;

    Hooks::Init();

	uintptr_t wglSwapBuffersLoc = reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers"));
	owglSwapBuffers = reinterpret_cast<wglSwapBuffersFn>(Memory::HookFunc2(wglSwapBuffersLoc, reinterpret_cast<uintptr_t>(hwglSwapBuffers), 5));
	origWndProc = SetWindowLongPtr(mainWindow, GWL_WNDPROC, (LONG_PTR)&hWndProc);

    uintptr_t glBeginLoc = reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle(L"opengl32.dll"), "glBegin"));
    oglBegin = reinterpret_cast<glBeginFn>(Memory::HookFunc2(glBeginLoc, reinterpret_cast<uintptr_t>(hglBegin), 5));

    uintptr_t glEndLoc = reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle(L"opengl32.dll"), "glEnd"));
    oglEnd = reinterpret_cast<glEndFn>(Memory::HookFunc2(glEndLoc, reinterpret_cast<uintptr_t>(hglEnd), 5));

    uint32_t offset = reinterpret_cast<uint32_t>(Memory::GetModuleInfo("client.dll").lpBaseOfDll);
    auto HUD_GetStudioModelInterface = (uint32_t)Hooks::GetClientFuncs()->pStudioInterface;//reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle(L"client.dll"), "HUD_GetStudioModelInterface"));

    g.engine_studio = *reinterpret_cast<engine_studio_api_s**>(HUD_GetStudioModelInterface + 0x1A);
    auto studioModelRenderer = *reinterpret_cast<CStudioModelRenderer**>(HUD_GetStudioModelInterface + 0x27);
    auto studioModelRenderer2 = reinterpret_cast<CStudioModelRenderer*>(Memory::FindPattern("client.dll", {0x56, 0x8B, 0xF1, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x06, 0x00, 0x00, 0x00, 0x00, 0xC6, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xC6, 0x5E, 0xC3}, 0xA));
    
    std::stringstream ss;
    ss << "HUD_...: " << std::hex << (uint32_t)GetProcAddress(GetModuleHandle(L"client.dll"), "HUD_GetStudioModelInterface") << "\n";
    ss << "g_Studio: " << std::hex << (uint32_t)g.engine_studio << "\n";
    ss << "Studio: " << std::hex << (uint32_t)studioModelRenderer << "\n";
    ss << "Studio2: " << std::hex << (uint32_t)studioModelRenderer2 << "\n";
    ss << "Offset: " << std::hex << offset << "\n";
    Hooks::Print(ss.str().c_str());

    

    g.studioModelRendererHook = new Memory::VMTHook(studioModelRenderer);
    g.studioModelRendererHook->HookVFunc(reinterpret_cast<void*>(hkStudioRenderModel), 18);
    g.studioModelRendererHook->Hook();
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

		moduleInfo = new Utils::ModuleInfo();
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