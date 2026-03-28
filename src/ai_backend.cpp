#include <numbers>
#include <cmath>

#include "ai_backend.h"
#include "p_mobj.h"

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

// The main entry point for all AI logic. Call all AI systems from here.
void AI_Tick(player_t* player)
{
    
}