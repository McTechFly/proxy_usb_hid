#include "ep0.h"
#include "usb_descriptors.h"
#include "usb_debug.h"
#include "usb_hid.h"
#include "input_mapping.h"
#include "usb_raw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

// Structures pour les transferts de contrôle EP0
struct usb_raw_control_event {
    struct usb_raw_event inner;
    struct usb_ctrlrequest ctrl;
};

struct usb_raw_control_io {
    struct usb_raw_ep_io inner;
    char data[256]; // Taille maximale pour EP0
};

volatile bool keep_running = true; // Variable de contrôle globale

void stop_ep0_loop(void) {
    keep_running = false;
}

void start_ep0_loop(void) {
    keep_running = true;
}

// Fonction interne de traitement d'une requête sur EP0
static int ep0_request(int fd, struct usb_raw_control_event *event, struct usb_raw_control_io *io) {
    switch (event->ctrl.bRequestType & USB_TYPE_MASK) {
        case USB_TYPE_STANDARD:
            switch (event->ctrl.bRequest) {
                case USB_REQ_GET_DESCRIPTOR:
                    switch (event->ctrl.wValue >> 8) {
                        case USB_DT_DEVICE:
                            memcpy(io->data, &usb_device, sizeof(usb_device));
                            io->inner.length = sizeof(usb_device);
                            return 1;
                        case USB_DT_DEVICE_QUALIFIER:
                            memcpy(io->data, &usb_qualifier, sizeof(usb_qualifier));
                            io->inner.length = sizeof(usb_qualifier);
                            return 1;
                        case USB_DT_CONFIG:
                            io->inner.length = build_config(io->data, sizeof(io->data), 0);
                            return 1;
                        case USB_DT_OTHER_SPEED_CONFIG:
                            io->inner.length = build_config(io->data, sizeof(io->data), 1);
                            return 1;
                        case USB_DT_STRING: {
                             uint8_t index = event->ctrl.wValue & 0xff;
                             if (index == STRING_ID_LANG) {
                                 io->data[0] = 4;
                                 io->data[1] = USB_DT_STRING;
                                 io->data[2] = 0x09;
                                 io->data[3] = 0x04;
                                 io->inner.length = 4;
                             } else if (index == STRING_ID_PRODUCT) {
                                 const char *prod = "Composite Joystick";
                                 int len = strlen(prod);
                                 int desc_len = 2 + len * 2;
                                 if (desc_len > (int)sizeof(io->data))
                                     desc_len = sizeof(io->data);
                                 io->data[0] = desc_len;
                                 io->data[1] = USB_DT_STRING;
                                 for (int i = 0; i < len; i++) {
                                     io->data[2 + i*2] = prod[i];
                                     io->data[2 + i*2+1] = 0;
                                 }
                                 io->inner.length = desc_len;
                             } else if (index == STRING_ID_MANUFACTURER) {
                                 const char *manuf = "MyManufacturer";
                                 int len = strlen(manuf);
                                 int desc_len = 2 + len * 2;
                                 if (desc_len > (int)sizeof(io->data))
                                     desc_len = sizeof(io->data);
                                 io->data[0] = desc_len;
                                 io->data[1] = USB_DT_STRING;
                                 for (int i = 0; i < len; i++) {
                                     io->data[2 + i*2] = manuf[i];
                                     io->data[2 + i*2+1] = 0;
                                 }
                                 io->inner.length = desc_len;
                             } else if (index == STRING_ID_SERIAL) {
                                 const char *serial = "0001";
                                 int len = strlen(serial);
                                 int desc_len = 2 + len * 2;
                                 if (desc_len > (int)sizeof(io->data))
                                     desc_len = sizeof(io->data);
                                 io->data[0] = desc_len;
                                 io->data[1] = USB_DT_STRING;
                                 for (int i = 0; i < len; i++) {
                                     io->data[2 + i*2] = serial[i];
                                     io->data[2 + i*2+1] = 0;
                                 }
                                 io->inner.length = desc_len;
                             } else if (index == STRING_ID_INTERFACE0) {
                                 const char *iface0 = "Composite Joystick 0";
                                 int len = strlen(iface0);
                                 int desc_len = 2 + len * 2;
                                 if (desc_len > (int)sizeof(io->data))
                                     desc_len = sizeof(io->data);
                                 io->data[0] = desc_len;
                                 io->data[1] = USB_DT_STRING;
                                 for (int i = 0; i < len; i++) {
                                     io->data[2 + i*2] = iface0[i];
                                     io->data[2 + i*2+1] = 0;
                                 }
                                 io->inner.length = desc_len;
                             } else if (index == STRING_ID_INTERFACE1) {
                                 const char *iface1 = "Composite Joystick 1";
                                 int len = strlen(iface1);
                                 int desc_len = 2 + len * 2;
                                 if (desc_len > (int)sizeof(io->data))
                                     desc_len = sizeof(io->data);
                                 io->data[0] = desc_len;
                                 io->data[1] = USB_DT_STRING;
                                 for (int i = 0; i < len; i++) {
                                     io->data[2 + i*2] = iface1[i];
                                     io->data[2 + i*2+1] = 0;
                                 }
                                 io->inner.length = desc_len;
                             } else {
                                 io->data[0] = 2;
                                 io->data[1] = USB_DT_STRING;
                                 io->inner.length = 2;
                             }
                             return 1;
                        }
                        case HID_DT_REPORT: {
                            if (event->ctrl.wIndex == 0) {
                                memcpy(io->data, usb_hid_report0, usb_hid_report0_size);
                                io->inner.length = usb_hid_report0_size;
                            } else {
                                memcpy(io->data, usb_hid_report1, usb_hid_report1_size);
                                io->inner.length = usb_hid_report1_size;
                            }
                            return 1;
                        }
                        default:
                            printf("ep0_request: unknown descriptor type: 0x%x\n", event->ctrl.wValue >> 8);
                            return 0;
                    }
                    break;
                case USB_REQ_SET_CONFIGURATION: {
                    extern int ep_int_in0, ep_int_in1;
                    ep_int_in0 = usb_raw_ep_enable(fd, &usb_endpoint0);
                    ep_int_in1 = usb_raw_ep_enable(fd, &usb_endpoint1);
                    printf("ep0_request: endpoints enabled: ep_int_in0 = %d, ep_int_in1 = %d\n", ep_int_in0, ep_int_in1);
                    
                    // Démarrage du thread HID
                    HidReportArgs *args = malloc(sizeof(HidReportArgs));
                    if (!args) {
                        perror("malloc");
                        exit(EXIT_FAILURE);
                    }
                    extern InputDevice *g_devices;
                    extern int g_nb_joysticks;
                    args->fd = fd;
                    args->devices = g_devices;
                    args->nb_joysticks = g_nb_joysticks;
                    pthread_t hid_thread;
                    int rv = pthread_create(&hid_thread, NULL, process_and_send_hid_reports, args);
                    if (rv != 0) {
                        perror("pthread_create");
                        exit(EXIT_FAILURE);
                    }
                    pthread_detach(hid_thread);
                    
                    usb_raw_vbus_draw(fd, usb_config.bMaxPower);
                    usb_raw_configure(fd);
                    io->inner.length = 0;
                    return 1;
                }
                case USB_REQ_GET_INTERFACE:
                    io->data[0] = 0;
                    io->inner.length = 1;
                    return 1;
                default:
                    printf("ep0_request: unsupported standard request 0x%x\n", event->ctrl.bRequest);
                    return 0;
            }
            break;
        case USB_TYPE_CLASS:
            switch (event->ctrl.bRequest) {
                case HID_REQ_SET_REPORT:
                    io->inner.length = 1;
                    return 1;
                case HID_REQ_SET_IDLE:
                    io->inner.length = 0;
                    return 1;
                case HID_REQ_SET_PROTOCOL:
                    io->inner.length = 0;
                    return 1;
                default:
                    printf("ep0_request: unsupported class request 0x%x\n", event->ctrl.bRequest);
                    return 0;
            }
            break;
        default:
            printf("ep0_request: unknown request type\n");
            return 0;
    }
    return 0;
}

