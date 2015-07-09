#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"

#include "customhid.h"

#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usbhid.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdhid.h"

#define NUM_STRING_DESCRIPTORS (sizeof(string_descriptors) /                \
                                sizeof(uint8_t *))

#define NUM_HID_SECTIONS        (sizeof(hid_sections) /                    \
                                 sizeof(hid_sections[0]))

#define USB_VID               	0x2047        // Vendor ID (VID)
#define USB_PID               	0x0301        // Product ID (PID)

databuffer_t rxdata;
databuffer_t txdata;

const uint8_t language_descriptor[] = {
		4,
		USB_DTYPE_STRING,
		USBShort(USB_LANG_EN_US)
};

const uint8_t manufacturer_string[] = {
    (8 + 1) * 2,
    USB_DTYPE_STRING,
    'A', 0, 'B', 0, 'C', 0, 'D', 0, 'E', 0, 'F', 0, 'G', 0, 'H', 0,
};

const uint8_t product_string[] = {
		8,
    USB_DTYPE_STRING,
    'X',0x00,'Y',0x00,'Z',0x00,
};

const uint8_t serialnumber_string[] = {
    (4 + 1) * 2,
    USB_DTYPE_STRING,
    'v', 0, '1', 0, '.', 0, '0', 0
};

const uint8_t interface_string[] = {
		28,
    USB_DTYPE_STRING,
    'H',0x00,'I',0x00,'D',0x00,' ',0x00,'I',0x00,'n',0x00,
    't',0x00,'e',0x00,'r',0x00,'f',0x00,'a',0x00,'c',0x00,
    'e',0x00
};

// The descriptor string table
const uint8_t * const string_descriptors[] = {
    language_descriptor,
    manufacturer_string,
    product_string,
    serialnumber_string,
    interface_string
};

static const uint8_t datapipereport_descriptor[] = {

    // 0x06, 0x00, 0xff,
    // HUT chap 3 table 1
    // Vendor specific value 0xff00
    UsagePageVendor(0xff00),
    // 0x09, 0x01,
    // Arbitrary number as we are vendor specific
    Usage(0x01),
    // 0xa1, 0x01,
    Collection(USB_HID_APPLICATION),
        // 0x85, 0x3f,
        // Vendor specific. Appears arbitrary.
        // ID can be left undefined if only one report type is created

		// 0x95, 0x3f,
		// The number of items in the report
		ReportCount(HID_PACKET_SIZE),
		// 0x75, 0x08,
		// As this is just generic data, 8 bits in a byte
		ReportSize(0x08),
		0x26, 0xff, 0x00,
//		LogicalMaximum(0xff00),
		// 0x15, 0x01,
		LogicalMinimum(0),
		// 0x09, 0x80,
		// Arbitrary number as we are vendor specific
		Usage(0x01),
		// 0x81, 0x02, Input (Data,Var,Abs)
		// This shows we are describing the report that goes to the host
		Input(USB_HID_INPUT_DATA | USB_HID_INPUT_VARIABLE | USB_HID_INPUT_ABS),
		// Second report, same except it's output
		ReportCount(HID_PACKET_SIZE),
		ReportSize(0x08),
//		LogicalMaximum(127),
		0x26, 0xff, 0x00,
		LogicalMinimum(0),
		Usage(0x01),
		Output(USB_HID_INPUT_DATA | USB_HID_INPUT_VARIABLE | USB_HID_INPUT_ABS),
    EndCollection
};

static const uint8_t * const datapipeclass_descriptors[] = {
        datapipereport_descriptor
};

uint8_t datapipe_descriptor[] = {
    //
    // Configuration descriptor header.
    //
    9,                          // Size of the configuration descriptor.
    USB_DTYPE_CONFIGURATION,    // Type of this descriptor.
    USBShort(34),               // The total size of this full structure.
    1,                          // The number of interfaces in this
                                // configuration.
    1,                          // The unique value for this configuration.
    5,                          // The string identifier that describes this
                                // configuration.
    USB_CONF_ATTR_SELF_PWR,     // Bus Powered, Self Powered, remote wake up.
    250,                        // The maximum power in 2mA increments.
};

static const tHIDDescriptor datapipehid_descriptor = {
    9, // bLength
    USB_HID_DTYPE_HID, // bDescriptorType
    0x111, // bcdHID (version 1.11 compliant)
    0, // bCountryCode (not localized)
    1, // bNumDescriptors
    USB_HID_DTYPE_REPORT, // Report descriptor
    sizeof(datapipereport_descriptor) // Size of report descriptor
};

uint8_t hidinterface_descriptor[HIDINTERFACE_SIZE] =
{
    //
    // HID Device Class Interface Descriptor.
    //
    9,                          // Size of the interface descriptor.
    USB_DTYPE_INTERFACE,        // Type of this descriptor.
    0,                          // The index for this interface.
    0,                          // The alternate setting for this interface.
    1,                          // The number of endpoints used by this
                                // interface.
    USB_CLASS_HID,              // The interface class
    USB_HID_SCLASS_NONE,        // The interface sub-class.
    USB_HID_PROTOCOL_NONE,     // The interface protocol for the sub-class
                                // specified above.
    5,                          // The string index for this interface.
};

