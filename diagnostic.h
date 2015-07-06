/*
 * diagnostic.h
 *
 *  Created on: Jun 28, 2015
 *      Author: gsantha
 */

#ifndef CUSTOMHID_ORIG_DIAGNOSTIC_H_
#define CUSTOMHID_ORIG_DIAGNOSTIC_H_

#define EVENT_MAX_HISTORY_EVENTS 50
#define EVENT_EMPTY 0xfffe
#define EVENT_END_OF_STRUCT 0xffff
#define EVENT_PRINT_TIMEOUT 20

typedef struct {
	uint32_t eventID;
	char const eventname[30];
}usb_event_t;

typedef struct {
	uint32_t eventIDhistory[EVENT_MAX_HISTORY_EVENTS];
	char const **eventCallerhistory[EVENT_MAX_HISTORY_EVENTS];
	int eventhistorypointer;
	int print_timeout;
	bool history_updated;
	bool print_history;
} event_struct_t;

void diagnostic_clear_eventhistory(void);
void diagnostic_add_event_to_history(uint32_t eventID, char const **ptrtostr);
void diagnostic_eventhistory_updated(void);
void diagnostic_print_eventhistory(void);

#endif /* CUSTOMHID_ORIG_DIAGNOSTIC_H_ */
