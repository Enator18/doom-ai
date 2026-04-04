#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <memory>
#include <iostream>

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
    std::shared_ptr<std::unordered_set<int16_t>> opened;
    float x;
    float y;
    bool door;
    bool redKey;
};

bool operator==(const SearchNode& a, const SearchNode& b)
{
    return a.subsector == b.subsector && a.seg == b.seg && a.opened == b.opened && a.redKey == b.redKey;
}

template<>
struct std::hash<SearchNode>
{
    std::size_t operator()(const SearchNode& node) const noexcept
    {
        std::size_t h1 = std::hash<subsector_t*>{}(node.subsector);
        std::size_t h2 = std::hash<seg_t*>{}(node.seg);
        std::size_t h3 = std::hash<std::shared_ptr<std::unordered_set<int16_t>>>{}(node.opened);
        std::size_t h4 = std::hash<bool>{}(node.redKey);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

std::unordered_set<int16_t> doorActions = {1, 117, 31, 118};
std::unordered_set<int16_t> switchDoorActions = {103};

std::vector<SearchNode> GetSubsectorNeighbors(subsector_t* subsector, bool checkHeight,
    std::shared_ptr<std::unordered_set<int16_t>> opened, bool redKey)
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
            if (switchDoorActions.contains(line->special))
            {
                SearchNode node
                {
                    .subsector = subsector,
                    .seg = seg,
                    .x = SegMidX(seg),
                    .y = SegMidY(seg),
                    .door = true,
                    .opened = std::make_shared<std::unordered_set<int16_t>>(*opened)
                };
                node.opened->insert(line->id);
                neighbors.push_back(node);
                continue;
            }
            if (line->flags & ML_BLOCKING)
            {
                continue;
            }
            sector_t* otherSector = other->sector;
            if (checkHeight)
            {
                P_LineOpening(line);
                if (openbottom - sector->floorheight > IntToFixed(24))
                {
                    continue;
                }
                if (openrange < IntToFixed(56))
                {
                    if (doorActions.contains(line->special) && line->frontsector == subsector->sector)
                    {
                        door = true;
                    }
                    else if (!opened->contains(otherSector->tag))
                    {
                        continue;
                    }
                }
            }
            if (otherSector->special == 4 || otherSector->special == 5 || otherSector->special == 7 || otherSector->special == 11)
            {
                continue;
            }
        }
        SearchNode node
        {
            .subsector = other,
            .seg = seg,
            .opened = opened,
            .x = SegMidX(seg),
            .y = SegMidY(seg),
            .door = door,
            .redKey = redKey
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
    SearchNode current
    {
        .subsector = start,
        .seg = nullptr,
        .opened = std::make_shared<std::unordered_set<int16_t>>(),
        .x = FixedToFloat(player->mo->x),
        .y = FixedToFloat(player->mo->y),
        .door = false,
        .redKey = false
    };
    gScore[current] = 0;

    auto heapCompare = [&](SearchNode& a, SearchNode& b)
    {
        float fScoreA = gScore[a] + Distance(targetX, targetY, a.x, a.y);
        float fScoreB = gScore[b] + Distance(targetX, targetY, b.x, b.y);
        return fScoreA > fScoreB;
    };

    while (current.subsector != targetSubsector)
    {
        bool lift = current.seg != nullptr && current.seg->linedef != nullptr && current.seg->linedef->special == 88;
        bool checkHeight = !(current.door || lift || current.opened->contains(current.subsector->sector->tag));
        std::vector<SearchNode> neighbors = GetSubsectorNeighbors(current.subsector, checkHeight, current.opened, current.redKey);
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
        if (current.door && current.subsector->sector->ceilingdata == nullptr)
        {
            door = true;
            doorX = current.x;
            doorY = current.y;
        }
        firstStep = current;
        current = cameFrom[current];
    }

    std::cout << "x: " << firstStep.x << ", y: " << firstStep.y << std::endl;
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
