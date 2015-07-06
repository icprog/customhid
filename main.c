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
#include "driverlib/uart.h"

// UART
#include "utils/uartstdio.h"

// USB
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usbhid.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdhid.h"

// application
#include "customhid.h"
#include "uart.h"
#include "board.h"
#include "diagnostic.h"


extern databuffer_t rxdata;
extern databuffer_t txdata;
extern tUSBDHIDDevice hiddatapipe_device;
extern usbstate_t usbstate;
extern event_struct_t events;

void systickhandler(void) {

	if (events.history_updated == true && (++events.print_timeout >= EVENT_PRINT_TIMEOUT))
		events.print_history = true;
	else
		events.print_history = false;
}


int main(void) {

    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    ROM_FPULazyStackingEnable();

    // Set the clocking to run from the PLL at 50MHz.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    // Enable the GPIO peripheral used for USB, and configure the USB
    // pins.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 + GPIO_PIN_2 + GPIO_PIN_3);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 + GPIO_PIN_2 + GPIO_PIN_3, 0);

    // Init diagnostic
    diagnostic_clear_eventhistory();

    // Init UART
    ConfigureUART();

    // Print welcome message
    UARTprintf("Configuring USB\n");

    // Set the USB stack mode to Device mode with VBUS monitoring.
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    // Pass our device information to the USB library and place the device
    // on the bus.
    USBDHIDInit(0, &hiddatapipe_device);

    // Block until connected
    while (!usbstate.connected)
    	;

    // Configure SysTick
	ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 1000); // 1 ms
	ROM_SysTickEnable();
	ROM_SysTickIntEnable();

    while(1) {

        if (events.print_history == true)
        	diagnostic_print_eventhistory();

    }
}
