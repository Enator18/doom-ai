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

float EvaluatePosition(float x, float y, std::vector<mobj_t*>& enemies, float pathDistance)
{
    float score = 0;

    for (mobj_t* enemy : enemies)
    {
        float enemyDistance = Distance(x, y, FixedToFloat(enemy->x), FixedToFloat(enemy->y));
        bool lineOfSight = LineOfSight(x, y, enemy);
        switch (enemy->type)
        {
            case MT_POSSESSED:
                score += ZOMBIE_DISTANCE_POS_LAYER.Get_Current_Val(enemyDistance);
                break;
            case MT_SHOTGUY:
                score += SHOTGUNNER_DISTANCE_POS_LAYER.Get_Current_Val(enemyDistance);
                break;
            case MT_TROOP:
                score += IMP_DISTANCE_POS_LAYER.Get_Current_Val(enemyDistance);
                break;
            default:
                break;
        }
    }

    return score;
}

void PlayerCombatMove(player_t* player, AI_Targeting* targeting)
{
    CalcSubsectorDistances(player);

    float min = std::numeric_limits<float>::infinity();
    SearchNode minNode{};

    std::vector<mobj_t*> enemies = targeting->Get_Enemies(player);

    for (std::pair<const SearchNode, float> pair : distances)
    {
        const SearchNode& node = pair.first;
        float score = EvaluatePosition(SegMidX(node.seg), SegMidY(node.seg),
            enemies, pair.second);
        if (score < min)
        {
            min = score;
            minNode = node;
        }
    }

    PathTowardsNode(player, minNode);
}