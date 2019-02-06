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

    enum class weapon_id
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

    enum class bullet_id
    {
        bullet_none = 0,
        bullet_9mm,
        bullet_mp5,
        bullet_357,
        bullet_buckshot,
        bullet_crowbar,

        bullet_m_9mm,
        bullet_m_mp5,
        bullet_m_12mm,

        bullet_45acp,
        bullet_338mag,
        bullet_762mm,
        bullet_556mm,
        bullet_50ae,
        bullet_57mm,
        bullet_357sig
    };

    inline bool is_knife(weapon_id id)
    {
        return (id == weapon_id::weapon_knife);
    }

    inline bool is_grenade(weapon_id id)
    {
        return ((id == weapon_id::weapon_hegrenade)    ||
                (id == weapon_id::weapon_smokegrenade) ||
                (id == weapon_id::weapon_flashbang));
    }

    inline bool is_c4(weapon_id id)
    {
        return (id == weapon_id::weapon_c4);
    }

    inline bool is_shield(weapon_id id)
    {
        return (id == weapon_id::weapon_shield);
    }

    inline bool is_gun(weapon_id id)
    {
        return  !is_knife(id) &&
                !is_c4(id) &&
                !is_grenade(id) && 
                !is_shield(id) &&
                (id != weapon_id::weapon_invalid) && 
                (id <= weapon_id::weapon_p90);
    }

    inline float get_spread(weapon_id id, float accuracy, float velocity, bool grounded, bool ducking, float fov, int state)
    {
        switch (id)
        {
            case weapon_id::weapon_ak47:
            {
                if (grounded)
                {
                    if (velocity > 140)
                        return 0.04 + (0.07) * accuracy;    // Running on ground
                    else
                        return 0.0275 * accuracy;           // Crouching / Walking slow
                }
                else
                {
                    return 0.04 + (0.4) * accuracy;         // In air
                }
            }

            case weapon_id::weapon_aug:
            {
                if (grounded)
                {
                    if (velocity > 140)
                        return 0.035 + (0.07) * accuracy;
                    else if (fov == 90)
                        return (0.02) * accuracy;
                    else
                        return (0.02) * accuracy;
                }
                else
                {
                    return 0.035 + (0.4) * accuracy;
                }
                
            }

            case weapon_id::weapon_awp:
            {
                if (grounded)
                {
                    if (velocity > 140)
                        return 0.25;
                    else if (velocity > 10)
                        return 0.1;
                    else if (ducking)
                        return 0.0;
                    else
                        return 0.001;
                }
                else
                {
                    return 0.85;
                }
            }

            case weapon_id::weapon_deagle:
            {
                if (grounded)
                {
                    if (velocity > 0)
                        return (0.25) * (1 - accuracy);
                    else if (ducking)
                        return (0.115) * (1 - accuracy);
                    else
                        return (0.13) * (1 - accuracy);
                }
                else
                {
                    return (1.5) * (1 - accuracy);
                }
            }

            case weapon_id::weapon_elite:
            {
                if (grounded)
                {
                    if (velocity > 0)
                        return (0.175) * (1 - accuracy);
                    else if (ducking)
                        return (0.08) * (1 - accuracy);
                    else
                        return (0.1) * (1 - accuracy);
                }
                else
                {
                    return (1.3) * (1 - accuracy);
                }
            }

            case weapon_id::weapon_famas:
            {
                if (grounded)
                {
                    if (velocity > 140)
                        return 0.030 + (0.07) * accuracy;
                    else
                        return (0.02) * accuracy;
                }
                else
                {
                    return 0.030 + (0.3) * accuracy;
                }
                
            }

            case weapon_id::weapon_fiveseven:
            {
                if (grounded)
                {
                    if (velocity > 0)
                        return (0.255) * (1 - accuracy);
                    else if (ducking)
                        return (0.075) * (1 - accuracy);
                    else
                        return (0.15) * (1 - accuracy);
                }
                else 
                {
                    return (1.5) * (1 - accuracy);
                }
            }

            case weapon_id::weapon_g3sg1:
            {
                if (grounded)
                {
                    if (velocity > 0)
                        return 0.15;
                    else if (ducking)
                        return 0.035;
                    else
                        return 0.055;
                }
                else
                {
                    return 0.45;
                }
                
            }

            case weapon_id::weapon_galil:
            {
                if (grounded)
                {
                    if (velocity > 140)
                        return 0.04 + (0.07) * accuracy;
                    else
                        return (0.0375) * accuracy;
                }
                else
                {
                    return 0.04 + (0.3) * accuracy;
                }
                
            }

            case weapon_id::weapon_glock18:
            {
                if (grounded)
                {
                    if (velocity > 0)
                        return (0.185) * (1 - accuracy);
                    else if (ducking)
                        return (0.095) * (1 - accuracy);
                    else
                        return (0.3) * (1 - accuracy);
                }
                else
                {
                    return (1.2) * (1 - accuracy);
                }
                
            }

            case weapon_id::weapon_m249:
            {
                if (grounded)
                {
                    if (velocity > 140)
                        return 0.045 + (0.095) * accuracy;
                    else
                        return (0.03) * accuracy;
                    
                }
                else
                {
                    return 0.045 + (0.5) * accuracy;
                }
                
            }

            // Shotgun
            case weapon_id::weapon_m3:
                return 0.0;

            // Todo
            case weapon_id::weapon_m4a1:
            {
                if (state & WPNSTATE_M4A1_SILENCED)
                {
                    if (grounded)
                    {
                        if (velocity > 140)
                            return 0.035 + (0.07) * accuracy;
                        else
                            return (0.025) * accuracy;
                    }
                    else
                    {
                        return 0.035 + (0.4) * accuracy;
                    }
                }
                else
                {
                    if (grounded)
                    {
                        if (velocity > 140)
                            return 0.035 + (0.07) * accuracy;
                        else
                            return (0.02) * accuracy;
                    }
                    else
                    {
                        return 0.035 + (0.4) * accuracy;
                    }
                }
            }

            case weapon_id::weapon_mac10:
            {
                if (grounded)
                    return (0.03) * accuracy;
                else
                    return (0.375) * accuracy;
            }

            case weapon_id::weapon_mp5navy:
            {
                if (grounded)
                    return (0.04) * accuracy;
                else
                    return (0.2) * accuracy;
            }

            case weapon_id::weapon_p228:
            {
                if (grounded)
                {
                    if (velocity > 0)
                        return (0.255) * (1 - accuracy);
                    else if (ducking)
                        return (0.075) * (1 - accuracy);
                    else
                        return (0.15) * (1 - accuracy);
                }
                else
                {
                    return (1.5) * (1 - accuracy);
                }
                
            }

            case weapon_id::weapon_p90:
            {
                if (grounded)
                {
                    if (velocity > 170)
                        return (0.115) * accuracy;
                    else
                        return (0.045) * accuracy;
                }
                else
                {
                    return (0.3) * accuracy;
                }
                
            }

            case weapon_id::weapon_scout:
            {
                if (grounded)
                {
                    if (velocity > 170)
                        return 0.075;
                    else if (ducking)
                        return 0.0;
                    else
                        return 0.007;
                }
                else
                {
                    return 0.2;
                }
            }

            case weapon_id::weapon_sg550:
            {
                if (grounded)
                {
                    if (velocity > 0)
                        return 0.15;
                    else if (ducking)
                        return (0.04) * (1 - accuracy);
                    else
                        return (0.05) * (1 - accuracy);
                }
                else
                {
                    return (0.45) * (1 - accuracy);
                }
            }

            case weapon_id::weapon_sg552:
            {
                if (grounded)
                {
                    if (velocity > 140)
                        return 0.035 + (0.075) * accuracy;
                    else if (fov == 90)
                        return (0.02) * accuracy;
                    else
                        return (0.02) * accuracy;
                }
                else
                {
                    return 0.035 + (0.45) * accuracy;
                }
            }

            case weapon_id::weapon_tmp:
            {
                if (grounded)
                {
                    return 0.03 * accuracy;
                }
                else
                {
                    return 0.25 * accuracy;
                }
            }

            case weapon_id::weapon_ump45:
            {
                if (grounded)
                {
                    return 0.04 * accuracy;
                }
                else
                {
                    return 0.24 * accuracy;
                }
            }

            // Todo
            case weapon_id::weapon_usp:
            {
                if (state & WPNSTATE_USP_SILENCED)
                {
                    if (grounded)
                    {
                        if (velocity > 0)
                            return (0.25) * (1 - accuracy);
                        else if (ducking)
                            return (0.125) * (1 - accuracy);
                        else
                            return (0.15) * (1 - accuracy);
                    }
                    else
                    {
                        return (1.3) * (1 - accuracy);
                    }
                }
                else
                {
                    if (grounded)
                    {
                        if (velocity > 0)
                            return (0.225) * (1 - accuracy);
                        else if (ducking)
                            return (0.08) * (1 - accuracy);
                        else
                            return (0.1) * (1 - accuracy);
                    }
                    else
                    {
                        return (1.2) * (1 - accuracy);
                    }
                }
            }

            // Shotgun
            case weapon_id::weapon_xm1014:
                return 0.0;

            default:
                // If we don't know the weapon's spread, just return the 0
                return 0.0;
        }
    }

    struct bullet_params
    {
        float penetration_range;
        int penetration_power;
    };

    struct weapon_params
    {
        bullet_params bullet;
        float range;
        int max_penetrations;
        int damage;

        float damage_dropoff;
        float armor_penetration;
    };

    inline bullet_params get_bullet_params(bullet_id id)
    {
        switch (id)
        {
            case bullet_id::bullet_9mm:
            {
                return {800, 21};
            }

            case bullet_id::bullet_45acp:
            {
                return {500, 15};
            }

            case bullet_id::bullet_50ae:
            {
                return {1000, 30};
            }

            case bullet_id::bullet_762mm:
            {
                return {5000, 39};
            }

            case bullet_id::bullet_556mm:
            {
                return {4000, 35};
            }

            case bullet_id::bullet_338mag:
            {
                return {8000, 45};
            }

            case bullet_id::bullet_57mm:
            {
                return {2000, 30};
            }

            case bullet_id::bullet_357sig:
            {
                return {800, 25};
            }

            default:
            {
                return {0, 0};
            }
        }
    };

    inline weapon_params get_weapon_params(weapon_id id)
    {
        switch (id)
        {
            case weapon_id::weapon_glock18:
                return {
                    get_bullet_params(bullet_id::bullet_9mm),
                    4096, 2, 25, 0.75, 0.475
                };
            case weapon_id::weapon_usp:
                return {
                    get_bullet_params(bullet_id::bullet_45acp),
                    8192, 2, 30, 0.79, 0.505 
                };
            case weapon_id::weapon_p228:
                return {
                    get_bullet_params(bullet_id::bullet_357sig),
                    8192, 2, 32, 0.8, 0.7768
                };
            case weapon_id::weapon_deagle:
                return {
                    get_bullet_params(bullet_id::bullet_50ae),
                    8192, 2, 54, 0.81, 0.932
                };
            case weapon_id::weapon_fiveseven:
                return {
                    get_bullet_params(bullet_id::bullet_57mm),
                    8192, 2, 20, 0.885, 0.9115
                };
            case weapon_id::weapon_elite:
                return {
                    get_bullet_params(bullet_id::bullet_9mm),
                    8192, 2, 36, 0.75, 0.525
                };
            case weapon_id::weapon_m3:
                return {
                    get_bullet_params(bullet_id::bullet_buckshot),
                    8192, 1, 180, 1.0, 0.5
                };
            case weapon_id::weapon_xm1014:
                return {
                    get_bullet_params(bullet_id::bullet_buckshot),
                    8192, 1, 120, 1.0, 0.8
                };
            case weapon_id::weapon_tmp:
                return {
                    get_bullet_params(bullet_id::bullet_9mm),
                    8192, 2, 20, 0.85, 0.6
                };
            case weapon_id::weapon_mac10:
                return {
                    get_bullet_params(bullet_id::bullet_45acp),
                    8192, 2, 29, 0.82, 0.575
                };
            case weapon_id::weapon_mp5navy:
                return {
                    get_bullet_params(bullet_id::bullet_9mm),
                    8192, 2, 26, 0.84, 0.69 // No idea
                };
            case weapon_id::weapon_ump45:
                return {
                    get_bullet_params(bullet_id::bullet_45acp),
                    8192, 2, 30, 0.82, 0.65
                };
            case weapon_id::weapon_p90:
                return {
                    get_bullet_params(bullet_id::bullet_57mm),
                    8192, 2, 21, 0.885, 0.69
                };
            case weapon_id::weapon_m249:
                return {
                    get_bullet_params(bullet_id::bullet_556mm),
                    8192, 2, 32, 0.97, 0.8
                };
            case weapon_id::weapon_galil:
                return {
                    get_bullet_params(bullet_id::bullet_556mm),
                    8192, 2, 30, 0.98, 0.775
                };
            case weapon_id::weapon_famas:
                return {
                    get_bullet_params(bullet_id::bullet_556mm),
                    8192, 2, 30, 0.96, 0.7
                };
            case weapon_id::weapon_ak47:
                return {
                    get_bullet_params(bullet_id::bullet_762mm),
                    8192, 2, 36, 0.98, 0.775
                };
            case weapon_id::weapon_m4a1:
                return {
                    get_bullet_params(bullet_id::bullet_556mm),
                    8192, 2, 32, 0.95, 0.7
                };
            case weapon_id::weapon_sg552:
                return {
                    get_bullet_params(bullet_id::bullet_556mm),
                    8192, 2, 33, 0.955, 1.0
                };
            case weapon_id::weapon_aug:
                return {
                    get_bullet_params(bullet_id::bullet_556mm),
                    8192, 2, 32, 0.96, 0.90
                };
            case weapon_id::weapon_scout:
                return {
                    get_bullet_params(bullet_id::bullet_762mm),
                    8192, 2, 75, 0.98, 0.85
                };
            case weapon_id::weapon_awp:
                return {
                    get_bullet_params(bullet_id::bullet_338mag),
                    8192, 3, 115, 0.99, 0.975
                };
            case weapon_id::weapon_g3sg1:
                return {
                    get_bullet_params(bullet_id::bullet_762mm),
                    8192, 2, 80, 0.98, 0.825
                };
            case weapon_id::weapon_sg550:
                return {
                    get_bullet_params(bullet_id::bullet_556mm),
                    8192, 2, 70, 0.98, 0.825
                };
        }
    }

    inline float get_hitbox_damage_modifier(hitbox_numbers id)
    {
        switch(id)
        {
            case hitbox_numbers::head:      // Head - 4x damage
                return 4;
            case hitbox_numbers::neck:
            case hitbox_numbers::torso_top:
            case hitbox_numbers::heart:
            case hitbox_numbers::left_arm_bottom:
            case hitbox_numbers::left_arm_top:
            case hitbox_numbers::left_shoulder:
            case hitbox_numbers::right_arm_bottom:
            case hitbox_numbers::right_arm_top:
            case hitbox_numbers::right_shoulder:
                return 1;                   // Upper body - 1x damage
            case hitbox_numbers::pelvis:
            case hitbox_numbers::torso_bottom:
                return 1.25;                // Lower body - 1.25x damage
            default:
                return 0.75;                // Legs - 0.75x damage
        }
    }

    inline math::vec3 get_spread_vec(unsigned int random_seed, float spread)
    {
        constexpr unsigned int future = 1;
        float spread_x = (math::shared_random_float(random_seed + future, -0.5, 0.5)     + math::shared_random_float(random_seed + future + 1, -0.5, 0.5)) * spread;
		float spread_y = (math::shared_random_float(random_seed + future + 2, -0.5, 0.5) + math::shared_random_float(random_seed + future + 3, -0.5, 0.5)) * spread;

        return {spread_x, spread_y, 0.0};
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
        float accuracy;
    };

    struct player_data
    {
        bool alive = false;
        bool dormant = true;
        player_team team = player_team::UNKNOWN;
        weapon_data weapon;

        char* name;

        math::vec3 origin;
        math::vec3 velocity;

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