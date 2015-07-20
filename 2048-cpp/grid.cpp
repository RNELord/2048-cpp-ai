//
//  Grid.h
//  2048-cpp
//
// Based on the integer representation by nneonneo
// https://github.com/nneonneo/2048-ai/
//
//  Created by Ryan Lord on 17/06/15.
//  Copyright (c) 2015 Ryan Lord. All rights reserved.
//

#include <iostream>
#include <math.h>
#include "grid.h"
#include "time.h"

static const grid_t ROW_MASK = 0xFFFFULL;
static const grid_t COL_MASK = 0x000F000F000F000FULL;

static const float SCORE_LOST_PENALTY = 200000.0;
static const float SCORE_MONOT_POWER = 4.0;
static const float SCORE_MONOT_WEIGHT = 47.0;
static const float SCORE_SUM_POWER = 3.5;
static const float SCORE_SUM_WEIGHT = 11.0;
static const float SCORE_MERGES_WEIGHT = 700.0;
static const float SCORE_EMPTY_WEIGHT = 270.0;

#define SIZEOF_TABLES 65536
static grid_t move_up_table[SIZEOF_TABLES];
//static row_t move_right_table[SIZEOF_TABLES];
static grid_t move_right_table[SIZEOF_TABLES]; // grid_t to avoid move errors due to shift >= type width and overflow
static grid_t move_down_table[SIZEOF_TABLES];
//static row_t move_left_table[SIZEOF_TABLES];
static grid_t move_left_table[SIZEOF_TABLES]; // grid_t to avoid move errors due to shift >= type width and overflow

static float real_score_table[SIZEOF_TABLES];
static float heuristic_score_table[SIZEOF_TABLES];


// TODO: use SIZEOF_TILE here (requires bitmasks to also change)
static inline grid_t unpack_column(row_t row) {
    grid_t tmp = row;
    return (tmp | (tmp << 12ULL) | (tmp << 24ULL) | (tmp << 36ULL)) & COL_MASK;
}

static inline row_t reverse_row(row_t row) {
    return (row >> 12) | ((row >> 4) & 0x00F0)  | ((row << 4) & 0x0F00) | (row << 12);
}

static inline grid_t transpose(grid_t x)
{
    grid_t a1 = x & 0xF0F00F0FF0F00F0FULL;
    grid_t a2 = x & 0x0000F0F00000F0F0ULL;
    grid_t a3 = x & 0x0F0F00000F0F0000ULL;
    grid_t a = a1 | (a2 << 12) | (a3 >> 12);
    grid_t b1 = a & 0xFF00FF0000FF00FFULL;
    grid_t b2 = a & 0x00FF00FF00000000ULL;
    grid_t b3 = a & 0x00000000FF00FF00ULL;
    return b1 | (b2 >> 24) | (b3 << 24);
}
// END TODO


template<typename int_t>
static inline void print_integer_to_binary(int bits, int_t n)
{
    char bitset[bits];
    int_t i;
    for (i = 0; i < bits; ++i) {
        if ((n & ((int_t)1 << i)) == 0) {
            bitset[bits - i] = '0';
        } else {
            bitset[bits - i] = '1';
        }
    }
    for (i = 0; i < bits; i++) {
        std::cout << bitset[i];
    }
}

static inline void print_grid_binary(grid_t grid)
{
    int i;
    grid_t tile;
    
    for (i = 0; i < sizeof(row_t) * SIZEOF_TILE; i++) {
        tile = grid >> SIZEOF_TILE;
        grid >>= SIZEOF_TILE;
        print_integer_to_binary(sizeof(row_t) * SIZEOF_TILE, tile);
    }
    std::cout << std::endl;
}

static inline void print_row_binary(row_t row)
{
    int i;
    int tile;
    for (i = 0; i < SIZEOF_TILE; ++i) {
        tile = row >> SIZEOF_TILE;
        row >>= SIZEOF_TILE;
        print_integer_to_binary(sizeof(row_t), tile);
    }
    std::cout << std::endl;
}

