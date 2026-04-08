#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <limits>

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
    ScanEnemies(player);
    if (ai_targeting == nullptr)
    {
        ai_targeting = new AI_Targeting(player);
    }

    float targetDistance;
    mobj_t* target = ai_targeting->Get_Target_Enemy(targetDistance);

    if (!(player->weaponowned[wp_shotgun] || shotguns.empty()))
    {
        float nearest = std::numeric_limits<float>::infinity();
        float shotgunX = 0;
        float shotgunY = 0;

        for (int i = 0; i < shotguns.size(); i++)
        {
            float distance =
                std::sqrt(std::pow(FixedToFloat(player->mo->x)
                                       - FixedToFloat(shotguns[i]->x),
                                   2)
                          + std::pow(FixedToFloat(player->mo->y)
                                         - FixedToFloat(shotguns[i]->y),
                                     2));

            if (distance < nearest)
            {
                nearest = distance;
                shotgunX = FixedToFloat(shotguns[i]->x);
                shotgunY = FixedToFloat(shotguns[i]->y);
            }
        }

        std::cout << "shotgun at: " << shotgunX << ", " << shotgunY << std::endl;
        PathState state = PathTowards(player, shotgunX, shotgunY, 1024);

        if (state != NO_PATH_FOUND)
        {
            return;
        }
    }

    if (target == nullptr || targetDistance > 1536)
    {
        float exitX = LineMidX(exitLine);
        float exitY = LineMidY(exitLine);
        std::cout << "exit!" << std::endl;
        PathState state = PathTowards(player, exitX, exitY, std::numeric_limits<float>::infinity());
        if (state == PATH_COMPLETE)
        {
            PlayerLookAt(player, exitX, exitY);
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
    // std::cout << ai_targeting->ticks_since_swap << std::endl;
}
