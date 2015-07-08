#include <stdint.h>
#include <stdbool.h>
#include "customhid.h"

#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usbhid.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdhid.h"

#include "board.h"
#include "diagnostic.h"

extern databuffer_t rxdata;
extern databuffer_t txdata;
extern char const *strRXHandler;
extern char const *strTXHandler;
extern event_struct_t events;

usbstate_t usbstate;



uint32_t txhandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgData, void *pvMsgData) {

	diagnostic_add_event_to_history(ui32Event, &strTXHandler);
	diagnostic_eventhistory_updated();

	switch(ui32Event) {

		 case USB_EVENT_TX_COMPLETE:
			 usbstate.txcomplete = 1;
			 break;

		case USBD_HID_EVENT_IDLE_TIMEOUT:
			//! This event indicates to an application that a report idle timeout has
			//! occurred and requests a pointer to the report that must be sent back to
			//! the host.  The ui32MsgData value contains the requested report ID and
			//! pvMsgData contains a pointer that must be written with a pointer to the
			//! report data that is to be sent.  The callback must return the number of
			//! bytes in the report pointed to by *pvMsgData.

			*(uint8_t **) pvMsgData = &txdata.buffer[0];
			return HID_PACKET_SIZE; // return the number of bytes in the buffer pointed to by *pvMsgData
			break;
		 default:
			 break;

	}

    return 0;
}

uint32_t rxhandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgData, void *pvMsgData) {
	uint32_t readPacketSize;

	diagnostic_add_event_to_history(ui32Event, &strRXHandler);
	diagnostic_eventhistory_updated();

	switch(ui32Event) {

	    case USB_EVENT_CONNECTED:
	        usbstate.connected = 1;
	        break;

	    case USB_EVENT_DISCONNECTED:
	    	usbstate.connected = 0;
	        break;

	    case USB_EVENT_RX_AVAILABLE:
	    	//Ignore the USB_EVENT_RX_AVAILABLE case as it was coded for the OUT endpoint 1 event.
	    	//USB_EVENT_RX_AVAILABLE would happen if you receive the HID report on OUT Endpoint 1.

	        break;

	    case USBD_HID_EVENT_IDLE_TIMEOUT:
			//! This event indicates to an application that a report idle timeout has
			//! occurred and requests a pointer to the report that must be sent back to
			//! the host.  The ui32MsgData value contains the requested report ID and
			//! pvMsgData contains a pointer that must be written with a pointer to the
			//! report data that is to be sent.  The callback must return the number of
			//! bytes in the report pointed to by *pvMsgData.

			*(uint8_t **) pvMsgData = &txdata.buffer[0];
			return HID_PACKET_SIZE; // return the number of bytes in the buffer pointed to by *pvMsgData
			break;

	    case USBD_HID_EVENT_GET_REPORT_BUFFER:
	    	//*****************************************************************************
	    	//
	    	//! This event indicates that the host has sent a Set_Report request to
	    	//! the device and requests that the device provide a buffer into which the
	    	//! report can be written.  The ui32MsgValue parameter contains the received
	    	//! report type in the high byte and report ID in the low byte (as passed in
	    	//! the wValue field of the USB request structure).  The pvMsgData parameter
	    	//! contains the length of buffer requested.  Note that this is the actual
	    	//! length value cast to a "void *" type and not a pointer in this case.
	    	//! The callback must return a pointer to a suitable buffer (cast to the
	    	//! standard "uint32_t" return type for the callback).
	    	//
	    	//*****************************************************************************

	        readPacketSize=(uint32_t)pvMsgData;
	        if(readPacketSize>HID_PACKET_SIZE) {
	                while(1) //Shouldn't happen
	                ; 
	            }

	        return((uint32_t)&rxdata.buffer);
	        break;

	    case USBD_HID_EVENT_GET_REPORT:
	    	//*****************************************************************************
	    	//
	    	//! This event indicates that the host is requesting a particular report be
	    	//! returned via endpoint 0, the control endpoint.  The ui32MsgValue parameter
	    	//! contains the requested report type in the high byte and report ID in the
	    	//! low byte (as passed in the wValue field of the USB request structure).
	    	//! The pvMsgData parameter contains a pointer which must be written with the
	    	//! address of the first byte of the requested report.  The callback must
	    	//! return the size in bytes of the report pointed to by *pvMsgData.  The
	    	//! memory returned in response to this event must remain unaltered until
	    	//! \b USBD_HID_EVENT_REPORT_SENT is sent.
	    	//
	    	//*****************************************************************************
	    	rxdata.buffer[0] = 0x0d;
	    	*(uint8_t **) pvMsgData = &rxdata.buffer[0];
	    	return HID_PACKET_SIZE;
	        break;

	    case USBD_HID_EVENT_SET_REPORT:
	    	//*****************************************************************************
	    	//
	    	//! This event indicates that the host has sent the device a report via
	    	//! endpoint 0, the control endpoint.  The ui32MsgValue field indicates the
	    	//! size of the report and pvMsgData points to the first byte of the report.
	    	//! The report buffer was previously returned in response to an
	    	//! earlier \b USBD_HID_EVENT_GET_REPORT_BUFFER callback.  The HID device class
	    	//! driver does not access the memory pointed to by pvMsgData after this
	    	//! callback is made so the application is free to reuse or free it at this
	    	//! point.
	    	//
	    	//*****************************************************************************

	        rxdata.size=(uint16_t) (ui32MsgData&0xffff);
	        usbstate.hostsentreport = 1;
	        break;

	    case USBD_HID_EVENT_SET_PROTOCOL:
	    	break;

	    default:
	        break;
	    }
	return 0;
}
