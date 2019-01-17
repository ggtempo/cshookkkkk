#pragma once
#include <Windows.h>
#include "../HLSDK/enginefuncs.hpp"
#include "../HLSDK/clientfuncs.hpp"
#include "../HLSDK/playermove.hpp"
#include "../Memory/Memory.hpp"

namespace Hooks
{
    void Init();
    void Print(const char* text);

    cl_enginefunc_t*    GetEngineFuncs();
    cldll_func_t*       GetClientFuncs();
}