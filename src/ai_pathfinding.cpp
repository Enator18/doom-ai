#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_set>

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
    subsector_t* subsector;
    seg_t* seg;
    float x;
    float y;
    bool door;
};

bool operator==(const SearchNode& a, const SearchNode& b)
{
    return a.subsector == b.subsector && a.seg == b.seg;
}

template<>
struct std::hash<SearchNode>
{
    std::size_t operator()(const SearchNode& node) const noexcept
    {
        std::size_t h1 = std::hash<subsector_t*>{}(node.subsector);
        std::size_t h2 = std::hash<seg_t*>{}(node.seg);
        return h1 ^ (h2 << 1);
    }
};

std::unordered_set<int16_t> doorActions = {1, 117, 31, 118};

std::vector<SearchNode> GetSubsectorNeighbors(subsector_t* subsector)
{
    std::vector<SearchNode> neighbors;
    for (uint32_t i = 0; i < subsector->numlines; i++)
    {
        seg_t* seg = &segs[subsector->firstline + i];
        if (seg->partner == nullptr)
        {
            continue;
        }
        subsector_t* other = seg->partner->subsector;
        bool door = false;
        line_t* line = seg->linedef;
        if (line != nullptr)
        {
            sector_t* sector = subsector->sector;
            if (line->flags & ML_BLOCKING)
            {
                continue;
            }
            P_LineOpening(line);
            if (openbottom - sector->floorheight > IntToFixed(24))
            {
                continue;
            }
            if (openrange < IntToFixed(56))
            {
                if (doorActions.contains(line->special))
                {
                    door = true;
                }
                else
                {
                    continue;
                }
            }
            sector_t* otherSector = other->sector;
            if (otherSector->special == 4 || otherSector->special == 5 || otherSector->special == 7 || otherSector->special == 11)
            {
                continue;
            }
        }
        SearchNode node
        {
            .subsector = other,
            .seg = seg,
            .x = SegMidX(seg),
            .y = SegMidY(seg),
            .door = door
        };
        neighbors.push_back(node);
    }
    return neighbors;
}

PathState PathTowards(player_t* player, float targetX, float targetY)
{
    subsector_t* targetSubsector = R_PointInSubsector(FloatToFixed(targetX), FloatToFixed(targetY));
    subsector_t* start = GetPlayerSubsector(player);

    if (start == targetSubsector)
    {
        return PATH_COMPLETE;
    }

    std::unordered_map<SearchNode, float> gScore;
    std::unordered_map<SearchNode, SearchNode> cameFrom;
    std::vector<SearchNode> nodes;
    SearchNode current = {start, nullptr, FixedToFloat(player->mo->x), FixedToFloat(player->mo->y)};
    gScore[current] = 0;

    auto heapCompare = [&](SearchNode& a, SearchNode& b)
    {
        float fScoreA = gScore[a] + Distance(targetX, targetY, a.x, a.y);
        float fScoreB = gScore[b] + Distance(targetX, targetY, b.x, b.y);
        return fScoreA > fScoreB;
    };

    while (current.subsector != targetSubsector)
    {
        std::vector<SearchNode> neighbors = GetSubsectorNeighbors(current.subsector);
        for (SearchNode neighbor : neighbors)
        {
            float score = gScore[current] + Distance(current.x, current.y, neighbor.x, neighbor.y);
            if (gScore.contains(neighbor))
            {
                if (score < gScore[neighbor])
                {
                    gScore[neighbor] = score;
                    cameFrom[neighbor] = current;
                    std::ranges::make_heap(nodes, heapCompare);
                }
            }
            else
            {
                gScore[neighbor] = score;
                cameFrom[neighbor] = current;
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

    bool door = false;
    float doorX = 0;
    float doorY = 0;

    while (cameFrom.contains(current))
    {
        if (current.door)
        {
            door = true;
            doorX = current.x;
            doorY = current.y;
        }
        firstStep = current;
        current = cameFrom[current];
    }

    MovePlayerTowards(player, firstStep.x, firstStep.y);

    if (door)
    {
        float doorDist = Distance(FixedToFloat(player->mo->x), FixedToFloat(player->mo->y), doorX, doorY);
        if (doorDist < 72)
        {
            PlayerLookAt(player, doorX, doorY);
            if (doorDist < 62)
            {
                PlayerInteract(player);
            }
        }
    }

    return FOLLOWING_PATH;
}
