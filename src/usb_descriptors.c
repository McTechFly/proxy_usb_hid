#include "usb_descriptors.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

// Définition des rapports HID pour Joystick 0 et 1
const unsigned char usb_hid_report0[] = {
    0x05, 0x01,            // Usage Page (Generic Desktop)
    0x09, 0x04,            // Usage (Joystick)
    0xA1, 0x01,            // Collection (Application)
    0x85, 0x01,            // Report ID (1)
    0x16, 0x00, 0x80,       // Logical Minimum (-32768)
    0x26, 0xFF, 0x7F,       // Logical Maximum (32767)
    0x75, 0x10,            // Report Size (16)
    0x95, 0x08,            // Report Count (8 axes)
    0x09, 0x30,            // Usage (X)
    0x09, 0x31,            // Usage (Y)
    0x09, 0x32,            // Usage (Z)
    0x09, 0x33,            // Usage (Rx)
    0x09, 0x34,            // Usage (Ry)
    0x09, 0x35,            // Usage (Rz)
    0x09, 0x36,            // Usage (Slider)
    0x09, 0x37,            // Usage (Dial)
    0x81, 0x02,            // Input (Data,Var,Abs) [Axes]
    0x05, 0x09,            // Usage Page (Button)
    0x19, 0x01,            // Usage Minimum (Button 1)
    0x29, 0x80,            // Usage Maximum (Button 128)
    0x15, 0x00,            // Logical Minimum (0)
    0x25, 0x01,            // Logical Maximum (1)
    0x75, 0x01,            // Report Size (1)
    0x95, 0x80,            // Report Count (128 boutons)
    0x81, 0x02,            // Input (Data,Var,Abs) [Boutons]
    0xC0                   // End Collection
};

const unsigned char usb_hid_report1[] = {
    0x05, 0x01,            // Usage Page (Generic Desktop)
    0x09, 0x04,            // Usage (Joystick)
    0xA1, 0x01,            // Collection (Application)
    0x85, 0x02,            // Report ID (2)
    0x16, 0x00, 0x80,       // Logical Minimum (-32768)
    0x26, 0xFF, 0x7F,       // Logical Maximum (32767)
    0x75, 0x10,            // Report Size (16)
    0x95, 0x08,            // Report Count (8 axes)
    0x09, 0x30,            // Usage (X)
    0x09, 0x31,            // Usage (Y)
    0x09, 0x32,            // Usage (Z)
    0x09, 0x33,            // Usage (Rx)
    0x09, 0x34,            // Usage (Ry)
    0x09, 0x35,            // Usage (Rz)
    0x09, 0x36,            // Usage (Slider)
    0x09, 0x37,            // Usage (Dial)
    0x81, 0x02,            // Input (Data,Var,Abs) [Axes]
    0x05, 0x09,            // Usage Page (Button)
    0x19, 0x01,            // Usage Minimum (Button 1)
    0x29, 0x80,            // Usage Maximum (Button 128)
    0x15, 0x00,            // Logical Minimum (0)
    0x25, 0x01,            // Logical Maximum (1)
    0x75, 0x01,            // Report Size (1)
    0x95, 0x80,            // Report Count (128 boutons)
    0x81, 0x02,            // Input (Data,Var,Abs) [Boutons]
    0xC0                   // End Collection
};

// Définition des tailles des rapports HID
const unsigned int usb_hid_report0_size = sizeof(usb_hid_report0);
const unsigned int usb_hid_report1_size = sizeof(usb_hid_report1);

// Descripteur HID pour le Joystick 0
struct hid_descriptor usb_hid0 = {
    .bLength = 9,
    .bDescriptorType = HID_DT_HID,
    .bcdHID = __constant_cpu_to_le16(0x0110),
    .bCountryCode = 0,
    .bNumDescriptors = 1,
    .desc = {
        {
            .bDescriptorType = HID_DT_REPORT,
            .wDescriptorLength = __constant_cpu_to_le16(usb_hid_report0_size),
        }
    },
};

// Descripteur HID pour le Joystick 1
struct hid_descriptor usb_hid1 = {
    .bLength = 9,
    .bDescriptorType = HID_DT_HID,
    .bcdHID = __constant_cpu_to_le16(0x0110),
    .bCountryCode = 0,
    .bNumDescriptors = 1,
    .desc = {
        {
            .bDescriptorType = HID_DT_REPORT,
            .wDescriptorLength = __constant_cpu_to_le16(usb_hid_report1_size),
        }
    },
};

// Descripteurs d'interface USB
struct usb_interface_descriptor usb_interface0 = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_HID,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = STRING_ID_INTERFACE0,
};

struct usb_interface_descriptor usb_interface1 = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 1,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_HID,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = STRING_ID_INTERFACE1,
};

