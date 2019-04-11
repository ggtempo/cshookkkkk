#pragma once
#include <cstring>
#include "../hooks.hpp"

#include "../../Utils/globals.hpp"
#include "../../Utils/math.hpp"

#include "../../HLSDK/Parsemsg.hpp"
#include "../../HLSDK/Weapons.hpp"
#include "../../HLSDK/Textures.hpp"

#include "../../Features/Utils/utils.hpp"
#include "../../Features/Miscelaneous/miscelaneous.hpp"

namespace hooks
{
    void hk_screenshot()
    {
        features::miscelaneous::instance().on_screenshot();
    }

    void hk_snapshot()
    {
        features::miscelaneous::instance().on_snapshot();
    }
}