#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <set>
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
    #include "m_bbox.h"
}

struct SearchNode
{
    subsector_t* subsector;
    line_t* line;
    float x;
    float y;
    bool door;
};

bool operator==(const SearchNode& a, const SearchNode& b)
{
    return a.subsector == b.subsector && a.line == b.line;
}

template<>
struct std::hash<SearchNode>
{
    std::size_t operator()(const SearchNode& node) const noexcept
    {
        std::size_t h1 = std::hash<subsector_t*>{}(node.subsector);
        std::size_t h2 = std::hash<line_t*>{}(node.line);
        return h1 ^ (h2 << 1);
    }
};

std::unordered_map<subsector_t*, std::vector<SearchNode>> subNeighbors;

bool crossedLine;

boolean CheckIntercept(intercept_t* intercept)
{
    if (!(crossedLine || intercept->d.line->flags & ML_BLOCKING))
    {
        crossedLine = true;
        return true;
    }
    return false;
}

bool CheckSubsectors(SearchNode a, SearchNode b)
{
    fixed_t aX = FloatToFixed(a.x);
    fixed_t aY = FloatToFixed(a.y);
    fixed_t bX = FloatToFixed(b.x);
    fixed_t bY = FloatToFixed(b.y);
    crossedLine = false;
    return P_PathTraverse(aX, aY, bX, bY, PT_ADDLINES, CheckIntercept);
}

std::unordered_map<sector_t*, std::vector<SearchNode>> sortedSubsectors;
std::unordered_map<line_t*, std::vector<SearchNode>> lineSubsectors;

void SortSubsectors(uint32_t nodeNum, fixed_t* bounds, float divX, float divY)
{
    while (!(nodeNum & NF_SUBSECTOR))
    {
        node_t* node = &nodes[nodeNum];
        divX = FixedToFloat(node->x) + FixedToFloat(node->dx) * 0.5;
        divY = FixedToFloat(node->y) + FixedToFloat(node->dy) * 0.5;

        SortSubsectors(node->children[0], node->bbox[0], divX, divY);
        nodeNum = node->children[1];
        bounds = node->bbox[1];
    }
    if (nodeNum == -1)
    {
        return;
    }

    subsector_t* subsector = &subsectors[nodeNum & ~NF_SUBSECTOR];
    // float x = (FixedToFloat(bounds[BOXLEFT]) + FixedToFloat(bounds[BOXRIGHT])) * 0.5;
    // float y = (FixedToFloat(bounds[BOXTOP]) + FixedToFloat(bounds[BOXBOTTOM])) * 0.5;
    float x = 0;
    float y = 0;
    for (uint32_t i = 0; i < subsector->numlines; i++)
    {
        seg_t* seg = &segs[subsector->firstline + i];
        x += FixedToFloat(seg->v1->x);
        y += FixedToFloat(seg->v1->y);
        x += FixedToFloat(seg->v2->x);
        y += FixedToFloat(seg->v2->y);
    }
    x /= subsector->numlines * 2;
    y /= subsector->numlines * 2;
    if (R_PointInSubsector(FloatToFixed(x), FloatToFixed(y)) != subsector)
    {
        std::cout << "subsector: " << subsector - subsectors << ", x: " << x << ", y: " << y << std::endl;
    }

    sortedSubsectors[subsector->sector].push_back({subsector, nullptr, x, y, false});
    for (uint32_t i = 0; i < subsector->numlines; i++)
    {
        line_t* line = segs[subsector->firstline + i].linedef;
        lineSubsectors[line].push_back({subsector, nullptr, x, y});
    }
}

void CalcSubsectorVisibility(sector_t* sector)
{
    std::vector<SearchNode>& sectorChildren = sortedSubsectors[sector];
    for (uint32_t i1 = 0; i1 < sectorChildren.size(); i1++)
    {
        for (uint32_t i2 = i1 + 1; i2 < sectorChildren.size(); i2++)
        {
            SearchNode a = sectorChildren[i1];
            SearchNode b = sectorChildren[i2];
            if (!CheckSubsectors(a, b))
            {
                continue;
            }

            subNeighbors[a.subsector].push_back(b);
            subNeighbors[b.subsector].push_back(a);
        }
    }
}

void AddLineNeighbors(line_t* line)
{
    std::vector<SearchNode> lineNeighbors = lineSubsectors[line];
    for (uint32_t i1 = 0; i1 < lineNeighbors.size(); i1++)
    {
        for (uint32_t i2 = i1 + 1; i2 < lineNeighbors.size(); i2++)
        {
            SearchNode a = lineNeighbors[i1];
            SearchNode b = lineNeighbors[i2];
            if (a.subsector->sector == b.subsector->sector)
            {
                continue;
            }

            a.line = line;
            b.line = line;
            subNeighbors[a.subsector].push_back(b);
            subNeighbors[b.subsector].push_back(a);
        }
    }
}

void CalcSubsectorNeighbors()
{
    SortSubsectors(numnodes - 1, nullptr, 0, 0);

    for (uint32_t i = 0; i < numsectors; i++)
    {
        CalcSubsectorVisibility(&sectors[i]);
    }
    for (uint32_t i = 0; i < numlines; i++)
    {
        line_t* line = &lines[i];
        if (line->flags & ML_TWOSIDED && !(line->flags & ML_BLOCKING))
        {
            AddLineNeighbors(line);
        }
    }

    sortedSubsectors.clear();
    lineSubsectors.clear();
}

std::unordered_set<int16_t> doorActions = {1, 117, 31, 118, 26, 27, 28, 32, 33, 34};
std::unordered_set<int16_t> damageSpecials = {4, 5, 7, 16};

std::vector<SearchNode> GetSubsectorNeighbors(subsector_t* subsector)
{
    std::vector<SearchNode>& potential = subNeighbors[subsector];
    std::vector<SearchNode> neighbors;
    sector_t* sector = subsector->sector;
    for (SearchNode node : potential)
    {
        line_t *line = node.line;
        if (line == nullptr)
        {
            neighbors.push_back(node);
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
                node.door = true;
            }
            else
            {
                continue;
            }
        }
        sector_t* other = node.subsector->sector;
        if (damageSpecials.contains(other->special))
        {
            continue;
        }
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

    std::cout << "x: " << firstStep.x << ", y: " << firstStep.y << std::endl;
    MovePlayerTowards(player, firstStep.x, firstStep.y);

    if (door)
    {
        float doorDist = PlayerDistance(player, doorX, doorY);
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
