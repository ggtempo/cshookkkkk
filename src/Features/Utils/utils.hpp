#pragma once
#include "../../Utils/globals.hpp"
#include "../../HLSDK/Net.hpp"

namespace utils
{
    // Helper functions
    bool is_valid_player(cl_entity_s* entity);
    CBasePlayerWeapon* get_weapon_info(custom::weapon_id id);

    // Update status
    void update_visibility();
    void update_status();
    void setup_hitboxes();

    // Autowall helpers    
    pmtrace_t better_trace(math::vec3 start, math::vec3 end);
    char get_texture_type(pmtrace_t& trace, math::vec3& start, math::vec3& end);
    int get_estimated_damage(math::vec3 start, math::vec3 end, custom::weapon_id weapon_id, int target_id, int hitbox_id);
}