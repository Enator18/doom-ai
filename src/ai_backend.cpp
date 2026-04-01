#include <numbers>
#include <cmath>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <functional>

#include "ai_backend.h"

extern "C"
{
    #include "p_mobj.h"
    #include "r_defs.h"
    #include "p_maputl.h"
    #include "r_state.h"
    #include "d_player.h"
    #include "r_main.h"
}

line_t* exitLine;

// Move the player relative to their facing direction
// forward and right should be a normalized direction
void MovePlayerLocal(player_t* player, float forward, float right)
{
    player->cmd.forwardmove = forward * 50;
    player->cmd.sidemove = right * 50;
}

// Move the player relative to the world
// x and y should be a normalized direction
void MovePlayerWorld(player_t* player, float x, float y)
{
    float angle = (float)player->mo->angle / ANG180 * std::numbers::pi;
    float forward = x * cos(angle) + y * sin(angle);
    float right = x * sin(angle) - y * cos(angle);
    MovePlayerLocal(player, forward, right);
}

// Move the player towards a position
void MovePlayerTowards(player_t* player, float x, float y)
{
    float playerX = FixedToFloat(player->mo->x);
    float playerY = FixedToFloat(player->mo->y);
    float distX = x - playerX;
    float distY = y - playerY;
    float distance = sqrt(distX * distX + distY * distY);
    if (distance > 0)
    {
        distX /= distance;
        distY /= distance;
    }
    MovePlayerWorld(player, distX, distY);
}

// Get the sector that the player is currently in
sector_t* GetPlayerSector(player_t* player)
{
    return player->mo->subsector->sector;
}



struct SearchNode
{
    sector_t* sector;
    float x;
    float y;
};

float LineMidX(line_t* line)
{
    return (FixedToFloat(line->v1->r_x) + FixedToFloat(line->v2->r_x)) / 2;
}

float LineMidY(line_t* line)
{
    return (FixedToFloat(line->v1->r_y) + FixedToFloat(line->v2->r_y)) / 2;
}

std::vector<SearchNode> GetSectorNeighbors(sector_t* sector)
{
    std::vector<SearchNode> neighbors;
    for (uint32_t i = 0; i < sector->linecount; i++)
    {
        line_t *line = sector->lines[i];

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
            .x = (FixedToFloat(line->v1->r_x) + FixedToFloat(line->v2->r_x)) / 2,
            .y = (FixedToFloat(line->v1->r_y) + FixedToFloat(line->v2->r_y)) / 2
        };
        neighbors.push_back(node);
    }
    return neighbors;
}

float Distance(float x1, float y1, float x2, float y2)
{
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

bool PathTowards(player_t* player, float targetX, float targetY)
{
    sector_t* targetSector = R_PointInSubsector(FloatToFixed(targetX), FloatToFixed(targetY))->sector;
    sector_t* start = GetPlayerSector(player);
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
            return false;
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

    return true;
}

std::unordered_set<int16_t> exitSpecials = {11, 51, 52, 124, 197, 198};

// Called after a new level is loaded.
void AI_Init()
{
    for (uint32_t i = 0; i < numlines; i++)
    {
        line_t& line = lines[i];
        if (exitSpecials.contains(line.special))
        {
            exitLine = &line;
            break;
        }
    }
}

// The main entry point for all AI logic. Called every tick.
// Call all AI systems from here.
void AI_Tick(player_t* player)
{
    
}