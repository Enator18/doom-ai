#pragma once

extern "C"
{
    #include "r_defs.h"
    #include "d_player.h"
}

extern line_t* exitLine;

// Move the player relative to their facing direction
// forward and right should be a normalized direction
void MovePlayerLocal(player_t* player, float forward, float right);

// Move the player relative to the world
// x and y should be a normalized direction
void MovePlayerWorld(player_t* player, float x, float y);

// Move the player towards a position
void MovePlayerTowards(player_t* player, float x, float y);

void PlayerLookAt(player_t* player, float x, float y);

void PlayerShoot(player_t* player);

void SelectPlayerWeapon(player_t* player, weapontype_t weapon);

void PlayerInteract(player_t* player);

// Get the subsector that the player is currently in
subsector_t* GetPlayerSubsector(player_t* player);

float LineMidX(line_t* line);

float LineMidY(line_t* line);

float SegMidX(seg_t* seg);

float SegMidY(seg_t* seg);

float Distance(float x1, float y1, float x2, float y2);

float PlayerDistance(player_t* player, float x, float y);