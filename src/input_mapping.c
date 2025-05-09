/**
 * @file input_mapping.c
 * @brief This file contains functions and utilities for managing input device mappings,
 *        including saving and loading mappings, detecting input devices, and handling
 *        HID raw devices for button parsing.
 *
 * @details
 * The code provides functionality to:
 * - Parse HID raw devices to extract button codes.
 * - Find HID raw devices corresponding to specific input devices.
 * - Save and load input device mappings to/from JSON files.
 * - Initialize and merge detected input devices with saved mappings.
 * - Handle global axis and button indices for virtual joystick mappings.
 *
 * Dependencies:
 * - Linux-specific headers for input device handling (`linux/input.h`, `linux/hidraw.h`).
 * - JSON-C library for JSON manipulation.
 * - Standard C libraries for file I/O, memory management, and string handling.
 *
 * Global Variables:
 * - `g_devices`: Pointer to the global array of input devices.
 * - `g_nb_joysticks`: Number of joysticks detected.
 * - `global_axis_index`: Global index for axis mapping.
 * - `global_button_index`: Global index for button mapping.
 * - `g_mapping_file`: Path to the JSON file used for saving/loading mappings.
 *
 * Functions:
 * - `int parse_hidraw_buttons(const char *hidraw_path, int *button_codes, int max_buttons)`: 
 *   Parses a HID raw device to extract button codes.
 * - `int find_hidraw_for_device(InputDevice *dev, char *hidraw_path, size_t hidraw_path_len)`:
 *   Finds the HID raw device corresponding to a given input device.
 * - `bool save_mapping(const char *filename, InputDevice *devices, int nb_joysticks, int global_axis, int global_button)`:
 *   Saves the input device mappings to a JSON file.
 * - `bool load_mapping(const char *filename, InputDevice **devices, int *nb_joysticks, int *global_axis, int *global_button)`:
 *   Loads input device mappings from a JSON file.
 * - `void init_physical_devices_wrapper(InputDevice **final_devices, int *nb_final)`:
 *   Initializes and merges detected input devices with saved mappings, and saves the updated mapping.
 *
 * Usage:
 * - The functions in this file are designed to work with Linux input devices and require
 *   appropriate permissions to access `/dev/input` and `/dev/hidraw` devices.
 * - The `init_physical_devices_wrapper` function serves as the main entry point for initializing
 *   and managing input device mappings.
 */
#include "input_mapping.h"
#include "usb_descriptors.h"    // Pour MAX_BUTTONS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <json-c/json.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/hidraw.h>        // Pour HIDIOCGRDESCSIZE, HIDIOCGRDESC, struct hidraw_devinfo

// Variables globales de mapping
extern InputDevice *g_devices;
extern int g_nb_joysticks;
int global_axis_index = 0;
int global_button_index = 0;
char g_mapping_file[PATH_MAX] = {0};

int parse_hidraw_buttons(const char *hidraw_path, int *button_codes, int max_buttons) {
    int fd = open(hidraw_path, O_RDONLY);
    if (fd < 0) return -1;
    int desc_size;
    if (ioctl(fd, HIDIOCGRDESCSIZE, &desc_size) < 0) {
        close(fd);
        return -1;
    }
    unsigned char *desc = malloc(desc_size);
    if (!desc) {
        close(fd);
        return -1;
    }
    if (ioctl(fd, HIDIOCGRDESC, desc) < 0) {
        free(desc);
        close(fd);
        return -1;
    }
    int count = 0;
    int usage_page_found = 0;
    int usage_min = -1, usage_max = -1;
    for (int i = 0; i < desc_size - 1; i++) {
         if (desc[i] == 0x05 && desc[i+1] == 0x09) { // Usage Page (Button)
             usage_page_found = 1;
             i++;
         } else if (usage_page_found && desc[i] == 0x19 && i+1 < desc_size) { // Usage Minimum
             usage_min = desc[i+1];
             i++;
         } else if (usage_page_found && desc[i] == 0x29 && i+1 < desc_size) { // Usage Maximum
             usage_max = desc[i+1];
             i++;
             break;
         }
    }
    free(desc);
    close(fd);
    if (usage_page_found && usage_min != -1 && usage_max != -1 && usage_min <= usage_max) {
         for (int u = usage_min; u <= usage_max && count < max_buttons; u++) {
              button_codes[count] = 0x120 + (u - 1);
              count++;
         }
         return count;
    }
    return -1;
}

