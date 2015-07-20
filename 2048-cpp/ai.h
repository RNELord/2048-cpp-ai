//
//  ai.h
//  2048-cpp
//
//  Created by Ryan Lord on 18/06/15.
//  Copyright (c) 2015 Ryan Lord. All rights reserved.
//

#ifndef ___048_cpp__ai__
#define ___048_cpp__ai__

#include "dll.h"
#include "grid.h"
#include <cfloat>

#define AI_ALGORITHM_EXPECTIMAX 0
#define AI_ALGORITHM_ALPHABETA 1
#define AI_ALGORITHM_MONTECARLO 2

#define AI_ALGORITHM AI_ALGORITHM_EXPECTIMAX
//#define AI_ALGORITHM AI_ALGORITHM_ALPHABETA
//#define AI_ALGORITHM AI_ALGORITHM_MONTECARLO

struct trans_table_entry_t {
    uint8_t depth;
    float heuristic;
};

#if defined(HAVE_UNORDERED_MAP)
#include <unordered_map>
typedef std::unordered_map<grid_t, trans_table_entry_t> trans_table_t;
#elif defined(HAVE_TR1_UNORDERED_MAP)
#include <tr1/unordered_map>
typedef std::tr1::unordered_map<grid_t, trans_table_entry_t> trans_table_t;
#else
#include <map>
typedef std::map<grid_t, trans_table_entry_t> trans_table_t;
#endif

struct state_t {
    trans_table_t cache;
    int max_depth;
    int current_depth;
    int depth_limit;
    int cache_hits;
    int evaluated;
    
    state_t(): max_depth(0), current_depth(0), depth_limit(0), cache_hits(0), evaluated(0) {};
};

float ai_score_move(state_t &state, grid_t grid, int direction);

extern "C" {
    DLL_PUBLIC float score_move(grid_t grid, int direction);
    DLL_PUBLIC int find_best_move(grid_t grid);
}

#endif /* defined(___048_cpp__ai__) */
