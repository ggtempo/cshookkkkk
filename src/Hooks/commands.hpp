#pragma once
#include <cstring>
#include "hooks.hpp"

#include "../Utils/globals.hpp"
#include "../Utils/math.hpp"

#include "../HLSDK/Parsemsg.hpp"
#include "../HLSDK/Weapons.hpp"
#include "../HLSDK/Textures.hpp"

#include "../Features/Utils/utils.hpp"

namespace hooks
{
    void hk_screenshot()
    {
        static auto& g = globals::instance();
        g.taking_screenshot = true;
    }

    void hk_snapshot()
    {
        static auto& g = globals::instance();
        g.taking_snapshot = true;
    }
}