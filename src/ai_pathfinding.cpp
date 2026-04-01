#include <vector>
#include <algorithm>
#include <functional>

#include "ai_utils.h"
#include "ai_pathfinding.h"

extern "C"
{
    #include "r_defs.h"
    #include "r_state.h"
    #include "p_maputl.h"
    #include "r_main.h"
    #include "p_mobj.h"
}

struct SearchNode
{
    sector_t* sector;
    float x;
    float y;
};

std::vector<SearchNode> GetSectorNeighbors(sector_t* sector)
{
    std::vector<SearchNode> neighbors;
    for (uint32_t i = 0; i < sector->linecount; i++)
    {
        line_t *line = sector->lines[i];
        if (line->flags & ML_BLOCKING)
        {
            continue;
        }

        P_LineOpening(line);
        if (openrange < IntToFixed(56))
        {
            continue;
        }
        if (openbottom - sector->floorheight > IntToFixed(24))
        {
            continue;
        }
        sector_t* other = sides[line->sidenum[sides[line->sidenum[0]].sector == sector]].sector;
        if (other->special == 4 || other->special == 5 || other->special == 7 || other->special == 11)
        {
            continue;
        }
        SearchNode node
        {
            .sector = other,
            .x = LineMidX(line),
            .y = LineMidY(line)
        };
        neighbors.push_back(node);
    }
    return neighbors;
}

PathState PathTowards(player_t* player, float targetX, float targetY)
{
    sector_t* targetSector = R_PointInSubsector(FloatToFixed(targetX), FloatToFixed(targetY))->sector;
    sector_t* start = GetPlayerSector(player);

    if (start == targetSector)
    {
        return PATH_COMPLETE;
    }

    std::unordered_map<sector_t*, float> gScore;
    std::unordered_map<sector_t*, SearchNode> cameFrom;
    std::vector<SearchNode> nodes;
    gScore[start] = 0;
    SearchNode current = {start, FixedToFloat(player->mo->x), FixedToFloat(player->mo->y)};

    auto heapCompare = [&](SearchNode& a, SearchNode& b)
    {
        float fScoreA = gScore[a.sector] + Distance(targetX, targetY, a.x, a.y);
        float fScoreB = gScore[b.sector] + Distance(targetX, targetY, b.x, b.y);
        return fScoreA > fScoreB;
    };

    while (current.sector != targetSector)
    {
        std::vector<SearchNode> neighbors = GetSectorNeighbors(current.sector);
        for (SearchNode neighbor : neighbors)
        {
            float score = gScore[current.sector] + Distance(current.x, current.y, neighbor.x, neighbor.y);
            if (gScore.contains(neighbor.sector))
            {
                if (score < gScore[neighbor.sector])
                {
                    gScore[neighbor.sector] = score;
                    cameFrom[neighbor.sector] = current;
                    std::ranges::make_heap(nodes, heapCompare);
                }
            }
            else
            {
                gScore[neighbor.sector] = score;
                cameFrom[neighbor.sector] = current;
                nodes.push_back(neighbor);
                std::ranges::push_heap(nodes, heapCompare);
            }
        }
        if (nodes.empty())
        {
            return NO_PATH_FOUND;
        }
        std::ranges::pop_heap(nodes, heapCompare);
        current = nodes.back();
        nodes.pop_back();
    }

    SearchNode firstStep = current;

    while (cameFrom.contains(current.sector))
    {
        firstStep = current;
        current = cameFrom[current.sector];
    }

    MovePlayerTowards(player, firstStep.x, firstStep.y);

    return FOLLOWING_PATH;
}
