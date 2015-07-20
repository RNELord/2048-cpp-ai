//
//  alphabeta.cpp
//  2048-cpp
//
//  Created by Ryan Lord on 21/06/15.
//  Copyright (c) 2015 Ryan Lord. All rights reserved.
//

#include "alphabeta.h"

#if AI_ALGORITHM == AI_ALGORITHM_ALPHABETA

float alphabeta(state_t &state, grid_t grid, float alpha, float beta, bool isPlayerTurn)
{
    if (state.current_depth == state.depth_limit) {
        state.max_depth = std::max(state.current_depth, state.max_depth);
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
    
    state.current_depth++;
    
    grid_t tmp;
    float new_score;
    float best_score = 0.0f;
    
    if (isPlayerTurn) {
        int dir;
        
        best_score = alpha;
        
        for (dir = 0; dir < 4; dir++) {
            tmp = grid_move(dir, grid);
            state.evaluated++;
            
            if (tmp != grid) {
                new_score = alphabeta(state, tmp, score_heuristic(tmp), beta, false);
            
                if (new_score > best_score) {
                    best_score = new_score;
                }
            
                if (best_score > beta) {
                    break;
                }
            }
        }
    } else {
        best_score = beta;
        
        grid_t tmp2;
        float score_2, score_4;
        
        tmp = grid;
        grid_t tile_2 = 1;
        while (tile_2) {
            if ((tmp & 0xf) == 0) {
                tmp2 = grid | tile_2;
                score_2 = alphabeta(state, tmp2, alpha, score_heuristic(tmp2), true);
                
                tmp2 = grid | (tile_2 << 1);
                score_4 = alphabeta(state, tmp2, alpha, score_heuristic(tmp2), true);
                
                new_score = std::min(score_2, score_4);
                
                if (new_score < best_score) {
                    best_score = new_score;
                }
                if (best_score < alpha) {
                    break;
                }
            }
            tmp >>= SIZEOF_TILE; // shift off a tile
            tile_2 <<= SIZEOF_TILE; // move from 1->2 (2 tile to 4 tile)
        }
    }
    
    state.current_depth--;
    
    if (state.current_depth < CACHE_DEPTH_LIMIT) {
        trans_table_entry_t entry = {static_cast<uint8_t>(state.current_depth), best_score};
        state.cache[grid] = entry;
    }
    
    return best_score;
}

float ai_score_move(state_t &state, grid_t grid, int direction)
{
    //state.depth_limit = std::max(3, std::max(num_distinct_tiles(grid) - 2, num_empty_tiles(grid)));
    state.depth_limit = std::max(12, 16 - num_empty_tiles(grid) + 1);
    grid_t new_grid = grid_move(direction, grid);
    
    if (new_grid == grid) {
        return 0.0f; // move not viable
    }
    
    return alphabeta(state, new_grid, 0.0f, FLT_MAX, false) + 1e-6;
}

#endif