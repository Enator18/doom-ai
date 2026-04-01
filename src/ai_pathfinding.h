#pragma once

enum PathState
{
    NO_PATH_FOUND,
    FOLLOWING_PATH,
    PATH_COMPLETE
};

PathState PathTowards(player_t* player, float targetX, float targetY);
