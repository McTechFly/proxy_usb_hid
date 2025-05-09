#ifndef USB_HID_H
#define USB_HID_H

#include "usb_raw.h"
#include "usb_descriptors.h"
#include <pthread.h>

// Structure d'arguments pour le thread HID
typedef struct {
    int fd;                   // Descripteur du gadget USB
    void *devices;            // Tableau des périphériques d'entrée (voir input_mapping.h)
    int nb_joysticks;         // Nombre de périphériques
} HidReportArgs;

// Prototype de la fonction de traitement des rapports HID
void *process_and_send_hid_reports(void *arg);


#endif // USB_HID_H
