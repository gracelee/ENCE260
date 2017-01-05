/** @file       game.c
    @author     Grace Lee    -   dil15  (51455525)
    @date       5 October 2015
    @brief      Game of Tetris.
    @COPYLEFT Grace Lee <dil15@uclive.ac.nz>	
*/

#include "system.h"
#include "navswitch.h"
#include "tinygl.h"
#include "pacer.h"
#include <stdlib.h>
#include "../fonts/font5x7_1.h"
#include <stdio.h>
#include "ir_uart.h"
#include "animation.h"
#include "ledmat.h"

#define LOOP_RATE 1000
#define SHAPE_SPEED 2
#define MESSAGE_RATE 10
#define PICTURE_LOOP_RATE 500


const int presets[6][10] = {
	{0,0,0,0,0,-1,0,-1,0,0},// T shape
	{-1,-1,0,-1,0,-1,-1,-1,0,0},
	{-1,0,1,0,0,0,0,0,1,1},
	{-1,-1,0,0,0,1,1,0,0,0},
	{0,0,-1,-1,0,0,-1,-1,0,0},
	{-1,-1,0,1,0,0,-1,-1,0,0}};

const int penaltySets [4][10] = {
	{-1,0,0,1,0,0,1,0,1,1},
	{-1,0,0,1,0,0,-1,-1,1,1},
	{-2,-1,0,1,0,0,0,0,1,1},
	{-2,-1,0,0,0,0,0,1,-1,1}};

bool currentBrickEasy = true;
bool nextBrickEasy = true;
bool gameRunning = true;


uint8_t ready = '?';
uint8_t confirm = '!';



static uint8_t sender = 0;
static uint8_t receiver = 0;


void rotate(tinygl_point_t b [5], int dir){
	int posx, posy;
	int i;
	for (i=0; i<4; i++){
		posx = b[i].x;
		posy = b[i].y;
		posx = posx*2;
		posy = posy*2;
		posx = posx + 1 - b[4].x;
		posy = posy + 1 - b[4].y;
	int tempX = posx;
			if (dir == 1){
					posx = -posy;
					posy = tempX;}
			if (dir == -1){
					posx = posy;
					posy = -1*tempX;}
					posx = posx - 1 + b[4].x;
					posy = posy - 1 + b[4].y;
					posx = posx/2;
					posy = posy/2;
					b[i].x = posx;
					b[i].y = posy;
	}
}

void renderFixed(tinygl_point_t pixel, tinygl_point_t fixed [35]){
int i;
	for (i=0;i<35;i++){
		pixel.x = fixed[i].x;
	pixel.y = fixed[i].y;
	tinygl_draw_point(pixel, 1);
  }
	if (nextBrickEasy == false){
		pixel.x = 4;
		pixel.y = 6;
		tinygl_draw_point(pixel, 1);
	}

}

void resetEverything(tinygl_point_t b [5], tinygl_point_t vector [2], int newBlock){
	int i, presetIndex;
	if (nextBrickEasy){
		presetIndex = newBlock % 6;
		currentBrickEasy = true;
		for (i = 0; i<4; i++){
			b[i].x = presets[presetIndex][i];
			b[i].y = presets[presetIndex][i+4];}
		b[4].x = presets[presetIndex][8];
		b[4].y = presets[presetIndex][9];
	}
	else{
			nextBrickEasy = true;
			currentBrickEasy = false;
			presetIndex = newBlock % 4;
			for (i = 0; i<4; i++){
				b[i].x = penaltySets[presetIndex][i];
				b[i].y = penaltySets[presetIndex][i+4];
			}
				b[4].x = penaltySets[presetIndex][8];
				b[4].y = penaltySets[presetIndex][9];
		}
		vector[0].x = 2;
	vector[0].y = 7;
	}

void renderBlock(tinygl_point_t pixel, tinygl_point_t b [5], tinygl_point_t vector [2]){
int i;
for (i=0; i<4; i++){
	pixel.x = b[i].x + vector[0].x;
	pixel.y = b[i].y + vector[0].y;
	tinygl_draw_point(pixel, 1);}
}

bool isInBorder(tinygl_point_t b [5], tinygl_point_t vector [2]){
  bool inBorder = false;
  int i;
  for (i=0; i<4; i++){
    if (b[i].y + vector[0].y<0){
      inBorder = true;}
    if ((b[i].x+vector[0].x > 4)||(b[i].x+vector[0].x < 0)){
      inBorder = true;}
  }
  return inBorder;
}

bool isOnFixed(tinygl_point_t b [5], tinygl_point_t fixed [35], tinygl_point_t vector [2]){
bool onFixed = false;
int bi, fi;
for (bi=0; bi<4; bi++){
	for (fi=0; fi<35; fi++){
		if ((b[bi].x + vector[0].x == fixed[fi].x)&&(b[bi].y + vector[0].y == fixed[fi].y)){
			onFixed = true;
		}
	}
}
return onFixed;
}

bool canBeCleared(tinygl_point_t fixed [35], int rowInQuestion){
bool rowComplete = false;
int column, fi, counter;
counter = 0;
for (column=0;column<5;column++){
	for (fi=0; fi<35; fi++){
		if ((fixed[fi].x == column) && (fixed[fi].y == rowInQuestion) && (column==counter)){
			counter ++;}
		}
		if (counter >= 5){rowComplete = true;}}
		return rowComplete;
}

void clearRow(tinygl_point_t fixed [35], int row){
int i;
for (i=0; i<35; i++){
	if (fixed[i].y == row){
		fixed[i].y = -1;
		fixed[i].x = -1;}
		if (fixed[i].y > row){
			fixed[i].y --;}}
			ir_uart_putc('A');}

