/** @file       animation.c
    @authors    Grace Lee    -   dil15  <dil15@uclive.ac.nz>
    @date       5 October 2015
    @brief      Animation for Game.
*/
#include "system.h"
#include "pio.h"
#include "ledmat.h"
#include "animation.h"

static uint8_t col = 1;
static uint8_t row = 2;

static pio_t rows[] =
{
    LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO, LEDMAT_ROW4_PIO,
    LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO, LEDMAT_ROW7_PIO
};

static pio_t columns[] =
{
    LEDMAT_COL1_PIO, LEDMAT_COL2_PIO, LEDMAT_COL3_PIO,
    LEDMAT_COL4_PIO, LEDMAT_COL5_PIO
};

void ledmat_pixel_set (int col, int row, bool state)
{
    if (state)
    {
        pio_output_low (rows[row]);
        pio_output_low (columns[col]);
    }
    else
    {
        pio_output_high (rows[row]);
        pio_output_high (columns[col]);
    }
}

void drawAnimation (void)
{
    ledmat_pixel_set (col, row, 0);

    if (row == MINROW && col != MAXCOL) {
        col++;
    }
    else if (col == MAXCOL && row != MAXROW) {
        row++;
    }
    else if (row == MAXROW && col != MINCOL) {
        col--;
    }
    else if (col == MINCOL && row != MINROW) {
        row--;
    }

    ledmat_pixel_set (col, row, 1);
}
