#include <numbers>
#include <cmath>

#include "ai_backend.h"
#include "p_mobj.h"

// Move the player relative to their facing direction
// forward and right range from -1 to 1
void MovePlayerLocal(player_t* player, float forward, float right)
{
    player->cmd.forwardmove = forward * 50;
    player->cmd.sidemove = right * 50;
}

// Move the player relative to the world
// x and y range from -1 to 1
void MovePlayerWorld(player_t* player, float x, float y)
{
    if (player->mo != nullptr)
    {
        float angle = (float)player->mo->angle / ANG180 * std::numbers::pi;
        float forward = x * cos(angle) + y * sin(angle);
        float right = x * sin(angle) - y * cos(angle);
        MovePlayerLocal(player, forward, right);
    }
}

// The main entry point for all AI logic. Call all AI systems from here.
void AI_Tick(player_t* player)
{

}