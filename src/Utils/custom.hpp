#pragma once
#include "math.hpp"
#include <unordered_map>
#include <string>

namespace custom
{
    struct angled_bbox
    {
        int bone;
        bool visible;
        math::bbox box;
        math::matrix3x4 matrix;
    };

    enum player_team
    {
        CT,
        T,
        UNKNOWN
    };

    enum weapon_id
    {
        weapon_invalid = 0,
        weapon_p228 = 1,
        weapon_shield,
        weapon_scout,
        weapon_hegrenade,
        weapon_xm1014,
        weapon_c4,
        weapon_mac10,
        weapon_aug,
        weapon_smokegrenade,
        weapon_elite,
        weapon_fiveseven,
        weapon_ump45,
        weapon_sg550,
        weapon_galil,
        weapon_famas,
        weapon_usp,
        weapon_glock18,
        weapon_awp,
        weapon_mp5navy,
        weapon_m249,
        weapon_m3,
        weapon_m4a1,
        weapon_tmp,
        weapon_g3sg1,
        weapon_flashbang,
        weapon_deagle,
        weapon_sg552,
        weapon_ak47,
        weapon_knife,
        weapon_p90
    };

    inline bool is_knife(weapon_id id)
    {
        return (id == weapon_knife);
    }

    inline bool is_grenade(weapon_id id)
    {
        return ((id == weapon_hegrenade)    ||
                (id == weapon_smokegrenade) ||
                (id == weapon_flashbang));
    }

    inline bool is_c4(weapon_id id)
    {
        return (id == weapon_c4);
    }

    inline bool is_shield(weapon_id id)
    {
        return (id == weapon_shield);
    }

    inline bool is_gun(weapon_id id)
    {
        return  !is_knife(id) &&
                !is_c4(id) &&
                !is_grenade(id) && 
                !is_shield(id) &&
                (id != weapon_invalid) && 
                (id <= weapon_p90);
    }

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

    struct weapon_data
    {
        weapon_id id;
        float next_primary_attack;
        float next_secondary_attack;
        float next_attack;
        bool in_reload;

        int clip;
        int ammo;

        unsigned int seed;
    };

    struct player_data
    {
        bool alive = false;
        bool dormant = true;
        player_team team = player_team::UNKNOWN;
        weapon_data weapon;
        std::unordered_map<int, angled_bbox> hitboxes;
    };

    static const std::unordered_map<int, const char*> key_map = {
            {VK_LBUTTON, "Mouse 1"},
			{VK_RBUTTON, "Mouse 3"},
			{VK_MBUTTON, "Mouse 2"},
			{VK_XBUTTON1, "Mouse 5" },
			{VK_XBUTTON2, "Mouse 4" },
			{VK_BACK, "Backspace"},
			{VK_TAB, "Tab" },
			{VK_RETURN, "Return"},
			{VK_SHIFT, "Shift"},
			{VK_RSHIFT, "Right shift"},
			{VK_CONTROL, "Ctrl"},
			{VK_MENU, "Menu"},
			{VK_ESCAPE, "ESC"},
			{VK_INSERT, "Insert" },
			{VK_DELETE, "Delete" },
            {VK_LEFT, "Left"},
            {VK_RIGHT, "Right"},
            {VK_UP, "Up"},
            {VK_DOWN, "Down"},
			{VK_F1, "F1"},
			{VK_F2, "F2"},
			{VK_F3, "F3"},
			{VK_F4, "F4"},
			{VK_F5, "F5"},
			{VK_F6, "F6"},
			{VK_F7, "F7"},
			{VK_F8, "F8"},
			{VK_F9, "F9"},
			{VK_F10, "F10"},
			{VK_F11, "F11"},
			{VK_F12, "F12"},
			{VK_PRIOR, "VK_PRIOR"},
			{VK_NEXT, "VK_NEXT"},
            {VK_SPACE, "Space"},
            {33, "!"},
            {34, "\""},
            {35, "#"},
            {36, "$"},
            {37, "%"},
            {38, "&"},
            {39, "'"},
            {40, "("},
            {41, ")"},
            {42, "*"},
            {43, "+"},
            {44, ","},
            {45, "-"},
            {46, "."},
            {47, "/"},
            {48, "0"},
            {49, "1"},
            {50, "2"},
            {51, "3"},
            {52, "4"},
            {53, "5"},
            {54, "6"},
            {55, "7"},
            {56, "8"},
            {57, "9"},
            {58, ":"},
            {59, ";"},
            {60, "<"},
            {61, "="},
            {62, ">"},
            {63, "?"},
            {64, "@"},
            {65, "A"},
            {66, "B"},
            {67, "C"},
            {68, "D"},
            {69, "E"},
            {70, "F"},
            {71, "G"},
            {72, "H"},
            {73, "I"},
            {74, "J"},
            {75, "K"},
            {76, "L"},
            {77, "M"},
            {78, "N"},
            {79, "O"},
            {80, "P"},
            {81, "Q"},
            {82, "R"},
            {83, "S"},
            {84, "T"},
            {85, "U"},
            {86, "V"},
            {87, "W"},
            {88, "X"},
            {89, "Y"},
            {90, "Z"},
            {91, "["},
            {92, "\\"},
            {93, "]"},
            {94, "^"},
            {95, "_"},
            {96, "`"},
            {97, "a"},
            {98, "b"},
            {99, "c"},
            {100, "d"},
            {101, "e"},
            {102, "f"},
            {103, "g"},
            {104, "h"},
            {105, "i"},
            {106, "j"},
            {107, "k"},
            {108, "l"},
            {109, "m"},
            {110, "n"},
            {111, "o"},
            {112, "p"},
            {113, "q"},
            {114, "r"},
            {115, "s"},
            {116, "t"},
            {117, "u"},
            {118, "v"},
            {119, "w"},
            {120, "x"},
            {121, "y"},
            {122, "z"},
            {123, "{"},
            {124, "|"},
            {125, "}"},
            {126, "~"}
    };

    inline const char* get_key_name(int key)
    {
        if (auto result = key_map.find(key); result != key_map.end())
        {
            return result->second;
        }

        return NULL;
    }
}