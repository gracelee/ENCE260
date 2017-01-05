/** @file       animation.h
    @authors    Grace Lee    -   dil15  (51455525)
                Cheng Qian   -   cqi17  (92490055)
    @date       5 October 2015
    @brief      Animation for Game.
*/
#ifndef ANIMATION_H
#define ANIMATION_H

#include "system.h"

#define ANIMATION_LOOP_RATE 15

#define MINCOL 1
#define MINROW 2
#define MAXCOL 3
#define MAXROW 4

void ledmat_pixel_set (int col, int row, bool state);

void drawAnimation(void);

#endif