static inline void print_grid(const int (*arr)[4][4])
{
    int x, y;
    
    for (x = 0; x < 4; x++) {
        for (y = 0; y < 4; y++) {
            std::cout << (*arr)[x][y];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

static inline grid_t move_vertical(grid_t grid, const grid_t* move_table)
{
    grid_t new_grid = grid;
    grid_t transposed = transpose(grid);
    
    new_grid ^= move_table[(transposed >> 0) & ROW_MASK] << 0;
    new_grid ^= move_table[(transposed >> (SIZEOF_TILE * 4)) & ROW_MASK] << SIZEOF_TILE;
    new_grid ^= move_table[(transposed >> (SIZEOF_TILE * 8)) & ROW_MASK] << (SIZEOF_TILE * 2);
    new_grid ^= move_table[(transposed >> (SIZEOF_TILE * 12)) & ROW_MASK] << (SIZEOF_TILE * 3);
    
    return new_grid;
}

static inline grid_t move_horizontal(grid_t grid, const grid_t* move_table)
{
    grid_t new_grid = grid;
    
    new_grid ^= move_table[(grid >> 0) & ROW_MASK] << 0;
    new_grid ^= move_table[(grid >> (SIZEOF_TILE * 4)) & ROW_MASK] << (SIZEOF_TILE * 4);
    new_grid ^= move_table[(grid >> (SIZEOF_TILE * 8)) & ROW_MASK] << (SIZEOF_TILE * 8);
    new_grid ^= move_table[(grid >> (SIZEOF_TILE * 12)) & ROW_MASK] << (SIZEOF_TILE * 12);
    
    return new_grid;
    
}

static float score_helper(grid_t grid, const float* score_table)
{
    return score_table[(grid >>  0) & ROW_MASK] +
    score_table[(grid >> 16) & ROW_MASK] +
    score_table[(grid >> 32) & ROW_MASK] +
    score_table[(grid >> 48) & ROW_MASK]
    ;
}

static inline grid_t choose_random_tile()
{
    return rand() <= PROBABILITY_OF_2_TILE ? 1 : 2;
}


/*
 * Shared library functions below here
 */


void init_lookup_tables()
{
    srand(time(NULL)); // good a place as any since this function is mandatory for correct operation
    
    unsigned row;
    int i, j;
    
    float sum, monot_left, monot_right;
    int empty, merges, counter, previous;
    
    for (row = 0; row < SIZEOF_TABLES; ++row) {
        unsigned line[4] = {
            (row >> 0                ) & 0xf,
            (row >> SIZEOF_TILE      ) & 0xf,
            (row >> (SIZEOF_TILE * 2)) & 0xf,
            (row >> (SIZEOF_TILE * 3)) & 0xf
        };
        
        // precalculate true score table (sum of tile + merged)
        float score = 0.0f;
        for (i = 0; i < 4; ++i) {
            if (line[i] >= 2) {
                score += (line[i] - 1) * (1 << line[i]);
            }
        }
        real_score_table[row] = score;
        
        // precalculate heuristic scores
        sum = empty = merges = previous = counter = 0;
        
        for (i = 0; i < 4; ++i) {
            if (line[i] == 0) {
                empty++;
                continue;
            }
            
            sum += pow(line[i], SCORE_SUM_POWER);
            
            if (previous == line[i]) {
                counter++;
            } else if (counter > 0) {
                merges += counter + 1;
                counter = 0;
            }
            
            previous = line[i];
        }
        if (counter > 0) {
            merges += counter + 1;
        }
        
        // since we transpose the board, we only need left/right, not up and down too since they're just transposed versions
        monot_left = monot_right = 0;
        for (i = 1; i < 4; ++i) {
            if (line[i - 1] > line[i]) {
                monot_left += pow(line[i-1], SCORE_MONOT_POWER) - pow(line[i], SCORE_MONOT_POWER);
            } else {
                monot_right += pow(line[i], SCORE_MONOT_POWER) - pow(line[i-1], SCORE_MONOT_POWER);
            }
        }
        
        heuristic_score_table[row] = SCORE_LOST_PENALTY
        + (SCORE_EMPTY_WEIGHT * empty)
        + (SCORE_MERGES_WEIGHT * merges)
        - (SCORE_MONOT_WEIGHT * std::min(monot_left, monot_right))
        - (SCORE_SUM_WEIGHT * sum)
        ;
        
        // move left
        for (i = 0; i < 3; ++i) {
            // find next non-empty tile
            for (j = i + 1; j < 4; ++j) {
                if (line[j] != 0) {
                    break;
                }
            }
            if (j == 4) {
                // no non-empty tile found
                break;
            }
            
            if (line[i] == 0) {
                // move next tile over
                line[i] = line[j];
                line[j] = 0;
                
                i--; // repeat moving over tiles for this row
            } else if (line[i] == line[j]) {
                // we have a merge to perform
                
                if (line[i] != 0xf) { // don't overflow - hard lock to 32768 (0xf = 2^16) (assume 32768 + 32768 = 32768)
                    line[i]++;
                }
                
                line[j] = 0;
            }
        }
        
        row_t result = (line[0] << 0) |
        (line[1] << SIZEOF_TILE) |
        (line[2] << (SIZEOF_TILE * 2)) |
        (line[3] << (SIZEOF_TILE * 3));
        
        row_t result_reversed = reverse_row(result);
        unsigned row_reversed = reverse_row(row);
        
        move_up_table[row]              = unpack_column(row)          ^ unpack_column(result);
        move_right_table[row_reversed]  = row_reversed                ^ result_reversed;
        move_down_table[row_reversed]   = unpack_column(row_reversed) ^ unpack_column(result_reversed);
        move_left_table[row]            = row                         ^ result;
    }
}

void print_grid(grid_t grid)
{
    int x, y;
    
    for (x = 0; x < 4; x++) {
        for (y = 0; y < 4; y++) {
            //print_grid_binary(grid);
            printf("%c", "0123456789abcdef"[(grid)&0xf]);
            grid >>= SIZEOF_TILE;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

grid_t array_to_packed_grid(const int (*arr)[4][4])
{
    grid_t grid = 0;
    int x, y;
    int i = 0;
    
    for (x = 0; x < 4; x++) {
        for (y = 0; y < 4; y++) {
            // 0 = 0, 1 = 2, 2 = 4, 3 = 8, 4 = 16, 5 = 32, 6 = 64, 7 = 128 etc...
            if ((*arr)[x][y] == 0) {
                grid |= (grid_t)0 << (SIZEOF_TILE * i);
            } else {
                grid |= (grid_t)round(log((*arr)[x][y]) / log(2)) << (SIZEOF_TILE * i);
            }
            i++;
            
            //print_grid_binary(grid);
        }
    }
    
    return grid;
}

grid_t grid_move(int dir, grid_t grid)
{
    switch (dir) {
        case GRID_DIR_UP:
            return move_vertical(grid, move_up_table);
            
        case GRID_DIR_DOWN:
            return move_vertical(grid, move_down_table);
            
        case GRID_DIR_LEFT:
            return move_horizontal(grid, move_left_table);
            
        case GRID_DIR_RIGHT:
            return move_horizontal(grid, move_right_table);
            
        default:
            return ~0ULL; // empty grid
    }
}

float score_heuristic(grid_t grid)
{
    // we only did left/right heuristics, so we need to include the transposed to get column scores
    return score_helper(grid, heuristic_score_table) + score_helper(transpose(grid), heuristic_score_table);
}

float score_real(grid_t grid)
{
    return score_helper(grid, real_score_table);
}

int num_distinct_tiles(grid_t grid)
{
    row_t bitset = 0;
    while (grid) {
        bitset |= (grid_t)1 << (grid & 0xf);
        grid >>= 4;
    }
    
    // we don't want empty tiles to be included
    bitset >>= 1;
    
    int count = 0;
    while (bitset) {
        bitset &= bitset - 1;
        count++;
    }
    
    return count;
}

// Borrowed from nneonneo's implementation
//
// Count the number of empty positions (= zero nibbles) in a board.
// Precondition: the board cannot be fully empty.
static inline int nneonneo_count_empty(grid_t x)
{
    x |= (x >> 2) & 0x3333333333333333ULL;
    x |= (x >> 1);
    x = ~x & 0x1111111111111111ULL;
    // At this point each nibble is:
    //  0 if the original nibble was non-zero
    //  1 if the original nibble was zero
    // Next sum them all
    x += x >> 32;
    x += x >> 16;
    x += x >>  8;
    x += x >>  4; // this can overflow to the next nibble if there were 16 empty positions
    return x & 0xf;
}

int num_empty_tiles(grid_t grid)
{
    if (grid == 0) {
        return 16;
    }
    
    /*
    int count = 0;
    grid_t tmp = grid;
    
    while (tmp) {
        if (((grid >> SIZEOF_TILE) & 0xf) == 0) {
            count++;
        }
        tmp >>= SIZEOF_TILE;
    }
    
    
    return count;
     */
    return nneonneo_count_empty(grid);
}

grid_t insert_random_tile(grid_t grid)
{
    grid_t tmp = grid;
    int empty = num_empty_tiles(grid);
    int pos = std::floor(rand() % empty);
    grid_t tile = choose_random_tile();
    
    for(;;) {
        while ((tmp & 0xf) != 0) {
            tmp >>= SIZEOF_TILE;
            tile <<= SIZEOF_TILE;
        }
        if (pos == 0) {
            break;
        }
        pos--;
        tmp >>= SIZEOF_TILE;
        tile <<= SIZEOF_TILE;
    }
    
    return grid | tile;
}

grid_t create_new_grid()
{
    grid_t grid = choose_random_tile() << (SIZEOF_TILE * (int)std::floor(rand() % 16 + 1));
    if (grid == 0) {
        throw "First tile placement failed";
    }
    return insert_random_tile(grid);
}
