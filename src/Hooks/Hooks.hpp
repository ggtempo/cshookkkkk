#pragma once
#include <Windows.h>
#include "../HLSDK/enginefuncs.hpp"
#include "../HLSDK/clientfuncs.hpp"
#include "../HLSDK/playermove.hpp"
#include "../Memory/memory.hpp"

namespace hooks
{
    void init();

    cl_enginefunc_t*    get_engine_funcs();
    cldll_func_t*       get_client_funcs();
}