#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <memory>
#include <iostream>
#include <bitset>

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
    std::bitset<64> opened;
    float x;
    float y;
    bool door;
    bool redKey;
};

bool operator==(const SearchNode& a, const SearchNode& b)
{
    return a.subsector == b.subsector && a.seg == b.seg &&
        a.opened == b.opened && a.redKey == b.redKey;
}

template<>
struct std::hash<SearchNode>
{
    std::size_t operator()(const SearchNode& node) const noexcept
    {
        std::size_t h1 = std::hash<subsector_t*>{}(node.subsector);
        std::size_t h2 = std::hash<seg_t*>{}(node.seg);
        std::size_t h3 = std::hash<std::bitset<64>>{}(node.opened);
        std::size_t h4 = std::hash<bool>{}(node.redKey);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

subsector_t* redKeySubsector = nullptr;
float redKeyX = 0;
float redKeyY = 0;

void InitPathfinding()
{
    for (int i = 0; i < numsectors; i++)
    {
        sector_t sector = sectors[i];
        mobj_t *mobj = sector.thinglist;
        while (mobj != nullptr)
        {
            if (mobj->type == MT_MISC5)
            {
                redKeySubsector = mobj->subsector;
                redKeyX = FixedToFloat(mobj->x);
                redKeyY = FixedToFloat(mobj->y);
                return;
            }
            mobj = mobj->snext;
        }
    }
}

std::unordered_set<int16_t> doorActions = {1, 117, 31, 118};
std::unordered_set<int16_t> switchDoorActions = {103};

std::vector<SearchNode> GetNodeNeighbors(SearchNode& node)
{
    std::vector<SearchNode> neighbors;
    subsector_t* subsector = node.subsector;
    if (redKeySubsector != nullptr && subsector == redKeySubsector && !node.redKey)
    {
        SearchNode newNode = node;
        newNode.seg = nullptr;
        newNode.x = redKeyX;
        newNode.y = redKeyY;
        newNode.redKey = true;
        neighbors.push_back(newNode);
    }
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
                SearchNode newNode = node;
                newNode.seg = seg;
                newNode.x = SegMidX(seg);
                newNode.y = SegMidY(seg);
                newNode.door = true;
                newNode.opened[line->id] = true;
                neighbors.push_back(newNode);
                continue;
            }
            if (line->flags & ML_BLOCKING)
            {
                continue;
            }
            sector_t* otherSector = other->sector;
            bool lift = node.seg != nullptr && node.seg->linedef != nullptr && node.seg->linedef->special == 88;
            if (!(node.door || lift || node.opened[node.subsector->sector->tag]))
            {
                P_LineOpening(line);
                if (openbottom - sector->floorheight > IntToFixed(24))
                {
                    continue;
                }
                if (openrange < IntToFixed(56))
                {
                    bool isDoor = doorActions.contains(line->special) ||
                        (node.redKey && (line->special == 28 || line->special == 33));
                    if (isDoor && line->frontsector == subsector->sector)
                    {
                        door = true;
                    }
                    else if (!node.opened[otherSector->tag])
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
        SearchNode newNode = node;
        newNode.subsector = other;
        newNode.seg = seg;
        newNode.x = SegMidX(seg);
        newNode.y = SegMidY(seg);
        newNode.door = door;
        neighbors.push_back(newNode);
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
        .opened = {},
        .x = FixedToFloat(player->mo->x),
        .y = FixedToFloat(player->mo->y),
        .door = false,
        .redKey = (bool)player->cards[it_redcard]
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
        std::vector<SearchNode> neighbors = GetNodeNeighbors(current);
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
