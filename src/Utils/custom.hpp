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

    struct player_data
    {
        std::unordered_map<int, angled_bbox> hitboxes;
    };
}