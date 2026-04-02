#include <unordered_set>
#include <vector>

#include "ai_backend.h"
#include "ai_utils.h"
#include "ai_pathfinding.h"
#include "ai_targeting.hpp"

extern "C"
{
    #include "r_defs.h"
    #include "r_state.h"
}

AI_Targeting* ai_targeting;

std::unordered_set<int16_t> exitSpecials = {11, 51, 52, 124, 197, 198};

// Called after a new level is loaded.
void AI_Init()
{
    for (uint32_t i = 0; i < numlines; i++)
    {
        line_t& line = lines[i];
        if (exitSpecials.contains(line.special))
        {
            exitLine = &line;
            break;
        }
    }
}

// The main entry point for all AI logic. Called every tick.
// Call all AI systems from here.
void AI_Tick(player_t* player)
{
    if (ai_targeting == nullptr)
    {
        ai_targeting = new AI_Targeting(player);
    }

    // float exitX = LineMidX(exitLine);
    // float exitY = LineMidY(exitLine);
    // PathState state = PathTowards(player, exitX, exitY);
    // if (state == PATH_COMPLETE)
    // {
    //     PlayerLookAt(player, exitX, exitY);
    //     MovePlayerTowards(player, exitX, exitY);
    // }

    mobj_t* enemy = ai_targeting->Get_Closest_Enemy();
    if (enemy != nullptr)
    {
        PlayerLookAt(player, enemy->x, enemy->y);
    } else
    {
        printf("No Enemy In Sight");
    }
}