#pragma once
#include "math.hpp"
#include <unordered_map>

namespace custom
{
    struct angled_bbox
    {
        int bone;
        math::bbox box;
        math::matrix3x4 matrix;
    };

    enum player_team
    {
        CT,
        T,
        UNKNOWN
    };

    union color4f
    {
        struct
        {
            float r, g, b, a;
        };
        struct
        {
            float clr[4];
        };
    };

    struct player_data
    {
        player_team team;
        std::unordered_map<int, angled_bbox> hitboxes;
    };
}