int find_hidraw_for_device(InputDevice *dev, char *hidraw_path, size_t hidraw_path_len) {
    glob_t glob_hid;
    if (glob("/dev/hidraw*", 0, NULL, &glob_hid) != 0) return -1;
    int ret = -1;
    for (size_t i = 0; i < glob_hid.gl_pathc; i++) {
         int fd = open(glob_hid.gl_pathv[i], O_RDONLY);
         if (fd < 0) continue;
         struct hidraw_devinfo info;
         memset(&info, 0, sizeof(info));
         if (ioctl(fd, HIDIOCGRAWINFO, &info) == 0) {
              if (info.vendor == dev->id.vendor &&
                  info.product == dev->id.product &&
                  info.bustype == dev->id.bustype) {
                  strncpy(hidraw_path, glob_hid.gl_pathv[i], hidraw_path_len-1);
                  hidraw_path[hidraw_path_len-1] = '\0';
                  ret = 0;
                  close(fd);
                  break;
              }
         }
         close(fd);
    }
    globfree(&glob_hid);
    return ret;
}

bool save_mapping(const char *filename, InputDevice *devices, int nb_joysticks, int global_axis, int global_button) {
    json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "global_axis_index", json_object_new_int(global_axis));
    json_object_object_add(jobj, "global_button_index", json_object_new_int(global_button));
    json_object *jdevices = json_object_new_array();
    for (int i = 0; i < nb_joysticks; i++) {
        json_object *jdev = json_object_new_object();
        json_object_object_add(jdev, "path", json_object_new_string(devices[i].path));
        json_object_object_add(jdev, "name", json_object_new_string(devices[i].name));
        json_object_object_add(jdev, "bustype", json_object_new_int(devices[i].id.bustype));
        json_object_object_add(jdev, "vendor", json_object_new_int(devices[i].id.vendor));
        json_object_object_add(jdev, "product", json_object_new_int(devices[i].id.product));
        json_object_object_add(jdev, "version", json_object_new_int(devices[i].id.version));
        json_object_object_add(jdev, "num_axes", json_object_new_int(devices[i].num_axes));
        json_object_object_add(jdev, "num_buttons", json_object_new_int(devices[i].num_buttons));
        
        json_object *jaxes = json_object_new_array();
        for (int code = 0; code < ABS_CNT; code++) {
            if (!devices[i].has_abs[code])
                continue;
            json_object *axobj = json_object_new_object();
            json_object_object_add(axobj, "code", json_object_new_int(code));
            json_object_object_add(axobj, "mapped_axis", json_object_new_int(devices[i].axis_mapping[code]));
            json_object_object_add(axobj, "dead_zone", json_object_new_int(devices[i].axis_dead_zone[code]));
            json_object_object_add(axobj, "invert", json_object_new_boolean(devices[i].axis_invert[code] != 0));
            json_object_object_add(axobj, "virtual_joystick", json_object_new_int(devices[i].axis_virtual_joystick[code]));
            json_object_object_add(axobj, "virtual_axis", json_object_new_int(devices[i].axis_virtual_axis[code]));
            json_object_array_add(jaxes, axobj);
        }
        json_object_object_add(jdev, "axes", jaxes);
        
        json_object *jbuttons = json_object_new_object();
        for (int code = 0; code <= KEY_MAX; code++) {
            if (!devices[i].has_button[code])
                continue;
            json_object *btnobj = json_object_new_object();
            json_object_object_add(btnobj, "mapped_button", json_object_new_int(devices[i].button_mapping[code]));
            json_object_object_add(btnobj, "virtual_joystick", json_object_new_int(devices[i].button_virtual_joystick[code]));
            char code_str[16];
            snprintf(code_str, sizeof(code_str), "%d", code);
            json_object_object_add(jbuttons, code_str, btnobj);
        }
        json_object_object_add(jdev, "buttons", jbuttons);
        
        json_object_array_add(jdevices, jdev);
    }
    json_object_object_add(jobj, "devices", jdevices);
    int rc = json_object_to_file_ext(filename, jobj, JSON_C_TO_STRING_PRETTY);
    json_object_put(jobj);
    return (rc == 0);
}

