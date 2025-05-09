#include "usb_hid.h"
#include "input_mapping.h" // Pour la définition de InputDevice
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <stdbool.h>
#include <linux/input.h>

// Les endpoints interrupt (déclarés dans main.c)
extern int ep_int_in0;
extern int ep_int_in1;

extern bool keep_running;

void *process_and_send_hid_reports(void *arg) {
    HidReportArgs *args = (HidReportArgs *)arg;
    int fd = args->fd;
    InputDevice *devices = (InputDevice *)args->devices;
    int nb_joysticks = args->nb_joysticks;
    
    int16_t report0_axes[8] = {0};
    uint8_t report0_buttons[128/8] = {0};
    int16_t report1_axes[8] = {0};
    uint8_t report1_buttons[128/8] = {0};
    
    // Structure pour les transferts interrupt
    struct usb_raw_int_io {
        struct usb_raw_ep_io inner;
        char data[256];
    } io0, io1;
    
    memset(&io0, 0, sizeof(io0));
    io0.inner.ep = ep_int_in0;
    io0.inner.flags = 0;
    io0.inner.length = 33;
    
    memset(&io1, 0, sizeof(io1));
    io1.inner.ep = ep_int_in1;
    io1.inner.flags = 0;
    io1.inner.length = 33;
    
    while (keep_running) {
        fd_set read_set;
        FD_ZERO(&read_set);
        int max_fd = 0;
        for (int i = 0; i < nb_joysticks; i++) {
            FD_SET(devices[i].fd, &read_set);
            if (devices[i].fd > max_fd)
                max_fd = devices[i].fd;
        }
        int sel = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if (sel < 0) {
            perror("select error in HID thread");
            break;
        }
        bool updated0 = false, updated1 = false;
        for (int i = 0; i < nb_joysticks; i++) {
            if (FD_ISSET(devices[i].fd, &read_set)) {
                struct input_event ev;
                ssize_t bytes = read(devices[i].fd, &ev, sizeof(ev));
                if (bytes < 0) {
                    perror("read error in HID thread");
                    continue;
                }
                if (bytes == sizeof(ev)) {
                    if (ev.type == EV_ABS && ev.code < ABS_CNT && devices[i].has_abs[ev.code]) {
                        struct input_absinfo info = devices[i].absinfo[ev.code];
                        int minv = info.minimum, maxv = info.maximum;
                        int range = maxv - minv;
                        int val = ev.value;
                        int target_joy = devices[i].axis_virtual_joystick[ev.code];
                        int target_axis = devices[i].axis_virtual_axis[ev.code];
                        int16_t final_val = 0;
                        printf("Device %s, axe code=%d, val=%d, min=%d, max=%d\n",
                               devices[i].name, ev.code, val, minv, maxv);
                        if (target_axis >= 0 && target_axis < 8 && (target_joy == 0 || target_joy == 1)) {
                            if (range != 0) {
                                if (val < minv) val = minv;
                                if (val > maxv) val = maxv;
                                int64_t tmp = (int64_t)(val - minv) * 65535 / range;
                                int64_t signed_val = tmp - 32768;
                                if (signed_val < -32768) signed_val = -32768;
                                if (signed_val > 32767) signed_val = 32767;
                                final_val = (int16_t)signed_val;
                                if (devices[i].axis_invert[ev.code])
                                    final_val = -final_val;
                                if (devices[i].axis_dead_zone[ev.code] > 0 &&
                                    final_val > -devices[i].axis_dead_zone[ev.code] &&
                                    final_val < devices[i].axis_dead_zone[ev.code])
                                    final_val = 0;
                            }
                            if (target_joy == 0) {
                                if (report0_axes[target_axis] != final_val) {
                                    report0_axes[target_axis] = final_val;
                                    updated0 = true;
                                }
                            } else if (target_joy == 1) {
                                if (report1_axes[target_axis] != final_val) {
                                    report1_axes[target_axis] = final_val;
                                    updated1 = true;
                                }
                            }
                        }
                    } else if (ev.type == EV_KEY && ev.code <= KEY_MAX && ev.value != 2) {
                        if (!devices[i].has_button[ev.code]) {
                            devices[i].has_button[ev.code] = 1;
                            printf("New button detected: code %d on device %s\n", ev.code, devices[i].name);
                        }
                        printf("Device %s: button %d %s\n", devices[i].name, ev.code, (ev.value ? "pressed" : "released"));
                        int code_phys = ev.code;
                        int mapped_button = devices[i].button_mapping[code_phys];
                        int target_joy = devices[i].button_virtual_joystick[code_phys];
                        if (mapped_button < 0)
                            continue;
                        if (mapped_button >= 0 && mapped_button < MAX_BUTTONS) {
                            int byte_index = mapped_button / 8;
                            int bit_index = mapped_button % 8;
                            if (target_joy == 0) {
                                uint8_t old_value = report0_buttons[byte_index];
                                if (ev.value)
                                    report0_buttons[byte_index] |= (1 << bit_index);
                                else
                                    report0_buttons[byte_index] &= ~(1 << bit_index);
                                if (report0_buttons[byte_index] != old_value)
                                    updated0 = true;
                            } else if (target_joy == 1) {
                                uint8_t old_value = report1_buttons[byte_index];
                                if (ev.value)
                                    report1_buttons[byte_index] |= (1 << bit_index);
                                else
                                    report1_buttons[byte_index] &= ~(1 << bit_index);
                                if (report1_buttons[byte_index] != old_value)
                                    updated1 = true;
                            }
                        }
                    }
                }
            }
        }
        if (updated0) {
            io0.inner.data[0] = 0x01; // Report ID 1
            memcpy(&io0.inner.data[1], report0_axes, 8 * sizeof(int16_t));
            memcpy(&io0.inner.data[1 + 8 * sizeof(int16_t)], report0_buttons, 128/8);
            int rv = usb_raw_ep_write_may_fail(fd, (struct usb_raw_ep_io *)&io0);
            if (rv < 0 && errno == ESHUTDOWN) {
                printf("ep_int_in0: device reset, ending HID thread\n");
                break;
            } else if (rv < 0) {
                perror("usb_raw_ep_write_may_fail() joystick 0");
                exit(EXIT_FAILURE);
            }
        }
        if (updated1) {
            io1.inner.data[0] = 0x02; // Report ID 2
            memcpy(&io1.inner.data[1], report1_axes, 8 * sizeof(int16_t));
            memcpy(&io1.inner.data[1 + 8 * sizeof(int16_t)], report1_buttons, 128/8);
            int rv = usb_raw_ep_write_may_fail(fd, (struct usb_raw_ep_io *)&io1);
            if (rv < 0 && errno == ESHUTDOWN) {
                printf("ep_int_in1: device reset, ending HID thread\n");
                break;
            } else if (rv < 0) {
                perror("usb_raw_ep_write_may_fail() joystick 1");
                exit(EXIT_FAILURE);
            }
        }
        usleep(1000);
    }
    free(args);
    return NULL;
}
