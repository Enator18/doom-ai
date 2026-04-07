#pragma once

#include "ai_targeting.hpp"

extern "C"
{
    #include "d_player.h"
    #include "p_mobj.h"
}

void PlayerCombatMove(player_t* player, AI_Targeting* targeting, mobj_t* target);
