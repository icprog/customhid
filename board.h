/*
 * board.h
 *
 *  Created on: 2015.05.17.
 *      Author: sg
 */

#ifndef CUSTOMHID_ORIG_BOARD_H_
#define CUSTOMHID_ORIG_BOARD_H_

#define RED 0x02
#define BLUE 0x04
#define GREEN 0x08


void toggle_red(void);
void toggle_blue(void);
void toggle_green(void);

void set_red(void);
void set_blue(void);
void set_green(void);

void clear_red(void);
void clear_blue(void);
void clear_green(void);

void blink_red(int ms);
void blink_blue(int ms);
void blink_green(int ms);

#endif /* CUSTOMHID_ORIG_BOARD_H_ */
