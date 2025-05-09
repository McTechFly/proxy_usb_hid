#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <linux/usb/ch9.h>
#include <linux/hid.h>

#ifdef __cplusplus
extern "C" {
#endif

// Conversion Little-endian
#ifndef __cpu_to_le16
#define __cpu_to_le16(x) ((uint16_t)(x))
#endif
#ifndef __constant_cpu_to_le16
#define __constant_cpu_to_le16(x) ((uint16_t)(x))
#endif

// Constantes USB
#define BCD_USB         0x0200
#define USB_VENDOR      0x1d6b
#define USB_PRODUCT     0x0101
#define EP_MAX_PACKET_CONTROL   64
#define USB_SPEED_HIGH  2

// Identifiants de chaîne USB
#define STRING_ID_LANG           0
#define STRING_ID_MANUFACTURER   1
#define STRING_ID_PRODUCT        2
#define STRING_ID_SERIAL         5
#define STRING_ID_CONFIG         4
#define STRING_ID_INTERFACE0     5
#define STRING_ID_INTERFACE1     6

// Numérotation des endpoints pour les rapports HID
#define EP_NUM_INT_IN0   1
#define EP_NUM_INT_IN1   2

// Nombre maximum de boutons
#define MAX_BUTTONS 128

// Structures HID (pour le descripteur HID)
struct hid_class_descriptor {
    uint8_t  bDescriptorType;
    __le16 wDescriptorLength;
} __attribute__ ((packed));

struct hid_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    __le16 bcdHID;
    uint8_t  bCountryCode;
    uint8_t  bNumDescriptors;
    struct hid_class_descriptor desc[1];
} __attribute__ ((packed));

// Déclarations des rapports HID et de leurs tailles
extern const unsigned char usb_hid_report0[];
extern const unsigned int usb_hid_report0_size;
extern const unsigned char usb_hid_report1[];
extern const unsigned int usb_hid_report1_size;

// Déclarations externes des descripteurs USB
extern struct hid_descriptor usb_hid0;
extern struct hid_descriptor usb_hid1;
extern struct usb_device_descriptor usb_device;
extern struct usb_config_descriptor usb_config;
extern struct usb_qualifier_descriptor usb_qualifier;
extern struct usb_interface_descriptor usb_interface0;
extern struct usb_interface_descriptor usb_interface1;
extern struct usb_endpoint_descriptor usb_endpoint0;
extern struct usb_endpoint_descriptor usb_endpoint1;

// Prototype de la fonction de construction dynamique de la configuration USB
int build_config(char *data, int length, int other_speed);

#ifdef __cplusplus
}
#endif

#endif // USB_DESCRIPTORS_H
