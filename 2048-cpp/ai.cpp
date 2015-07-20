//
//  ai.cpp
//  2048-cpp
//
//  Created by Ryan Lord on 18/06/15.
//  Copyright (c) 2015 Ryan Lord. All rights reserved.
//

#include "ai.h"
#include <iostream>
#include <algorithm>

#if AI_ALGORITHM == AI_ALGORITHM_EXPECTIMAX
#include "expectimax.h"
#elif AI_ALGORITHM == AI_ALGORITHM_MONTECARLO
#include "montecarlo.h"
#elif AI_ALGORITHM == AI_ALGORITHM_ALPHABETA
#include "alphabeta.h"
#else
#error Unknown AI algorithm
// any algorithms should implement this function
// float ai_score_move(state_t state, grid_t grid, int direction);
#endif

float score_move(grid_t grid, int direction)
{    
    state_t state;
    
    float score = ai_score_move(state, grid, direction);
    std::printf("Direction: %5s | max depth: %d\tscore: %.3lf\tevaluated: %d\tcache hits: %d", GRID_MOVES[direction], state.max_depth, score, state.evaluated, state.cache_hits);
    std::cout << std::endl;
    return score;
}

int find_best_move(grid_t grid)
{
    int direction, best_direction = -1;
    float best_score = 0.0f, score;
    
    print_grid(grid);
    std::printf("Current scores   | real: %.0f, heuristic: %.3lf", score_real(grid), score_heuristic(grid));
    std::cout << std::endl;
    
    for (direction = 0; direction < 4; direction++) {
        score = score_move(grid, direction);
        
        if (score > best_score) {
            best_score = score;
            best_direction = direction;
        } else if (score == best_score && best_score > 0.0f) {
            // coin flip to break a tie
            if (std::floor(rand() % 2) == 2) {
                best_direction = direction;
            }
        }
    }
    
    if (best_direction != -1) {
        std::printf("Move %s", GRID_MOVES[best_direction]);
        std::cout << std::endl;
    } else {
        std::cout << "No move found" << std::endl;
    }
    
    return best_direction;
}