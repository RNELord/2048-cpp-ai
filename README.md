2048 AI testing framework
=========================
  
* Place _client.js_ inside offical 2048/js directory as _socket.js_ (or inject it another way - e.g. bookmarklet)  
* Add this to the end of the scripts in index.html: <script src="js/socket.js"></script>  
* Launch socket server (see _socket-server.sh_)  
* Open/refresh page to perform initial socket connection, sit back, watch  
  
  
Changing algorithm
------------------
* Change __AI_ALGORITHM__ definte inside _ai.h_  
* Compile  

Adding new algorithms
----------------------
(Hacky for now)  
Create your cpp file similar to this:  
```
#include "montecarlo.h"

#if AI_ALGORITHM == AI_ALGORITHM_MONTECARLO

float ai_score_move(state_t &state, grid_t grid, int direction)
{
    // your AI code here
    
    return 0.0f;
}

#endif
```
  
and your header file similar to this  
```
#ifndef ___048_cpp__montecarlo__
#define ___048_cpp__montecarlo__

#include "ai.h"
#include "grid.h"

#endif /* defined(___048_cpp__montecarlo__) */
```  
  
then adjust the define mentioned earlier in ai.h like this:   
```
//#define AI_ALGORITHM AI_ALGORITHM_EXPECTIMAX
//#define AI_ALGORITHM AI_ALGORITHM_ALPHABETA
#define AI_ALGORITHM AI_ALGORITHM_MONTECARLO
```
  
See expectimax.h and expectimax.cpp or alphabeta for examples of how this is used.
  
  
Outside credits
---------------
Integer optimised grid representation and metrics based upon https://github.com/nneonneo/2048-ai/
  
Build targets
-------------
Targets are 2048-cpp, libgrid, and libai  
libgrid is standalone representation of the grid (libai will load this, or it can be used standalone)  
libai is the AI bundled in - this depends on libgrid being built  
2048-cpp is just a way to launch a quick game to help debug AI/grid logic inside the IDE 