void checkRows(tinygl_point_t fixed [35]){
				int i;
				for(i = 0; i < 6; i++){
					if(canBeCleared(fixed,i)){
						clearRow(fixed,i);}}
			}

void makeFixed(tinygl_point_t b [5], tinygl_point_t fixed [35], tinygl_point_t vector [2]){
  int bi, fi;
  bool solved;
  for (bi=0; bi<4; bi++){
    solved = false;
    for (fi=0; fi<35; fi++){
      if (!solved){
        if ((fixed[fi].x==-1)&&(fixed[fi].y==-1)){
          fixed[fi].x = b[bi].x+vector[0].x;
          fixed[fi].y = b[bi].y+vector[0].y;
          solved = true;}}}}

					checkRows(fixed);
					for (bi=0; bi<35; bi++){
						if (fixed[bi].y > 6){
							//ir_uart_putc('G');
							//win = false;
							gameRunning = false;
		}
	}
}



void player_init (void)
{
    pacer_wait ();
    navswitch_update ();
    drawAnimation();
    ir_uart_init ();

    if (navswitch_push_event_p (NAVSWITCH_PUSH))
    {
        ir_uart_putc(confirm);
        sender = 1;
    }
    else if (ir_uart_read_ready_p ()) {
        if (ir_uart_getc () == confirm) {
            receiver = 1;
        }
    }
}

void sender_init (void)
			{
				uint8_t pressed = 0;
				while (!pressed){
					pacer_wait ();
					navswitch_update ();
					if (navswitch_push_event_p (NAVSWITCH_PUSH))
					{
						ir_uart_putc(ready);
						pressed = 1;}
			    }
			}

			// Makes this device player 2 and waits for a given signal
void receiver_init (void)
			{
				uint8_t pressed = 0;
				while (!pressed)
				{
					pacer_wait ();
					if (ir_uart_read_ready_p ()) {
						if (ir_uart_getc () == ready) {
							pressed = 1;
						}
					}
					}
			}

void connect(void){
		ledmat_init ();
    navswitch_init ();
    ir_uart_init ();
    pacer_init (ANIMATION_LOOP_RATE);

    while (!sender && !receiver)
    {
        player_init ();
    }

    pacer_init (PICTURE_LOOP_RATE);
    ledmat_init ();

		if (sender) {
        sender_init ();
    }

    if (receiver) {
        receiver_init ();
    }
}


void game_over(void){

	tinygl_font_set (&font5x7_1);
	tinygl_text_speed_set (MESSAGE_RATE);
	tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
	tinygl_text("Game Over");

	while(1){
		pacer_wait();
		tinygl_update();
	}
}


int main (void){
	int tick = 0;

	system_init ();
	ir_uart_init ();
	tinygl_init (LOOP_RATE);
	navswitch_init ();
	pacer_init (LOOP_RATE);
	srand(1);
	tinygl_font_set (&font5x7_1);
	tinygl_text_speed_set (MESSAGE_RATE);
	tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);

	tinygl_point_t pixel;
	tinygl_point_t pos [2];

	tinygl_point_t block [5];
	resetEverything(block, pos, rand());
	tinygl_point_t fixed [35];
	  int i = 0;
	  for (i=0; i<35; i++){
	    fixed[i].x = -1;
	    fixed[i].y = -1;}

	connect();

	while (gameRunning){
		pacer_wait ();
		navswitch_update ();

		tinygl_clear();
		if (ir_uart_read_ready_p ()){
			if (ir_uart_getc () == 'A'){
				if (currentBrickEasy == true){
					nextBrickEasy = false;
				}
			}

	}

		tick = tick + 1;
		if (tick > LOOP_RATE / SHAPE_SPEED) {
		tick = 0;
		      pos[0].y --;

		      if (isInBorder(block, pos)){
		        pos[0].y ++;

		        makeFixed(block,fixed,pos);


		        resetEverything(block, pos, rand());

		      }
						/*check for coll against fixed*/
			if (isOnFixed(block, fixed, pos)){
			pos[0].y ++;
			        makeFixed(block,fixed,pos);
			        resetEverything(block, pos, rand());
			}
				checkRows(fixed);
				}

			if (navswitch_push_event_p (NAVSWITCH_WEST)){
				pos[0].x --;
			if (isInBorder(block, pos)){pos[0].x++;}
			if (isOnFixed(block, fixed, pos)){pos[0].x++;}
		    }
			if (navswitch_push_event_p (NAVSWITCH_EAST)){
				pos[0].x ++;
			if (isInBorder(block, pos)){pos[0].x--;}
			if (isOnFixed(block, fixed, pos)){pos[0].x--;}
				}
			if (navswitch_push_event_p (NAVSWITCH_NORTH)){
					pos[0].y --;

				if (isInBorder(block, pos)){
					pos[0].y ++;
        makeFixed(block,fixed,pos);
        resetEverything(block, pos, rand());
			}
			if (isOnFixed(block, fixed, pos)){
				pos[0].y ++;
        makeFixed(block,fixed,pos);
        resetEverything(block, pos, rand());
			}
		}
			if (navswitch_push_event_p (NAVSWITCH_SOUTH)){


				rotate(block, 1);
			if (isInBorder(block, pos)){rotate(block, -1);}
			if (isOnFixed(block, fixed, pos)){
			rotate(block, -1);
		}
			}
			if (navswitch_push_event_p (NAVSWITCH_PUSH)){
			rotate(block, 1);
			if (isInBorder(block, pos)){rotate(block, -1);}
			if (isOnFixed(block, fixed, pos)){
			rotate(block, -1);}}
			renderBlock(pixel,block,pos);
			renderFixed(pixel, fixed);
			tinygl_update ();
			}
			game_over();

			return 0;
}