void ep0_loop(int fd) {
    while (keep_running) { // La boucle s'exécute tant que keep_running est true
        struct usb_raw_control_event event;
        event.inner.type = 0;
        event.inner.length = sizeof(event.ctrl);
        usb_raw_event_fetch(fd, (struct usb_raw_event *)&event);
        log_event((struct usb_raw_event *)&event);
        if (event.inner.type == USB_RAW_EVENT_CONNECT)
            continue;
        if (event.inner.type == USB_RAW_EVENT_RESET)
            continue;
        if (event.inner.type != USB_RAW_EVENT_CONTROL)
            continue;
        
        struct usb_raw_control_io io;
        memset(&io, 0, sizeof(io));
        io.inner.ep = 0;
        io.inner.flags = 0;
        io.inner.length = 0;
        int reply = ep0_request(fd, &event, &io);
        if (!reply) {
            printf("ep0: stalling\n");
            usb_raw_ep0_stall(fd);
            continue;
        }
        if (event.ctrl.wLength < io.inner.length)
            io.inner.length = event.ctrl.wLength;
        if (event.ctrl.bRequestType & USB_DIR_IN) {
            int rv = usb_raw_ep0_write(fd, (struct usb_raw_ep_io *)&io);
            printf("ep0: transferred %d bytes (in)\n", rv);
        } else {
            int rv = usb_raw_ep0_read(fd, (struct usb_raw_ep_io *)&io);
            printf("ep0: transferred %d bytes (out)\n", rv);
        }
    }
    printf("ep0_loop stoppé.\n");
}