bool load_mapping(const char *filename, InputDevice **devices, int *nb_joysticks, int *global_axis, int *global_button) {
    FILE *f = fopen(filename, "r");
    if (!f) return false;
    fclose(f);
    json_object *jobj = json_object_from_file(filename);
    if (!jobj) return false;
    json_object *jglobal_axis = NULL;
    json_object *jglobal_button = NULL;
    json_object_object_get_ex(jobj, "global_axis_index", &jglobal_axis);
    json_object_object_get_ex(jobj, "global_button_index", &jglobal_button);
    if (jglobal_axis)
        *global_axis = json_object_get_int(jglobal_axis);
    else
        *global_axis = 0;
    if (jglobal_button)
        *global_button = json_object_get_int(jglobal_button);
    else
        *global_button = 0;
    json_object *jdevices = NULL;
    if (!json_object_object_get_ex(jobj, "devices", &jdevices)) {
        json_object_put(jobj);
        return false;
    }
    int count = json_object_array_length(jdevices);
    *devices = calloc(count, sizeof(InputDevice));
    *nb_joysticks = count;
    for (int i = 0; i < count; i++) {
        json_object *jdev = json_object_array_get_idx(jdevices, i);
        InputDevice *idev = &(*devices)[i];
        memset(idev, 0, sizeof(InputDevice));
        for (int j = 0; j < ABS_CNT; j++) {
            idev->axis_mapping[j] = -1;
            idev->axis_dead_zone[j] = 0;
            idev->axis_invert[j] = 0;
            idev->axis_virtual_joystick[j] = 0;
            idev->axis_virtual_axis[j] = -1;
        }
        for (int j = 0; j <= KEY_MAX; j++) {
            idev->button_mapping[j] = -1;
            idev->button_virtual_joystick[j] = 0;
            idev->has_button[j] = 0;
        }
        json_object *jpath = json_object_object_get(jdev, "path");
        if (jpath) {
            strncpy(idev->path, json_object_get_string(jpath), sizeof(idev->path)-1);
        }
        json_object *jname = json_object_object_get(jdev, "name");
        if (jname) {
            strncpy(idev->name, json_object_get_string(jname), sizeof(idev->name)-1);
        }
        idev->id.bustype = json_object_get_int(json_object_object_get(jdev, "bustype"));
        idev->id.vendor = json_object_get_int(json_object_object_get(jdev, "vendor"));
        idev->id.product = json_object_get_int(json_object_object_get(jdev, "product"));
        idev->id.version = json_object_get_int(json_object_object_get(jdev, "version"));
        idev->num_axes = json_object_get_int(json_object_object_get(jdev, "num_axes"));
        idev->num_buttons = json_object_get_int(json_object_object_get(jdev, "num_buttons"));
        json_object *jaxes = NULL;
        if (json_object_object_get_ex(jdev, "axes", &jaxes)) {
            int nax = json_object_array_length(jaxes);
            for (int ax = 0; ax < nax; ax++) {
                json_object *axobj = json_object_array_get_idx(jaxes, ax);
                int code = json_object_get_int(json_object_object_get(axobj, "code"));
                int mapped = json_object_get_int(json_object_object_get(axobj, "mapped_axis"));
                if (code >= 0 && code < ABS_CNT) {
                    idev->axis_mapping[code] = mapped;
                }
                json_object *jdz = json_object_object_get(axobj, "dead_zone");
                if (jdz) {
                    int dz = json_object_get_int(jdz);
                    if (dz < 0) dz = 0;
                    if (dz > 32767) dz = 32767;
                    idev->axis_dead_zone[code] = dz;
                }
                json_object *jinvert = json_object_object_get(axobj, "invert");
                if (jinvert) {
                    bool inv = json_object_get_boolean(jinvert);
                    idev->axis_invert[code] = inv ? 1 : 0;
                }
                json_object *jvirt_joy = json_object_object_get(axobj, "virtual_joystick");
                if (jvirt_joy)
                    idev->axis_virtual_joystick[code] = json_object_get_int(jvirt_joy);
                else
                    idev->axis_virtual_joystick[code] = 0;
                json_object *jvirt_axis = json_object_object_get(axobj, "virtual_axis");
                if (jvirt_axis)
                    idev->axis_virtual_axis[code] = json_object_get_int(jvirt_axis);
                else
                    idev->axis_virtual_axis[code] = idev->axis_mapping[code] % 8;
            }
        }
        json_object *jbuttons = NULL;
        if (json_object_object_get_ex(jdev, "buttons", &jbuttons)) {
            json_object_object_foreach(jbuttons, key_str, jval) {
                int code = atoi(key_str);
                json_object *jmappedb, *jvirt;
                if (json_object_object_get_ex(jval, "mapped_button", &jmappedb)) {
                    int mappedb = json_object_get_int(jmappedb);
                    if (code >= 0 && code <= KEY_MAX) {
                        idev->button_mapping[code] = mappedb;
                    }
                }
                if (json_object_object_get_ex(jval, "virtual_joystick", &jvirt)) {
                    idev->button_virtual_joystick[code] = json_object_get_int(jvirt);
                }
            }
        }
        idev->fd = open(idev->path, O_RDONLY | O_NONBLOCK);
        if (idev->fd < 0) {
            perror("Erreur open device from mapping");
        }
    }
    json_object_put(jobj);
    return true;
}

