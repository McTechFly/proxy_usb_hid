#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <limits.h>
#include "usb_raw.h"
#include "usb_descriptors.h"
#include "ep0.h"
#include "input_mapping.h"

// Déclaration globale des périphériques utilisés par le mapping
InputDevice *g_devices = NULL;
int g_nb_joysticks = 0;

// Variables globales pour les endpoints HID (utilisées par usb_hid.c et ep0.c)
int ep_int_in0 = -1;
int ep_int_in1 = -1;

int main(int argc, char **argv) {
    const char *device = "dummy_udc.0";
    const char *driver = "dummy_udc";
    if (argc >= 2)
        device = argv[1];
    if (argc >= 3)
        driver = argv[2];
    {
        char exe_path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
        if (len < 0) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }
        exe_path[len] = '\0';
        char *dir = dirname(exe_path);
        snprintf(g_mapping_file, sizeof(g_mapping_file), "%s/mapping/mapping.json", dir);
        printf("Chemin du mapping: %s\n", g_mapping_file);
    }
    int fd = usb_raw_open();
    usb_raw_init(fd, USB_SPEED_HIGH, driver, device);
    usb_raw_run(fd);
    InputDevice *devices = NULL;
    int nb_joysticks = 0;
    init_physical_devices_wrapper(&devices, &nb_joysticks);
    printf("Total axes trouvés: %d\n", global_axis_index);
    if (nb_joysticks == 0) {
        printf("Aucun joystick/gamepad trouvé.\n");
        free(devices);
        close(fd);
        return 1;
    }
    g_devices = devices;
    g_nb_joysticks = nb_joysticks;
    ep0_loop(fd);
    for (int i = 0; i < nb_joysticks; i++) {
        close(devices[i].fd);
    }
    free(devices);
    close(fd);
    return 0;
}
