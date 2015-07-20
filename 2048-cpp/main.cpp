//
//  main.cpp
//  2048-cpp
//
//  Created by Ryan Lord on 17/06/15.
//  Copyright (c) 2015 Ryan Lord. All rights reserved.
//

#include <iostream>
#include "grid.h"
#include "ai.h"

int main(int argc, const char * argv[]) {
    /*
     init_lookup_tables();

    int testGrid[4][4] = {
        
        {4, 8, 8, 0},
        {4, 8, 8, 8},
        {2, 8, 8, 0},
        {2, 8, 0, 2}
     
        {2, 2, 2, 2},
        {2, 2, 2, 2},
        {2, 2, 2, 2},
        {2, 2, 2, 0}
     
        
        {0, 2, 4, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };
    
    grid_t tgrid = array_to_packed_grid(&testGrid);
    print_grid(tgrid);
    return 0;
    
    std::cout << "Move up" << std::endl;
    print_grid(grid_move(0, tgrid));
     
    std::cout << "Move down" << std::endl;
    print_grid(grid_move(1, tgrid));
    
    std::cout << "Move left" << std::endl;
    print_grid(grid_move(2, tgrid));
    
    std::cout << "Move right" << std::endl;
    print_grid(grid_move(3, tgrid));
    return 0;
    */
    
    std::cout << "Initialising lookup tables" << std::endl;
    init_lookup_tables();
    
    std::cout << "Generating new grid" << std::endl;
    grid_t grid = create_new_grid();
    if (grid == 0) {
        std::cerr << "Grid initialisation failed";
        return 1;
    }
    
    int best_direction;
    unsigned move_count = 0;
    
    std::cout << "Playing the game" << std::endl;
    for (;;) {
        std::printf("Move #%d", ++move_count);
        std::cout << std::endl;
        
        best_direction = find_best_move(grid);
        if (best_direction == -1) {
            break;
        }
        grid = insert_random_tile(grid_move(best_direction, grid));
        std::cout << "----" << std::endl;
    }
    
    std::cout << "No move moves. Game over" << std::endl;
    return 0;
}
