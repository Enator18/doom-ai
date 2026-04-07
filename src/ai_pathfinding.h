#pragma once

#include <bitset>

extern "C"
{
    #include "r_defs.h"
    #include "d_player.h"
}

enum PathState
{
    NO_PATH_FOUND,
    FOLLOWING_PATH,
    PATH_COMPLETE
};

void InitPathfinding();

PathState PathTowards(player_t* player, float targetX, float targetY);

struct SearchNode
{
    subsector_t* subsector;
    seg_t* seg;
    std::bitset<64> opened;
    float x;
    float y;
    bool door;
    bool redKey;
    bool blueKey;
    bool yellowKey;
};

bool operator==(const SearchNode& a, const SearchNode& b);

template<>
struct std::hash<SearchNode>
{
    std::size_t operator()(const SearchNode& node) const noexcept
    {
        std::size_t h1 = std::hash<subsector_t*>{}(node.subsector);
        std::size_t h2 = std::hash<seg_t*>{}(node.seg);
        std::size_t h3 = std::hash<std::bitset<64>>{}(node.opened);
        std::size_t h4 = std::hash<uint8_t>{}(node.redKey + (node.yellowKey << 1) + (node.blueKey << 2));
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

extern std::unordered_map<SearchNode, float> distances;
void CalcSubsectorDistances(player_t* player);

PathState PathTowardsNode(player_t* player, SearchNode node);
