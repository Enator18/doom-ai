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
    sector_t* sector;
    line_t* line;
    float x;
    float y;
    bool door;
};

bool operator==(const SearchNode& a, const SearchNode& b)
{
    return a.sector == b.sector && a.line == b.line;
}

template<>
struct std::hash<SearchNode>
{
    std::size_t operator()(const SearchNode& node) const noexcept
    {
        std::size_t h1 = std::hash<sector_t*>{}(node.sector);
        std::size_t h2 = std::hash<line_t*>{}(node.line);
        return h1 ^ (h2 << 1);
    }
};

std::unordered_set<int16_t> doorActions = {1, 117, 31, 118};

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
        if (openbottom - sector->floorheight > IntToFixed(24))
        {
            continue;
        }
        bool door = false;
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
        sector_t* other = sides[line->sidenum[sides[line->sidenum[0]].sector == sector]].sector;
        if (other->special == 4 || other->special == 5 || other->special == 7 || other->special == 11)
        {
            continue;
        }
        SearchNode node
        {
            .sector = other,
            .line = line,
            .x = LineMidX(line),
            .y = LineMidY(line),
            .door = door
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

    while (current.sector != targetSector)
    {
        std::vector<SearchNode> neighbors = GetSectorNeighbors(current.sector);
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
