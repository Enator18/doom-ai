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

bool LineOfSight(float x, float y, mobj_t* enemy)
{
    //
    // check for trivial rejection
    //
    int32_t s1 = (enemy->subsector->sector - sectors);
    int32_t s2 = R_PointInSubsector(FloatToFixed(x), FloatToFixed(y))->sector - sectors;
    int32_t pnum = s1*numsectors + s2;
    int32_t bytenum = pnum>>3;
    int32_t bitnum = 1 << (pnum&7);

    if (rejectmatrix[bytenum]&bitnum)
    {
        return false;    // can't possibly be connected
    }

    return P_SightPathTraverse (enemy->x, enemy->y, FloatToFixed(x), FloatToFixed(y));
}

float EvaluatePosition(float x, float y, std::vector<mobj_t*>& enemies, float pathDistance, mobj_t* target)
{
    float score = 0;

    for (mobj_t* enemy : enemies)
    {
        if (enemy == target)
        {
            continue;
        }
        float enemyDistance = Distance(x, y, FixedToFloat(enemy->x), FixedToFloat(enemy->y));
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
        if (!LineOfSight(x, y, enemy))
        {
            distScore *= losMult;
        }
        score += distScore;
    }

    float targetDistance = Distance(x, y, FixedToFloat(target->x), FixedToFloat(target->y));

    score /= TARGET_DISTANCE_POS_LAYER.Get_Current_Val(targetDistance);
    score /= PATH_DISTANCE_POS_LAYER.Get_Current_Val(pathDistance);

    return score;
}

void PlayerCombatMove(player_t* player, AI_Targeting* targeting, mobj_t* target)
{
    CalcSubsectorDistances(player);

    float min = std::numeric_limits<float>::infinity();
    SearchNode minNode{};

    std::vector<mobj_t*> enemies = targeting->Get_Enemies(player);

    for (std::pair<const SearchNode, float> pair : distances)
    {
        const SearchNode& node = pair.first;
        float score = EvaluatePosition(node.x, node.y,
            enemies, pair.second, target);
        if (score < min)
        {
            min = score;
            minNode = node;
        }
    }

    PathTowardsNode(player, minNode);
}