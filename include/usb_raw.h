#ifndef USB_RAW_H
#define USB_RAW_H

#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/usb/ch9.h>
#include <linux/hidraw.h>

// Définition des IOCTL pour le gadget USB raw
#define USB_RAW_IOCTL_INIT              _IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN               _IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH       _IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE         _IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ          _IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_ENABLE         _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_RAW_IOCTL_EP_DISABLE        _IOW('U', 6, __u32)
#define USB_RAW_IOCTL_EP_WRITE          _IOW('U', 7, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_READ           _IOWR('U', 8, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_CONFIGURE         _IO('U', 9)
#define USB_RAW_IOCTL_VBUS_DRAW         _IOW('U', 10, __u32)
#define USB_RAW_IOCTL_EPS_INFO          _IOR('U', 11, struct usb_raw_eps_info)
#define USB_RAW_IOCTL_EP0_STALL         _IO('U', 12)

// Structures pour l'initialisation et gestion du gadget USB raw
struct usb_raw_init {
    __u8 driver_name[128];
    __u8 device_name[128];
    __u8 speed;
};

enum usb_raw_event_type {
    USB_RAW_EVENT_INVALID = 0,
    USB_RAW_EVENT_CONNECT = 1,
    USB_RAW_EVENT_CONTROL = 2,
    USB_RAW_EVENT_SUSPEND = 3,
    USB_RAW_EVENT_RESUME = 4,
    USB_RAW_EVENT_RESET = 5,
    USB_RAW_EVENT_DISCONNECT = 6,
};

struct usb_raw_event {
    __u32 type;
    __u32 length;
    __u8 data[];
};

struct usb_raw_ep_io {
    __u16 ep;
    __u16 flags;
    __u32 length;
    __u8 data[];
};

#define USB_RAW_EPS_NUM_MAX 30
#define USB_RAW_EP_NAME_MAX 16
#define USB_RAW_EP_ADDR_ANY 0xff

struct usb_raw_ep_caps {
    __u32 type_control : 1;
    __u32 type_iso     : 1;
    __u32 type_bulk    : 1;
    __u32 type_int     : 1;
    __u32 dir_in       : 1;
    __u32 dir_out      : 1;
};

struct usb_raw_ep_limits {
    __u16 maxpacket_limit;
    __u16 max_streams;
    __u32 reserved;
};

struct usb_raw_ep_info {
    __u8 name[USB_RAW_EP_NAME_MAX];
    __u32 addr;
    struct usb_raw_ep_caps caps;
    struct usb_raw_ep_limits limits;
};

struct usb_raw_eps_info {
    struct usb_raw_ep_info eps[USB_RAW_EPS_NUM_MAX];
};

// Prototypes des fonctions d'accès au gadget USB raw
int usb_raw_open(void);
void usb_raw_init(int fd, int speed, const char *driver, const char *device);
void usb_raw_run(int fd);
void usb_raw_event_fetch(int fd, struct usb_raw_event *event);
int usb_raw_ep0_read(int fd, struct usb_raw_ep_io *io);
int usb_raw_ep0_write(int fd, struct usb_raw_ep_io *io);
int usb_raw_ep_enable(int fd, struct usb_endpoint_descriptor *desc);
int usb_raw_ep_disable(int fd, int ep);
int usb_raw_ep_write_may_fail(int fd, struct usb_raw_ep_io *io);
void usb_raw_configure(int fd);
void usb_raw_vbus_draw(int fd, uint32_t power);
int usb_raw_eps_info(int fd, struct usb_raw_eps_info *info);
void usb_raw_ep0_stall(int fd);

#endif // USB_RAW_H
