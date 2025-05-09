#include "usb_raw.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int usb_raw_open(void) {
    int fd = open("/dev/raw-gadget", O_RDWR);
    if (fd < 0) {
        perror("open(/dev/raw-gadget)");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void usb_raw_init(int fd, int speed, const char *driver, const char *device) {
    struct usb_raw_init arg;
    strncpy((char *)arg.driver_name, driver, sizeof(arg.driver_name)-1);
    strncpy((char *)arg.device_name, device, sizeof(arg.device_name)-1);
    arg.speed = speed;
    int rv = ioctl(fd, USB_RAW_IOCTL_INIT, &arg);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_INIT)");
        exit(EXIT_FAILURE);
    }
}

void usb_raw_run(int fd) {
    int rv = ioctl(fd, USB_RAW_IOCTL_RUN, 0);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_RUN)");
        exit(EXIT_FAILURE);
    }
}

void usb_raw_event_fetch(int fd, struct usb_raw_event *event) {
    int rv = ioctl(fd, USB_RAW_IOCTL_EVENT_FETCH, event);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_EVENT_FETCH)");
        exit(EXIT_FAILURE);
    }
}

int usb_raw_ep0_read(int fd, struct usb_raw_ep_io *io) {
    int rv = ioctl(fd, USB_RAW_IOCTL_EP0_READ, io);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_EP0_READ)");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int usb_raw_ep0_write(int fd, struct usb_raw_ep_io *io) {
    int rv = ioctl(fd, USB_RAW_IOCTL_EP0_WRITE, io);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_EP0_WRITE)");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int usb_raw_ep_enable(int fd, struct usb_endpoint_descriptor *desc) {
    int rv = ioctl(fd, USB_RAW_IOCTL_EP_ENABLE, desc);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_EP_ENABLE)");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int usb_raw_ep_disable(int fd, int ep) {
    int rv = ioctl(fd, USB_RAW_IOCTL_EP_DISABLE, ep);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_EP_DISABLE)");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int usb_raw_ep_write_may_fail(int fd, struct usb_raw_ep_io *io) {
    return ioctl(fd, USB_RAW_IOCTL_EP_WRITE, io);
}

void usb_raw_configure(int fd) {
    int rv = ioctl(fd, USB_RAW_IOCTL_CONFIGURE, 0);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_CONFIGURE)");
        exit(EXIT_FAILURE);
    }
}

void usb_raw_vbus_draw(int fd, uint32_t power) {
    int rv = ioctl(fd, USB_RAW_IOCTL_VBUS_DRAW, power);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_VBUS_DRAW)");
        exit(EXIT_FAILURE);
    }
}

int usb_raw_eps_info(int fd, struct usb_raw_eps_info *info) {
    int rv = ioctl(fd, USB_RAW_IOCTL_EPS_INFO, info);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_EPS_INFO)");
        exit(EXIT_FAILURE);
    }
    return rv;
}

void usb_raw_ep0_stall(int fd) {
    int rv = ioctl(fd, USB_RAW_IOCTL_EP0_STALL, 0);
    if (rv < 0) {
        perror("ioctl(USB_RAW_IOCTL_EP0_STALL)");
        exit(EXIT_FAILURE);
    }
}
