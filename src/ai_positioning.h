#pragma once

extern "C"
{
    #include "d_player.h"
    #include "p_mobj.h"
}

void PlayerCombatMove(player_t* player, mobj_t* target);