void init_physical_devices_wrapper(InputDevice **final_devices, int *nb_final) {
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
    if (len < 0) {
        perror("readlink");
        exit(EXIT_FAILURE);
    }
    exe_path[len] = '\0';
    char *dir = dirname(exe_path);
    char mapping_file[PATH_MAX];
    snprintf(mapping_file, sizeof(mapping_file), "%s/mapping.json", dir);
    strncpy(g_mapping_file, mapping_file, sizeof(g_mapping_file)-1);
    g_mapping_file[sizeof(g_mapping_file)-1] = '\0';
    
    InputDevice *merged_devices = NULL;
    int merged_count = 0;
    if (access(mapping_file, F_OK) == 0) {
        printf("Fichier de mapping trouvé. Chargement depuis %s\n", mapping_file);
        InputDevice *saved_devices = NULL;
        int saved_count = 0;
        if (!load_mapping(mapping_file, &saved_devices, &saved_count, &global_axis_index, &global_button_index)) {
            printf("Erreur lors du chargement du mapping. Nouveau mapping.\n");
        }
        InputDevice *detected_devices = NULL;
        int detected_count = 0;
        glob_t glob_result;
        if (glob("/dev/input/event*", 0, NULL, &glob_result) != 0) {
            printf("Aucun périphérique evdev trouvé.\n");
            return;
        }
        detected_count = glob_result.gl_pathc;
        detected_devices = malloc(detected_count * sizeof(InputDevice));
        if (!detected_devices) {
            perror("malloc detected_devices");
            globfree(&glob_result);
            exit(EXIT_FAILURE);
        }
        int actual_count = 0;
        for (int i = 0; i < detected_count; i++) {
            InputDevice *dev = &detected_devices[actual_count];
            memset(dev, 0, sizeof(InputDevice));
            strncpy(dev->path, glob_result.gl_pathv[i], sizeof(dev->path)-1);
            dev->path[sizeof(dev->path)-1] = '\0';
            dev->fd = open(dev->path, O_RDONLY | O_NONBLOCK);
            if (dev->fd < 0) {
                perror("Erreur open dev");
                continue;
            }
            if (ioctl(dev->fd, EVIOCGNAME(sizeof(dev->name)), dev->name) < 0) {
                perror("Erreur EVIOCGNAME");
                strncpy(dev->name, "Unknown", sizeof(dev->name)-1);
            }
            if (ioctl(dev->fd, EVIOCGID, &dev->id) < 0) {
                perror("Erreur EVIOCGID");
                memset(&dev->id, 0, sizeof(dev->id));
            }
            if (strstr(dev->name, "vc4-hdmi")) {
                close(dev->fd);
                continue;
            }
            for (int j = 0; j < ABS_CNT; j++) {
                dev->axis_mapping[j] = -1;
                dev->axis_dead_zone[j] = 0;
                dev->axis_invert[j] = 0;
                dev->axis_virtual_joystick[j] = 0;
                dev->axis_virtual_axis[j] = -1;
            }
            for (int j = 0; j <= KEY_MAX; j++) {
                dev->button_mapping[j] = -1;
                dev->button_virtual_joystick[j] = 0;
                dev->has_button[j] = 0;
            }
            unsigned char abs_bitmask[(ABS_CNT/8)+1] = {0};
            if (ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) {
                perror("Erreur EVIOCGBIT(EV_ABS)");
            } else {
                for (int j = 0; j < ABS_CNT; j++) {
                    if (abs_bitmask[j/8] & (1 << (j % 8))) {
                        if (ioctl(dev->fd, EVIOCGABS(j), &dev->absinfo[j]) == 0) {
                            dev->has_abs[j] = 1;
                            dev->axis_mapping[j] = global_axis_index++;
                            dev->num_axes++;
                            if (dev->num_axes <= 8)
                                dev->axis_virtual_axis[j] = dev->num_axes - 1;
                            else
                                dev->axis_virtual_axis[j] = (dev->num_axes - 1) % 8;
                            dev->axis_virtual_joystick[j] = 0;
                        }
                    }
                }
            }
            int taille_bitmask = (KEY_MAX + 7) / 8;
            unsigned char key_bitmask[taille_bitmask];
            memset(key_bitmask, 0, taille_bitmask);
            if (ioctl(dev->fd, EVIOCGBIT(EV_KEY, taille_bitmask), key_bitmask) < 0) {
                perror("Erreur EVIOCGBIT(EV_KEY)");
            } else {
                for (int j = 0; j <= KEY_MAX; j++) {
                    if (key_bitmask[j / 8] & (1 << (j % 8))) {
                        dev->has_button[j] = 1;  // Correction : pas "1_"
                        dev->num_buttons++;
                    }
                }
            }
            {
                char hidraw_path[PATH_MAX];
                if (find_hidraw_for_device(dev, hidraw_path, sizeof(hidraw_path)) == 0) {
                    int button_codes[MAX_BUTTONS];
                    int num = parse_hidraw_buttons(hidraw_path, button_codes, MAX_BUTTONS);
                    if (num > 0) {
                        for (int b = 0; b < num; b++) {
                            int ev_code = button_codes[b];
                            if (ev_code <= KEY_MAX) {
                                dev->has_button[ev_code] = 1;
                            }
                        }
                    }
                }
            }
            printf("Périphérique: %s (%s) => %d axes, %d boutons\n",
                   dev->path, dev->name, dev->num_axes, dev->num_buttons);
            actual_count++;
        }
        globfree(&glob_result);
        merged_devices = malloc(actual_count * sizeof(InputDevice));
        if (!merged_devices) {
            perror("malloc merged_devices");
            exit(EXIT_FAILURE);
        }
        merged_count = actual_count;
        for (int i = 0; i < actual_count; i++) {
            bool found = false;
            for (int j = 0; j < saved_count; j++) {
                if (saved_devices && (saved_devices[j].id.bustype == detected_devices[i].id.bustype &&
                    saved_devices[j].id.vendor == detected_devices[i].id.vendor &&
                    saved_devices[j].id.product == detected_devices[i].id.product &&
                    saved_devices[j].id.version == detected_devices[i].id.version)) {
                    memcpy(detected_devices[i].axis_mapping, saved_devices[j].axis_mapping, sizeof(saved_devices[j].axis_mapping));
                    memcpy(detected_devices[i].axis_dead_zone, saved_devices[j].axis_dead_zone, sizeof(saved_devices[j].axis_dead_zone));
                    memcpy(detected_devices[i].axis_invert, saved_devices[j].axis_invert, sizeof(saved_devices[j].axis_invert));
                    memcpy(detected_devices[i].axis_virtual_joystick, saved_devices[j].axis_virtual_joystick, sizeof(saved_devices[j].axis_virtual_joystick));
                    memcpy(detected_devices[i].axis_virtual_axis, saved_devices[j].axis_virtual_axis, sizeof(saved_devices[j].axis_virtual_axis));
                    memcpy(detected_devices[i].button_mapping, saved_devices[j].button_mapping, sizeof(saved_devices[j].button_mapping));
                    memcpy(detected_devices[i].button_virtual_joystick, saved_devices[j].button_virtual_joystick, sizeof(saved_devices[j].button_virtual_joystick));
                    detected_devices[i].num_axes = saved_devices[j].num_axes;
                    detected_devices[i].num_buttons = saved_devices[j].num_buttons;
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("Nouveau joystick détecté: %s\n", detected_devices[i].name);
            }
            merged_devices[i] = detected_devices[i];
        }
        for (int j = 0; j < saved_count; j++) {
            bool still_present = false;
            for (int i = 0; i < merged_count; i++) {
                if (saved_devices && (saved_devices[j].id.bustype == merged_devices[i].id.bustype &&
                    saved_devices[j].id.vendor == merged_devices[i].id.vendor &&
                    saved_devices[j].id.product == merged_devices[i].id.product &&
                    saved_devices[j].id.version == merged_devices[i].id.version)) {
                    still_present = true;
                    break;
                }
            }
            if (!still_present) {
                printf("Joystick '%s' du mapping n'est plus détecté.\n", saved_devices[j].name);
            }
        }
        free(saved_devices);
        free(detected_devices);
        if (save_mapping(mapping_file, merged_devices, merged_count, global_axis_index, global_button_index))
            printf("Mapping sauvegardé dans %s\n", mapping_file);
        else
            printf("Erreur lors de la sauvegarde du mapping\n");
        *final_devices = merged_devices;
        *nb_final = merged_count;
        return;
    }
    glob_t glob_result;
    if (glob("/dev/input/event*", 0, NULL, &glob_result) != 0) {
        printf("Aucun périphérique evdev trouvé.\n");
        return;
    }
    int nb_devices = glob_result.gl_pathc;
    InputDevice *devices = malloc(nb_devices * sizeof(InputDevice));
    if (!devices) {
        perror("malloc devices");
        globfree(&glob_result);
        return;
    }
    int count = 0;
    for (int i = 0; i < nb_devices; i++) {
        InputDevice *dev = &devices[count];
        memset(dev, 0, sizeof(InputDevice));
        strncpy(dev->path, glob_result.gl_pathv[i], sizeof(dev->path)-1);
        dev->path[sizeof(dev->path)-1] = '\0';
        dev->fd = open(dev->path, O_RDONLY | O_NONBLOCK);
        if (dev->fd < 0) {
            perror("Erreur open dev");
            continue;
        }
        if (ioctl(dev->fd, EVIOCGNAME(sizeof(dev->name)), dev->name) < 0) {
            perror("Erreur EVIOCGNAME");
            strncpy(dev->name, "Unknown", sizeof(dev->name)-1);
        }
        if (ioctl(dev->fd, EVIOCGID, &dev->id) < 0) {
            perror("Erreur EVIOCGID");
            memset(&dev->id, 0, sizeof(dev->id));
        }
        if (strstr(dev->name, "vc4-hdmi")) {
            close(dev->fd);
            continue;
        }
        unsigned char abs_bitmask[(ABS_CNT/8)+1] = {0};
        if (ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) {
            perror("Erreur EVIOCGBIT(EV_ABS)");
        } else {
            for (int j = 0; j < ABS_CNT; j++) {
                if (abs_bitmask[j/8] & (1 << (j % 8))) {
                    if (ioctl(dev->fd, EVIOCGABS(j), &dev->absinfo[j]) == 0) {
                        dev->has_abs[j] = 1;
                        dev->axis_mapping[j] = global_axis_index++;
                        dev->num_axes++;
                        if (dev->num_axes <= 8)
                            dev->axis_virtual_axis[j] = dev->num_axes - 1;
                        else
                            dev->axis_virtual_axis[j] = (dev->num_axes - 1) % 8;
                        dev->axis_virtual_joystick[j] = 0;
                    }
                }
            }
        }
        for (int j = 0; j <= KEY_MAX; j++) {
            dev->button_mapping[j] = -1;
            dev->button_virtual_joystick[j] = 0;
            dev->has_button[j] = 0;
        }
        int taille_bitmask = (KEY_MAX + 7) / 8;
        unsigned char key_bitmask[taille_bitmask];
        memset(key_bitmask, 0, taille_bitmask);
        if (ioctl(dev->fd, EVIOCGBIT(EV_KEY, taille_bitmask), key_bitmask) < 0) {
            perror("Erreur EVIOCGBIT(EV_KEY)");
        } else {
            for (int j = 0; j <= KEY_MAX; j++) {
                if (key_bitmask[j / 8] & (1 << (j % 8))) {
                    dev->has_button[j] = 1;
                    dev->num_buttons++;
                }
            }
        }
        printf("Périphérique: %s (%s) => %d axes, %d boutons\n",
               dev->path, dev->name, dev->num_axes, dev->num_buttons);
        count++;
    }
    globfree(&glob_result);
    *final_devices = devices;
    *nb_final = count;
    if (count > 0) {
        if (save_mapping(mapping_file, devices, count, global_axis_index, global_button_index))
            printf("Mapping sauvegardé dans %s\n", mapping_file);
        else
            printf("Erreur lors de la sauvegarde du mapping\n");
    }
}
