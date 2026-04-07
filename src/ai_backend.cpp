#include <unordered_set>
#include <unordered_map>
#include <iostream>

#include "ai_backend.h"
#include "ai_pathfinding.h"
#include "ai_targeting.hpp"
#include "ai_positioning.h"
#include "ai_utils.h"

extern "C"
{
#include "d_event.h"
#include "r_defs.h"
#include "r_state.h"
}

AI_Targeting *ai_targeting;
std::unordered_set<int16_t> exitSpecials = {11, 52, 197};

line_t* exitLine;

// Called after a new level is loaded.
void AI_Init()
{
    for (int32_t i = 0; i < numlines; i++)
    {
        line_t& line = lines[i];
        if (exitSpecials.contains(line.special))
        {
            exitLine = &line;
            break;
        }
    }
    InitPathfinding();
}

// The main entry point for all AI logic. Called every tick.
// Call all AI systems from here.
void AI_Tick(player_t* player)
{
    if (ai_targeting == nullptr)
    {
        ai_targeting = new AI_Targeting(player);
    }

    float targetDistance;
    mobj_t* target = ai_targeting->Get_Target_Enemy(targetDistance);

    if (target == nullptr || targetDistance > 1536)
    {
        float exitX = LineMidX(exitLine);
        float exitY = LineMidY(exitLine);
        PathState state = PathTowards(player, exitX, exitY);
        if (state == PATH_COMPLETE)
        {
            PlayerLookAt(player, exitX, exitY);
            MovePlayerTowards(player, exitX, exitY);
            if (PlayerDistance(player, exitX, exitY) < 62)
            {
                PlayerInteract(player);
            }
        }
    }
    else
    {
        PlayerCombatMove(player, ai_targeting, target);
        ai_targeting->Shoot_Enemy(player, target);
    }


    ai_targeting->ticks_since_swap++;
    std::cout << ai_targeting->ticks_since_swap << std::endl;
}
