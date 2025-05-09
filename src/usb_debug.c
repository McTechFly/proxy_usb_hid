#include "usb_debug.h"
#include <stdio.h>

void log_control_request(struct usb_ctrlrequest *ctrl) {
    printf("  bRequestType: 0x%x, bRequest: 0x%x, wValue: 0x%x, wIndex: 0x%x, wLength: %d\n",
           ctrl->bRequestType, ctrl->bRequest, ctrl->wValue, ctrl->wIndex, ctrl->wLength);
}

void log_event(struct usb_raw_event *event) {
    switch (event->type) {
        case USB_RAW_EVENT_CONNECT:
            printf("event: connect, length: %u\n", event->length);
            break;
        case USB_RAW_EVENT_CONTROL:
            printf("event: control, length: %u\n", event->length);
            log_control_request((struct usb_ctrlrequest *)&event->data[0]);
            break;
        case USB_RAW_EVENT_SUSPEND:
            printf("event: suspend\n");
            break;
        case USB_RAW_EVENT_RESUME:
            printf("event: resume\n");
            break;
        case USB_RAW_EVENT_RESET:
            printf("event: reset\n");
            break;
        case USB_RAW_EVENT_DISCONNECT:
            printf("event: disconnect\n");
            break;
        default:
            printf("event: %d (unknown), length: %u\n", event->type, event->length);
    }
}
