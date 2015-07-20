//
//  expectimax.cpp
//  2048-cpp
//
//  Created by Ryan Lord on 18/06/15.
//  Copyright (c) 2015 Ryan Lord. All rights reserved.
//

#include "expectimax.h"

#if AI_ALGORITHM == AI_ALGORITHM_EXPECTIMAX

static float score_move_state(state_t &state, grid_t grid, float probability);

static float score_choice(state_t &state, grid_t grid, float probability)
{
    if (probability < PROBABILITY_CUTOFF || state.current_depth >= state.depth_limit) {
        state.max_depth = std::max(state.max_depth, state.current_depth);
        return score_heuristic(grid);
    }
    
    if (state.current_depth < CACHE_DEPTH_LIMIT) {
        const trans_table_t::iterator &i = state.cache.find(grid);
        if (i != state.cache.end()) {
            trans_table_entry_t entry = i->second;
            
            if (entry.depth <= state.current_depth) {
                state.cache_hits++;
                return entry.heuristic;
            }
        }
    }
    
    int empty_tiles = num_empty_tiles(grid);
    probability /= empty_tiles;
    
    float score = 0.0f;
    grid_t tmp = grid;
    grid_t tile_2 = 1;
    while (tile_2) {
        if ((tmp & 0xf) == 0) {
            score += score_move_state(state, grid | tile_2       , probability * PROBABILITY_OF_2_TILE) * PROBABILITY_OF_2_TILE; // 2 tile
            score += score_move_state(state, grid | (tile_2 << 1), probability *PROBABILITY_OF_4_TILE) * PROBABILITY_OF_4_TILE;
        }
        tmp >>= SIZEOF_TILE; // shift off a tile
        tile_2 <<= SIZEOF_TILE; // move from 1->2 (2 tile to 4 tile)
    }
    score = score / empty_tiles;
    if (state.current_depth < CACHE_DEPTH_LIMIT) {
        trans_table_entry_t entry = {static_cast<uint8_t>(state.current_depth), score};
        state.cache[grid] = entry;
    }

    return score;
}

static float score_move_state(state_t &state, grid_t grid, float probability)
{
    int direction;
    float best_score = 0.0f;
    grid_t new_grid;
    
    state.current_depth++;
    
    for (direction = 0; direction < 4; ++direction) {
        new_grid = grid_move(direction, grid);
        state.evaluated++;
        
        if (grid != new_grid) {
            // this was a valid move
            best_score = std::max(best_score, score_choice(state, new_grid, probability));
        }
    }
    
    state.current_depth--;
    
    return best_score;
}

float ai_score_move(state_t &state, grid_t grid, int direction)
{
    state.depth_limit = std::max(3, num_distinct_tiles(grid) - 2);
    grid_t new_grid = grid_move(direction, grid);
    
    if (new_grid == grid) {
        return 0.0f; // move not viable
    }
    
    return score_choice(state, new_grid, 1.0f) + 1e-6;
}

#endif