const uint8_t interruptINendpoint_descriptor[HIDINENDPOINT_SIZE] =
{
    //
    // Interrupt IN endpoint descriptor
    //
    7,                          // The size of the endpoint descriptor.
    USB_DTYPE_ENDPOINT,         // Descriptor type is an endpoint.
    USB_EP_DESC_IN | USBEPToIndex(USB_EP_1),
    USB_EP_ATTR_INT,            // Endpoint is an interrupt endpoint.
    USBShort(USBFIFOSizeToBytes(USB_FIFO_SZ_64)),
                                // The maximum packet size.
    250,                         // The polling interval for this endpoint.
    							//either 1ms for low/full speed devices and 125us for high speed devices
};

// Not needed until the out interrupt is enabled
// TODO: enable by adding this array to g_psHIDSections
//      and setting g_sHIDDataPipeDevice.bUseOutEndpoint to true
//
const uint8_t interruptOUTendpoint_descriptor[HIDOUTENDPOINT_SIZE] =
{
    //
    // Interrupt Out endpoint descriptor
    //
    7,                          // The size of the endpoint descriptor.
    USB_DTYPE_ENDPOINT,         // Descriptor type is an endpoint.
    USB_EP_DESC_OUT | USBEPToIndex(USB_EP_1),
    USB_EP_ATTR_INT,            // Endpoint is an interrupt endpoint.
    USBShort(USBFIFOSizeToBytes(USB_FIFO_SZ_64)),
                                // The maximum packet size.
    16,                         // The polling interval for this endpoint.
};

//-------------------------------------------------------------------------------------------------------------------
const tConfigSection hidconfig_section = {
    sizeof(datapipe_descriptor),
    datapipe_descriptor
};

const tConfigSection hidinterface_section = {
    sizeof(hidinterface_descriptor),
    hidinterface_descriptor
};

const tConfigSection hidINendpoint_section = {
    sizeof(interruptINendpoint_descriptor),
    interruptINendpoint_descriptor
};

const tConfigSection hidOUTendpoint_section = {
    sizeof(interruptOUTendpoint_descriptor),
    interruptOUTendpoint_descriptor
};

tConfigSection hiddescriptor_section = {
   sizeof(datapipehid_descriptor),
   (const uint8_t *)&datapipehid_descriptor
};
//-------------------------------------------------------------------------------------------------------------------

const tConfigSection *hid_sections[] = {
    &hidconfig_section,
    &hidinterface_section,
    &hiddescriptor_section,
    &hidINendpoint_section,
//    &hidOUTendpoint_section,
};

tConfigHeader hidconfig_header = {
    NUM_HID_SECTIONS,
    hid_sections
};

const tConfigHeader * const hidconfig_descriptors[] = {
    &hidconfig_header
};


tHIDReportIdle report_idle[1] = {
        { 125, 0, 0, 0 } // Report 0 polled every 500 ms (4*125)
//        { 0, 0, 0, 0 } // Report 0 never polled
};

tUSBDHIDDevice hiddatapipe_device = {
    //
    // Stealing the MSP430 id so it works with the java app
    //
    //
    USB_VID,
    //
    // The product ID you have assigned for this device.
    //
    USB_PID,
    //
    // The power consumption of your device in milliamps.
    //
    250,
    //
    // The value to be passed to the host in the USB configuration descriptor’s
    // bmAttributes field.
    //
    USB_CONF_ATTR_BUS_PWR,
    //
    // This mouse supports the boot subclass.
    //
    USB_HID_SCLASS_NONE,
    //
    // This device supports the BIOS mouse report protocol.
    //
    USB_HID_PROTOCOL_NONE,
    //
    // The device has a single input report.
    //
    1,
    //
    // A pointer to our array of tHIDReportIdle structures. For this device,
    // the array must have 1 element (matching the value of the previous field).
    //
    report_idle,
    //
    // A pointer to your receive callback event handler.
    //
    rxhandler,
    //
    // A value that you want passed to the receive callback alongside every
    // event.
    //
    (void *)&rxdata,
    //
    // A pointer to your transmit callback event handler.
    //
    txhandler,
    //
    // A value that you want passed to the transmit callback alongside every
    // event.
    //
    (void *)&txdata,
    //
    // This device does not want to use a dedicated interrupt OUT endpoint
    // since there are no output or feature reports required.
    //
    false,
    //
    // A pointer to the HID descriptor for the device.
    //
    &datapipehid_descriptor,
    //
    // A pointer to the array of HID class descriptor pointers for this device.
    // The number of elements in this array and their order must match the
    // information in the HID descriptor provided above.
    //
    datapipeclass_descriptors,
    //
    // A pointer to your string table.
    //
    string_descriptors,
    //
    // The number of entries in your string table. This must equal
    // (1 + (5 + (num HID strings)) * (num languages)).
    //
    NUM_STRING_DESCRIPTORS,

    // Config section
    hidconfig_descriptors

};
