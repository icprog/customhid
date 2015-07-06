/*
 * diagnostic.c
 *
 *  Created on: Jun 28, 2015
 *      Author: gsantha
 */

#include <stdint.h>
#include <stdbool.h>
#include "diagnostic.h"
#include "utils/uartstdio.h"

usb_event_t const usbevents[] = {
		{0x0000, "UE_CONNECTED"},
		{0x0001, "UE_DISCONNECTED"},
		{0x0002, "UE_RX_AVAILABLE"},
		{0x0003, "UE_DATA_REMAINING"},
		{0x0004, "UE_REQUEST_BUFFER"},
		{0x0005, "UE_TX_COMPLETE"},
		{0x0006, "UE_ERROR"},
		{0x0007, "UE_SUSPEND"},
		{0x0008, "UE_RESUME"},
		{0x0009, "UE_SCHEDULER"},
		{0x000a, "UE_STALL"},
		{0x000b, "UE_POWER_FAULT"},
		{0x000c, "UE_POWER_ENABLE"},
		{0x000d, "UE_POWER_DISABLE"},
		{0x000e, "UE_COMP_IFACE_CHANGE"},
		{0x000f, "UE_COMP_EP_CHANGE"},
		{0x0010, "UE_COMP_STR_CHANGE"},
		{0x0011, "UE_COMP_CONFIG"},
		{0x0012, "UE_UNKNOWN_CONNECTED"},
		{0x0013, "UE_SOF"},
		{0x0014, "UE_LPM_SLEEP"},
		{0x0015, "UE_LPM_RESUME"},
		{0x0016, "UE_LPM_ERROR"},

		{0x9000, "UDE_GET_REPORT"},
		{0x9001, "UDE_REPORT_SENT"},
		{0x9002, "UDE_GET_REPORT_BUFFER"},
		{0x9003, "UDE_SET_REPORT"},
		{0x9004, "UDE_GET_PROTOCOL"},
		{0x9005, "UDE_SET_PROTOCOL"},
		{0x9006, "UDE_IDLE_TIMEOUT"},

		{0xffff, "eos"},
};

char const *strRXHandler = {"RX Handler"};
char const *strTXHandler = {"TX Handler"};
event_struct_t events;


void diagnostic_clear_eventhistory(void) {
	events.print_history = false;
	events.history_updated = false;
	events.print_timeout = 0;
	events.eventhistorypointer = 0;
}

void diagnostic_add_event_to_history(uint32_t eventID, char const **ptrtostr) {
	events.eventIDhistory[events.eventhistorypointer] = eventID;
	events.eventCallerhistory[events.eventhistorypointer] = ptrtostr;
	if (++events.eventhistorypointer >= EVENT_MAX_HISTORY_EVENTS)
		events.eventhistorypointer = 0;
}

void diagnostic_eventhistory_updated(void) {
	events.history_updated = true;
	events.print_timeout = 0;
}

void diagnostic_print_eventhistory(void) {
	int i, j;
	bool event_unknown;

	UARTprintf("------------------------------------------------------------------------------------------------------------\n");

	for (i = 0; i < events.eventhistorypointer; i++) {
		j = 0;
		event_unknown = true;
		while (usbevents[j].eventID != EVENT_END_OF_STRUCT) {
			if (events.eventIDhistory[i] == usbevents[j].eventID) {
				UARTprintf("ID: 0x%04x\tCaller: %s\tEvent: %s\n", events.eventIDhistory[i], *events.eventCallerhistory[i], usbevents[j].eventname);
				event_unknown = false;
				break;
			}
			j++;
		}

		if (event_unknown == true)
			UARTprintf("ID: 0x%04x\tCaller: %s\tEvent: %s\n", events.eventIDhistory[i], *events.eventCallerhistory[i], "UNKNOWN USB EVENT!");
	}

	UARTprintf("------------------------------------------------------------------------------------------------------------\n");

	diagnostic_clear_eventhistory();
}



