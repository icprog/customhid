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

#define DATA_INC_TIMEOUT 500

void systickhandler(void) {
	static int systickcnt = 0;

	if (events.history_updated == true && (systickcnt % EVENT_PRINT_TIMEOUT) == 0)
		events.print_history = true;
	else
		events.print_history = false;

	if ((systickcnt % DATA_INC_TIMEOUT) == 0) {
		txdata.buffer[0]++;
		usbstate.txdataupdated = 1;
		UARTprintf("txdata.buffer[0] updated: %d\n", txdata.buffer[0]);
	}

	systickcnt++;
}


int main(void) {
	int i;
	uint8_t data[64];

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

	// enable first IN report by clearing USB tx complete
	usbstate.txcomplete = 1;

    while(1) {

    	// uncomment this if event history log is needed
//        if (events.print_history == true)
//        	diagnostic_print_eventhistory();

        // usbstate.txcomplete flag was set to 1 by txhandler after usbdhidreportwrite had succeeded
        if (usbstate.txcomplete  && usbstate.txdataupdated) {
        	// clear usbstate.txcomplete flag to indicate that usbdhidreportwrite is busy
        	usbstate.txcomplete = 0;
        	usbstate.txdataupdated = 0;
        	// put data (1 byte in this case) into the usb pipeline and the host will poll&read it
        	// the polling rate is defined in the interrupt IN endpoint descriptor

        	data[0] = txdata.buffer[0]; // temp buffer to avoid update change of txdata.buffer between the next 2 calls
        	USBDHIDReportWrite(&hiddatapipe_device, data, 1, 0);
        	UARTprintf("Sending irq IN to host: %d\n", data[0]);

        }

        if (usbstate.hostsentreport) {
        	// clear hostsentreport flag
        	usbstate.hostsentreport = 0;

        	for (i = 0; i < HID_REPORTOUT_SIZE; i++) {
        		UARTprintf("Received OUT report from host: %02x\n", rxdata.buffer[i]);
        		rxdata.buffer[i] = 0;
        	}

        }

    }
}
