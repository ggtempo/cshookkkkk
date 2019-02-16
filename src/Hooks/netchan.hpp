
#pragma once
#include "hooks.hpp"
#include "../Utils/globals.hpp"

namespace hooks
{
    int hk_netchan_canpacket(void* netchan)
    {
        using netchan_canpacket_fn = int(*)(void*);
        
        static auto& g = globals::instance();
        static auto original_func = reinterpret_cast<netchan_canpacket_fn>(g.original_can_packet);

        if (g.send_packet)
            return original_func(netchan);

        return false;
    }
}