// Descripteurs d'endpoint pour les deux interfaces
struct usb_endpoint_descriptor usb_endpoint0 = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = USB_DIR_IN | EP_NUM_INT_IN0,
    .bmAttributes = USB_ENDPOINT_XFER_INT,
    .wMaxPacketSize = __cpu_to_le16(33),
    .bInterval = 1,
};

struct usb_endpoint_descriptor usb_endpoint1 = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = USB_DIR_IN | EP_NUM_INT_IN1,
    .bmAttributes = USB_ENDPOINT_XFER_INT,
    .wMaxPacketSize = __cpu_to_le16(33),
    .bInterval = 1,
};

// Descripteur de périphérique USB
struct usb_device_descriptor usb_device = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = __constant_cpu_to_le16(BCD_USB),
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = EP_MAX_PACKET_CONTROL,
    .idVendor = __constant_cpu_to_le16(USB_VENDOR),
    .idProduct = __constant_cpu_to_le16(USB_PRODUCT),
    .bcdDevice = __constant_cpu_to_le16(0x0100),
    .iManufacturer = STRING_ID_MANUFACTURER,
    .iProduct = STRING_ID_PRODUCT,
    .iSerialNumber = STRING_ID_SERIAL,
    .bNumConfigurations = 1,
};

// Descripteur qualifier
struct usb_qualifier_descriptor usb_qualifier = {
    .bLength = sizeof(struct usb_qualifier_descriptor),
    .bDescriptorType = USB_DT_DEVICE_QUALIFIER,
    .bcdUSB = __constant_cpu_to_le16(BCD_USB),
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = EP_MAX_PACKET_CONTROL,
    .bNumConfigurations = 1,
    .bRESERVED = 0,
};

// Descripteur de configuration USB
struct usb_config_descriptor usb_config = {
    .bLength = USB_DT_CONFIG_SIZE,
    .bDescriptorType = USB_DT_CONFIG,
    .wTotalLength = 0, // Calculé dynamiquement
    .bNumInterfaces = 2,
    .bConfigurationValue = 1,
    .iConfiguration = STRING_ID_CONFIG,
    .bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
    .bMaxPower = 0x32,
};

int build_config(char *data, int length, int other_speed) {
    struct usb_config_descriptor *config_desc = (struct usb_config_descriptor *)data;
    int total_length = 0;
    
    assert(length >= (int)sizeof(usb_config));
    memcpy(data, &usb_config, sizeof(usb_config));
    data += sizeof(usb_config);
    length -= sizeof(usb_config);
    total_length += sizeof(usb_config);
    
    // Interface 0 + HID + endpoint
    assert(length >= (int)sizeof(usb_interface0));
    memcpy(data, &usb_interface0, sizeof(usb_interface0));
    data += sizeof(usb_interface0);
    length -= sizeof(usb_interface0);
    total_length += sizeof(usb_interface0);
    
    assert(length >= (int)sizeof(usb_hid0));
    memcpy(data, &usb_hid0, sizeof(usb_hid0));
    data += sizeof(usb_hid0);
    length -= sizeof(usb_hid0);
    total_length += sizeof(usb_hid0);
    
    assert(length >= (int)USB_DT_ENDPOINT_SIZE);
    memcpy(data, &usb_endpoint0, USB_DT_ENDPOINT_SIZE);
    data += USB_DT_ENDPOINT_SIZE;
    length -= USB_DT_ENDPOINT_SIZE;
    total_length += USB_DT_ENDPOINT_SIZE;
    
    // Interface 1 + HID + endpoint
    assert(length >= (int)sizeof(usb_interface1));
    memcpy(data, &usb_interface1, sizeof(usb_interface1));
    data += sizeof(usb_interface1);
    length -= sizeof(usb_interface1);
    total_length += sizeof(usb_interface1);
    
    assert(length >= (int)sizeof(usb_hid1));
    memcpy(data, &usb_hid1, sizeof(usb_hid1));
    data += sizeof(usb_hid1);
    length -= sizeof(usb_hid1);
    total_length += sizeof(usb_hid1);
    
    assert(length >= (int)USB_DT_ENDPOINT_SIZE);
    memcpy(data, &usb_endpoint1, USB_DT_ENDPOINT_SIZE);
    data += USB_DT_ENDPOINT_SIZE;
    length -= USB_DT_ENDPOINT_SIZE;
    total_length += USB_DT_ENDPOINT_SIZE;
    
    config_desc->wTotalLength = __cpu_to_le16(total_length);
    
    if (other_speed)
        config_desc->bDescriptorType = USB_DT_OTHER_SPEED_CONFIG;
    
    printf("Composite config wTotalLength: %d\n", total_length);
    return total_length;
}
