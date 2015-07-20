//
//  grid.h
//  2048-cpp
//
//  Created by Ryan Lord on 18/06/15.
//  Copyright (c) 2015 Ryan Lord. All rights reserved.
//

#ifndef _048_cpp_grid_h
#define _048_cpp_grid_h

#include "dll.h"
#include <stdint.h>
#include <cmath>

typedef uint64_t grid_t;
typedef uint16_t row_t; // 4 rows to a grid (16 * 4 = 64 grid_t)
static const grid_t SIZEOF_TILE = 4ULL; // 4 nibbles to a row (4 * 4 = 16 row_t)

static const int GRID_DIR_UP = 0;
static const int GRID_DIR_DOWN = 1;
static const int GRID_DIR_LEFT = 2;
static const int GRID_DIR_RIGHT = 3;
static const char* GRID_MOVES[] = {"UP\0", "DOWN\0", "LEFT\0", "RIGHT\0"};

static const float PROBABILITY_OF_2_TILE = 0.9f;
static const float PROBABILITY_OF_4_TILE = 0.1f;

extern "C" {
    DLL_PUBLIC void init_lookup_tables();
    DLL_PUBLIC void print_grid(grid_t grid);
    DLL_PUBLIC grid_t array_to_packed_grid(const int (*arr)[4][4]);
    DLL_PUBLIC grid_t grid_move(int dir, grid_t grid);
    DLL_PUBLIC float score_heuristic(grid_t grid);
    DLL_PUBLIC float score_real(grid_t grid);
    DLL_PUBLIC int num_distinct_tiles(grid_t grid);
    DLL_PUBLIC int num_empty_tiles(grid_t grid);
    DLL_PUBLIC grid_t insert_random_tile(grid_t grid);
    DLL_PUBLIC grid_t create_new_grid();
}

#endif
