#include <unordered_map>
#include <limits>

#include "ai_heuristics.hpp"
#include "ai_layers.hpp"
#include "ai_pathfinding.h"
#include "ai_positioning.h"
#include "ai_targeting.hpp"
#include "ai_utils.h"

extern "C"
{
    #include "p_maputl.h"
    #include "r_state.h"
    #include "r_main.h"
    #include "p_setup.h"
}

std::unordered_map<SearchNode, float> nodeScores;

float EvaluatePosition(const SearchNode& node, std::vector<mobj_t*>& enemies, float pathDistance, mobj_t* target)
{
    float score = 0;

    for (mobj_t* enemy : enemies)
    {
        if (enemy == target)
        {
            continue;
        }
        float enemyDistance = Distance(node.x, node.y, FixedToFloat(enemy->x), FixedToFloat(enemy->y));
        float distScore = 0;
        float losMult = 0;
        switch (enemy->type)
        {
            case MT_POSSESSED:
                distScore = ZOMBIE_DISTANCE_POS_LAYER.Get_Current_Val(enemyDistance);
                losMult = 0.25f;
                break;
            case MT_SHOTGUY:
                distScore = SHOTGUNNER_DISTANCE_POS_LAYER.Get_Current_Val(enemyDistance);
                losMult = 0.25f;
                break;
            case MT_TROOP:
                distScore = IMP_DISTANCE_POS_LAYER.Get_Current_Val(enemyDistance);
                losMult = 0.75f;
                break;
            default:
                break;
        }
        if (!P_CheckSight_Pos(enemy, FloatToFixed(node.x), FloatToFixed(node.y), IntToFixed(56), node.subsector->sector))
        {
            distScore *= losMult;
        }
        score += distScore;
    }

    for (mobj_t* projectile : projectiles)
    {
        float projectileDistance = Distance(node.x, node.y, FixedToFloat(projectile->x), FixedToFloat(projectile->y));
        score += FIRE_DISTANCE_POS_LAYER.Get_Current_Val(projectileDistance);
    }

    for (mobj_t* barrel : barrels)
    {
        float barrelDistance = Distance(node.x, node.y, FixedToFloat(barrel->x), FixedToFloat(barrel->y));
        score += BARREL_DISTANCE_POS_LAYER.Get_Current_Val(barrelDistance);
    }

    float targetDistance = Distance(node.x, node.y, FixedToFloat(target->x), FixedToFloat(target->y));

    score /= TARGET_DISTANCE_POS_LAYER.Get_Current_Val(targetDistance);
    score /= PATH_DISTANCE_POS_LAYER.Get_Current_Val(pathDistance);

    return score;
}

void PlayerCombatMove(player_t* player, AI_Targeting* targeting, mobj_t* target)
{
    CalcSubsectorDistances(player);

    float min = std::numeric_limits<float>::infinity();
    SearchNode minNode{};

    for (std::pair<const SearchNode, float> pair : distances)
    {
        float score = EvaluatePosition(pair.first, enemies, pair.second, target);
        if (score < min)
        {
            min = score;
            minNode = pair.first;
        }
    }

    PathTowardsNode(player, minNode);
}