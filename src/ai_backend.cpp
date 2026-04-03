#include <unordered_set>

#include "ai_backend.h"
#include "ai_pathfinding.h"
#include "ai_targeting.hpp"
#include "ai_utils.h"

extern "C"
{
#include "d_event.h"
#include "r_defs.h"
#include "r_state.h"
}

AI_Targeting *ai_targeting;
std::unordered_set<int16_t> exitSpecials = {11, 51, 52, 124, 197, 198};

// Called after a new level is loaded.
void AI_Init()
{
    for (uint32_t i = 0; i < numlines; i++)
    {
        line_t &line = lines[i];
        if (exitSpecials.contains(line.special))
        {
            exitLine = &line;
            break;
        }
    }
}

// The main entry point for all AI logic. Called every tick.
// Call all AI systems from here.
void AI_Tick(player_t *player)
{
    // float exitX = LineMidX(exitLine);
    // float exitY = LineMidY(exitLine);
    // PathState state = PathTowards(player, exitX, exitY);
    // if (state == PATH_COMPLETE)
    // {
    //     PlayerLookAt(player, exitX, exitY);v
    //     MovePlayerTowards(player, exitX, exitY);
    // }

    if (ai_targeting == nullptr)
    {
        ai_targeting = new AI_Targeting(player);
    }

    ai_targeting->Shoot_Closest_Enemy(player);
}
