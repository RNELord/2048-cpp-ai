//
//  montecarlo.cpp
//  2048-cpp
//
//  Created by Ryan Lord on 21/06/15.
//  Copyright (c) 2015 Ryan Lord. All rights reserved.
//

#include "montecarlo.h"

#if AI_ALGORITHM == AI_ALGORITHM_MONTECARLO

float ai_score_move(state_t &state, grid_t grid, int direction)
{
    state.depth_limit = std::max(3, num_distinct_tiles(grid) - 2);
    
    return 0.0f;
}

#endif
