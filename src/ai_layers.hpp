#pragma once

#include <vector>

#include "ai_heuristics.hpp"

// Layer For Changing Weapons
inline static Bezier weapon_change_bezier = Bezier({20.0f, 0.0f}, {20.0f, 1.0f}, {25.0f, 0.75f}, {30.0f, 1.0f});
inline static std::vector<Line*> weapon_change_heuristic_lines = {&weapon_change_bezier};
inline static Heuristic_Layer weapon_change_layer = {weapon_change_heuristic_lines};

// Layer For Shotgun Range
inline static Bezier shotgun_dist_bezier = Bezier({800.0f, 1.0f}, {1100.0f, 0.8f}, {1300.0f, 0.35f}, {1500.0f, 0.0f});
// Lines are auto-sorted based on start x
inline static std::vector<Line*> shotgun_dist_heuristic_lines = {&shotgun_dist_bezier};
inline static Heuristic_Layer shotgun_dist_layer = {shotgun_dist_heuristic_lines};


struct Shooting_Layers
{
    const Heuristic_Layer WEAPON_CHANGE_LAYER = weapon_change_layer;
    const Heuristic_Layer SHOTGUN_DISTANCE_LAYER = shotgun_dist_layer;
};

static const Shooting_Layers shooting_layers;

// FOR DEBUGGING HEURISTICS
// void Debug_Heuristic_Layers() {
//     for (int i = 0; i < 36; i++)
//     {
//         printf("Frame %d: %f\n", i, shooting_layers.WEAPON_CHANGE_LAYER.Get_Current_Val(i));
//     }
//
//     for (int i = 0; i < 1001; i += 50)
//     {
//         printf("Frame %d: %f\n", i, shooting_layers.SHOTGUN_DISTANCE_LAYER.Get_Current_Val(i));
//     }
// }