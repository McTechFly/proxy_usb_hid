#ifndef USB_DEBUG_H
#define USB_DEBUG_H

#include <linux/usb/ch9.h>
#include "usb_raw.h"

// Prototypes des fonctions de log USB
void log_control_request(struct usb_ctrlrequest *ctrl);
void log_event(struct usb_raw_event *event);

#endif // USB_DEBUG_H
