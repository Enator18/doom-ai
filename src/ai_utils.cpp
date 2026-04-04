#include <numbers>
#include <cmath>
#include <vector>
#include <unordered_map>

#include "ai_utils.h"

extern "C"
{
    #include "p_mobj.h"
    #include "r_defs.h"
    #include "p_maputl.h"
    #include "d_player.h"
    #include "d_event.h"
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

void PlayerLookAt(player_t* player, float x, float y)
{
    float dx = x - FixedToFloat(player->mo->x);
    float dy = y - FixedToFloat(player->mo->y);
    float targetAngleRad = atan2(dy, dx);
    int32_t targetAngleSigned = targetAngleRad * (float) ANG180  / std::numbers::pi;
    angle_t targetAngle = targetAngleSigned;
    angle_t offset = targetAngle - player->mo->angle;
    angle_t ticoffset = targetAngle - player->ticangle;
    player->cmd.angleturn = offset >> 16;
    player->cmd.ticangleturn = ticoffset >> 16;
}

void PlayerShoot(player_t* player)
{
    player->cmd.buttons |= BT_ATTACK;
}

void SelectPlayerWeapon(player_t* player, weapontype_t weapon)
{
    player->cmd.buttons |= BT_CHANGE;
    player->cmd.buttons |= weapon << BT_WEAPONSHIFT;
}

bool useToggle = false;

void PlayerInteract(player_t* player)
{
    useToggle = !useToggle;
    if (useToggle)
    {
        player->cmd.buttons |= BT_USE;
    }
}

// Get the subsector that the player is currently in
subsector_t* GetPlayerSubsector(player_t* player)
{
    return player->mo->subsector;
}

float LineMidX(line_t* line)
{
    return (FixedToFloat(line->v1->r_x) + FixedToFloat(line->v2->r_x)) / 2;
}

float LineMidY(line_t* line)
{
    return (FixedToFloat(line->v1->r_y) + FixedToFloat(line->v2->r_y)) / 2;
}

float SegMidX(seg_t* seg)
{
    return (FixedToFloat(seg->v1->r_x) + FixedToFloat(seg->v2->r_x)) / 2;
}

float SegMidY(seg_t* seg)
{
    return (FixedToFloat(seg->v1->r_y) + FixedToFloat(seg->v2->r_y)) / 2;
}

float Distance(float x1, float y1, float x2, float y2)
{
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

float PlayerDistance(player_t* player, float x, float y)
{
    return Distance(FixedToFloat(player->mo->x), FixedToFloat(player->mo->y), x, y);
}
