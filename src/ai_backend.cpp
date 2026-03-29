#include <numbers>
#include <cmath>
#include <vector>
#include <unordered_set>

#include "ai_backend.h"

extern "C"
{
    #include "p_mobj.h"
    #include "r_defs.h"
    #include "p_maputl.h"
    #include "r_state.h"
    #include "d_player.h"
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

struct SectorEdge
{
    sector_t* otherSector;
    float x;
    float y;
};

std::vector<SectorEdge> GetSectorNeighbors(sector_t* sector)
{
    std::vector<SectorEdge> neighbors;
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
        SectorEdge edge
        {
            .otherSector = other,
            .x = (FixedToFloat(line->v1->r_x) + FixedToFloat(line->v2->r_x)) / 2,
            .y = (FixedToFloat(line->v1->r_y) + FixedToFloat(line->v2->r_y)) / 2
        };
        neighbors.push_back(edge);
    }
    return neighbors;
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