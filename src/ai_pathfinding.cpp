#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <memory>
#include <iostream>
#include <bitset>

#include "ai_pathfinding.hpp"
#include "ai_utils.hpp"

extern "C"
{
    #include "r_defs.h"
    #include "r_state.h"
    #include "p_maputl.h"
    #include "r_main.h"
    #include "p_mobj.h"
}

bool operator==(const SearchNode& a, const SearchNode& b)
{
    return a.subsector == b.subsector && a.seg == b.seg &&
        a.opened == b.opened && a.redKey == b.redKey &&
        a.blueKey == b.blueKey && a.yellowKey == b.yellowKey;
}

subsector_t* redKeySubsector = nullptr;
subsector_t* blueKeySubsector = nullptr;
subsector_t* yellowKeySubsector = nullptr;
float redKeyX = 0;
float redKeyY = 0;
float blueKeyX = 0;
float blueKeyY = 0;
float yellowKeyX = 0;
float yellowKeyY = 0;

void InitPathfinding()
{
    for (int i = 0; i < numsectors; i++)
    {
        sector_t sector = sectors[i];
        mobj_t *mobj = sector.thinglist;
        while (mobj != nullptr)
        {
            if (mobj->type == MT_MISC4)
            {
                blueKeySubsector = mobj->subsector;
                blueKeyX = FixedToFloat(mobj->x);
                blueKeyY = FixedToFloat(mobj->y);
            }
            else if (mobj->type == MT_MISC5)
            {
                redKeySubsector = mobj->subsector;
                redKeyX = FixedToFloat(mobj->x);
                redKeyY = FixedToFloat(mobj->y);
            }
            else if (mobj->type == MT_MISC6)
            {
                yellowKeySubsector = mobj->subsector;
                yellowKeyX = FixedToFloat(mobj->x);
                yellowKeyY = FixedToFloat(mobj->y);
            }
            mobj = mobj->snext;
        }
    }
}

std::unordered_set<int16_t> doorActions = {1, 117, 31, 118};
std::unordered_set<int16_t> switchDoorActions = {103};

boolean CheckBlocking(intercept_t* intercept)
{
    mobj_t* mob = intercept->d.thing;
    return (mob->flags & MF_SHOOTABLE && mob->type != MT_BARREL) || !(mob->flags & MF_SOLID);
}

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
    if (blueKeySubsector != nullptr && subsector == blueKeySubsector && !node.blueKey)
    {
        SearchNode newNode = node;
        newNode.seg = nullptr;
        newNode.x = blueKeyX;
        newNode.y = blueKeyY;
        newNode.blueKey = true;
        neighbors.push_back(newNode);
    }
    if (yellowKeySubsector != nullptr && subsector == yellowKeySubsector && !node.yellowKey)
    {
        SearchNode newNode = node;
        newNode.seg = nullptr;
        newNode.x = yellowKeyX;
        newNode.y = yellowKeyY;
        newNode.yellowKey = true;
        neighbors.push_back(newNode);
    }
    for (int32_t i = 0; i < subsector->numlines; i++)
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
                        (node.redKey && (line->special == 28 || line->special == 33)) ||
                        (node.blueKey && (line->special == 26 || line->special == 32)) ||
                        (node.yellowKey && (line->special == 27 || line->special == 34));
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
        float newX = SegMidX(seg);
        float newY = SegMidY(seg);

        if (!P_PathTraverse(FloatToFixed(node.x), FloatToFixed(node.y),
            FloatToFixed(newX), FloatToFixed(newY), PT_ADDTHINGS, CheckBlocking))
        {
            continue;
        }

        SearchNode newNode = node;
        newNode.subsector = other;
        newNode.seg = seg;
        newNode.x = newX;
        newNode.y = newY;
        newNode.door = door;
        neighbors.push_back(newNode);
    }
    return neighbors;
}

PathState PathTowards(player_t* player, float targetX, float targetY, float maxDistance)
{
    subsector_t* targetSubsector = R_PointInSubsector(FloatToFixed(targetX), FloatToFixed(targetY));
    subsector_t* start = GetPlayerSubsector(player);

    if (start == targetSubsector)
    {
        MovePlayerTowards(player, targetX, targetY);
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
        .redKey = (bool)player->cards[it_redcard],
        .blueKey = (bool)player->cards[it_bluecard],
        .yellowKey = (bool)player->cards[it_yellowcard]
    };
    gScore[current] = 0;

    auto heapCompare = [&](SearchNode& a, SearchNode& b)
    {
        float fScoreA = gScore[a] + Distance(targetX, targetY, a.x, a.y);
        float fScoreB = gScore[b] + Distance(targetX, targetY, b.x, b.y);
        return fScoreA > fScoreB;
    };

    nodes.push_back(current);

    while (current.subsector != targetSubsector)
    {
        if (nodes.empty())
        {
            return NO_PATH_FOUND;
        }
        std::ranges::pop_heap(nodes, heapCompare);
        current = nodes.back();
        nodes.pop_back();
        if (gScore.contains(current) && gScore[current] > maxDistance)
        {
            continue;
        }
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

    // std::
    // << "x: " << firstStep.x << ", y: " << firstStep.y << std::endl;
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

std::unordered_map<SearchNode, SearchNode> cameFrom;
std::unordered_map<SearchNode, float> distances;
SearchNode playerStartNode;
void CalcSubsectorDistances(player_t* player)
{
    cameFrom.clear();
    distances.clear();
    subsector_t* start = GetPlayerSubsector(player);

    std::vector<SearchNode> nodes;
    SearchNode current
    {
        .subsector = start,
        .seg = nullptr,
        .opened = {},
        .x = FixedToFloat(player->mo->x),
        .y = FixedToFloat(player->mo->y),
        .door = false,
        .redKey = (bool)player->cards[it_redcard],
        .blueKey = (bool)player->cards[it_bluecard],
        .yellowKey = (bool)player->cards[it_yellowcard]
    };
    distances[current] = 0;

    auto heapCompare = [&](SearchNode& a, SearchNode& b)
    {
        return distances[a] > distances[b];
    };

    nodes.push_back(current);

    while (!nodes.empty())
    {
        std::ranges::pop_heap(nodes, heapCompare);
        current = nodes.back();
        nodes.pop_back();
        if (distances.contains(current) && distances[current] > 1024)
        {
            continue;
        }
        std::vector<SearchNode> neighbors = GetNodeNeighbors(current);
        for (SearchNode neighbor : neighbors)
        {
            float score = distances[current] + Distance(current.x, current.y, neighbor.x, neighbor.y);
            if (distances.contains(neighbor))
            {
                if (score < distances[neighbor])
                {
                    distances[neighbor] = score;
                    cameFrom[neighbor] = current;
                    std::ranges::make_heap(nodes, heapCompare);
                }
            }
            else
            {
                distances[neighbor] = score;
                cameFrom[neighbor] = current;
                nodes.push_back(neighbor);
                std::ranges::push_heap(nodes, heapCompare);
            }
        }
    }
}

PathState PathTowardsNode(player_t* player, SearchNode node)
{
    if (!cameFrom.contains(node))
    {
        return NO_PATH_FOUND;
    }

    bool door = false;
    float doorX = 0;
    float doorY = 0;

    SearchNode current = node;
    SearchNode firstStep = current;

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

    // std::cout << "x: " << firstStep.x << ", y: " << firstStep.y << std::endl;
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