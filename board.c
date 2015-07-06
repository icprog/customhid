/*
 * board.c
 *
 *  Created on: 2015.05.17.
 *      Author: sg
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/systick.h"
#include "board.h"

// red: RED
// blue: BLUE
// green: GREEN

static void delay_ms(int ms) {

    ROM_SysCtlDelay( (ROM_SysCtlClockGet()/(3*1000))*ms ) ;  // less accurate
}

// Toggle functions
void toggle_red(void) {
	static uint32_t ledstate = GPIO_PIN_1;

	ledstate ^=GPIO_PIN_1;
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, ledstate);
}

void toggle_blue(void) {
	static uint32_t ledstate = GPIO_PIN_2;

	ledstate ^=GPIO_PIN_2;
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, ledstate);
}

void toggle_green(void) {
	static uint32_t ledstate = GPIO_PIN_3;

	ledstate ^=GPIO_PIN_3;
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, ledstate);
}

// Set functions
void set_red(void) {
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
}

void set_blue(void) {
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
}

void set_green(void) {
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
}

// Clear functions
void clear_red(void) {
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

void clear_blue(void) {
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
}

void clear_green(void) {
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}

// Blink functions
void blink_red(int ms) {
	set_red();
	delay_ms(ms);
	clear_red();
}

void blink_blue(int ms) {
	set_blue();
	delay_ms(ms);
	clear_blue();
}

void blink_green(int ms) {
	set_green();
	delay_ms(ms);
	clear_green();
}



