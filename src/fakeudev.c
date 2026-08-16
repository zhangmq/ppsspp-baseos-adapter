/* fakeudev.c - fake libudev.so.1 for SDL 2.0.12 on BaseOS.
 *
 * BaseOS has no udev daemon, so device properties (ID_INPUT_JOYSTICK etc.)
 * are absent and SDL 2.0.12's udev joystick detection finds nothing.
 * SDL dlopens "libudev.so.1" and dlsym's its functions; if this library is
 * loaded via LD_PRELOAD under that name, dlopen returns this handle and
 * SDL calls OUR wrappers. Everything passes through to the real libudev
 * (loaded by absolute path) except:
 *   - udev_enumerate_add_match_property: no-op (accept all devices)
 *   - udev_device_get_property_value: ID_INPUT_JOYSTICK/GAMEPAD -> "1"
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>

#define REAL_LIB "/mnt/sdcard/Emus/h700/PSP.pak/lib/libudev.so.1.7.2"

static void *real_handle;
#define DBG(fmt, ...) fprintf(stderr, "fakeudev: " fmt "\n", ##__VA_ARGS__)

static void *sym(const char *name) {
    void *p = dlsym(real_handle, name);
    if (!p) fprintf(stderr, "fakeudev: missing %s\n", name);
    return p;
}

/* --- types we only touch through opaque pointers --- */
typedef void *udev_t;
typedef void *udev_enumerate_t;
typedef void *udev_list_entry_t;
typedef void *udev_device_t;
typedef void *udev_monitor_t;

void *udev_new(void) {
    static void *(*f)(void) = 0;
    if (!f) f = sym("udev_new");
    return f ? f() : NULL;
}
void udev_unref(void *u) {
    static void (*f)(void *) = 0;
    if (!f) f = sym("udev_unref");
    if (f) f(u);
}
udev_enumerate_t udev_enumerate_new(udev_t u) {
    static void *(*f)(void *) = 0;
    if (!f) f = sym("udev_enumerate_new");
    return f ? f(u) : NULL;
}
void udev_enumerate_unref(udev_enumerate_t e) {
    static void (*f)(void *) = 0;
    if (!f) f = sym("udev_enumerate_unref");
    if (f) f(e);
}
int udev_enumerate_add_match_subsystem(udev_enumerate_t e, const char *sub) {
    static int (*f)(void *, const char *) = 0;
    if (!f) f = sym("udev_enumerate_add_match_subsystem");
    return f ? f(e, sub) : -1;
}
/* THE FIX: do not filter by property */
int udev_enumerate_add_match_property(udev_enumerate_t e, const char *k,
                                      const char *v) {
    (void)e; (void)k; (void)v;
    return 0;
}
int udev_enumerate_scan_devices(udev_enumerate_t e) {
    static int (*f)(void *) = 0;
    if (!f) f = sym("udev_enumerate_scan_devices");
    return f ? f(e) : -1;
}
udev_list_entry_t udev_enumerate_get_list_entry(udev_enumerate_t e) {
    static void *(*f)(void *) = 0;
    if (!f) f = sym("udev_enumerate_get_list_entry");
    return f ? f(e) : NULL;
}
udev_list_entry_t udev_list_entry_get_next(udev_list_entry_t le) {
    static void *(*f)(void *) = 0;
    if (!f) f = sym("udev_list_entry_get_next");
    return f ? f(le) : NULL;
}
const char *udev_list_entry_get_name(udev_list_entry_t le) {
    static const char *(*f)(void *) = 0;
    if (!f) f = sym("udev_list_entry_get_name");
    return f ? f(le) : NULL;
}
udev_device_t udev_device_new_from_syspath(udev_t u, const char *path) {
    static void *(*f)(void *, const char *) = 0;
    if (!f) f = sym("udev_device_new_from_syspath");
    return f ? f(u, path) : NULL;
}
void udev_device_unref(udev_device_t d) {
    static void (*f)(void *) = 0;
    if (!f) f = sym("udev_device_unref");
    if (f) f(d);
}
const char *udev_device_get_devnode(udev_device_t d) {
    static const char *(*f)(void *) = 0;
    if (!f) f = sym("udev_device_get_devnode");
    return f ? f(d) : NULL;
}
const char *udev_device_get_syspath(udev_device_t d) {
    static const char *(*f)(void *) = 0;
    if (!f) f = sym("udev_device_get_syspath");
    return f ? f(d) : NULL;
}
const char *udev_device_get_subsystem(udev_device_t d) {
    static const char *(*f)(void *) = 0;
    if (!f) f = sym("udev_device_get_subsystem");
    return f ? f(d) : NULL;
}
const char *udev_device_get_sysattr_value(udev_device_t d, const char *attr) {
    static const char *(*f)(void *, const char *) = 0;
    if (!f) f = sym("udev_device_get_sysattr_value");
    return f ? f(d, attr) : NULL;
}
const char *udev_device_get_action(udev_device_t d) {
    static const char *(*f)(void *) = 0;
    if (!f) f = sym("udev_device_get_action");
    return f ? f(d) : NULL;
}
const char *udev_device_get_parent_with_subsystem_devtype(udev_device_t d,
                                                   const char *sub,
                                                   const char *devtype) {
    static const char *(*f)(void *, const char *, const char *) = 0;
    if (!f) f = sym("udev_device_get_parent_with_subsystem_devtype");
    return f ? f(d, sub, devtype) : NULL;
}
/* THE FIX: claim joystick/gamepad properties ONLY for ANBERNIC-keys
 * (event1). dierct-keys-polled (event2) and axp2202-pek (event0) share the
 * same bus/vendor/product GUID, so without this filter they would also
 * become gamepads and be picked up as PPSSPP pad 0. */
const char *udev_device_get_property_value(udev_device_t d, const char *k) {
    static const char *(*f)(void *, const char *) = 0;
    static const char *(*devnode_f)(void *) = 0;
    if (strcmp(k, "ID_INPUT_JOYSTICK") == 0 ||
        strcmp(k, "ID_INPUT_GAMEPAD") == 0) {
        if (!devnode_f) devnode_f = sym("udev_device_get_devnode");
        const char *node = devnode_f ? devnode_f(d) : NULL;
        if (node && strcmp(node, "/dev/input/event1") == 0)
            return "1";
        return NULL;
    }
    if (!f) f = sym("udev_device_get_property_value");
    return f ? f(d, k) : NULL;
}
udev_monitor_t udev_monitor_new_from_netlink(udev_t u, const char *name) {
    static void *(*f)(void *, const char *) = 0;
    if (!f) f = sym("udev_monitor_new_from_netlink");
    return f ? f(u, name) : NULL;
}
int udev_monitor_enable_receiving(udev_monitor_t m) {
    static int (*f)(void *) = 0;
    if (!f) f = sym("udev_monitor_enable_receiving");
    return f ? f(m) : -1;
}
int udev_monitor_filter_add_match_subsystem_devtype(udev_monitor_t m,
                                                    const char *s,
                                                    const char *t) {
    static int (*f)(void *, const char *, const char *) = 0;
    if (!f) f = sym("udev_monitor_filter_add_match_subsystem_devtype");
    return f ? f(m, s, t) : -1;
}
int udev_monitor_get_fd(udev_monitor_t m) {
    static int (*f)(void *) = 0;
    if (!f) f = sym("udev_monitor_get_fd");
    return f ? f(m) : -1;
}
udev_device_t udev_monitor_receive_device(udev_monitor_t m) {
    static void *(*f)(void *) = 0;
    if (!f) f = sym("udev_monitor_receive_device");
    return f ? f(m) : NULL;
}

__attribute__((constructor))
static void init(void) {
    real_handle = dlopen(REAL_LIB, RTLD_NOW | RTLD_GLOBAL);
    DBG("fake loaded, real_handle=%p", real_handle);
    if (!real_handle)
        fprintf(stderr, "fakeudev: cannot load %s: %s\n", REAL_LIB, dlerror());
}


/* SDL 2.0.12 dlsym's udev symbols with a leading underscore */


/* SDL 2.0.12 dlsym's udev symbols with a leading underscore */
__typeof__(udev_new) _udev_new __attribute__((alias("udev_new")));
__typeof__(udev_unref) _udev_unref __attribute__((alias("udev_unref")));
__typeof__(udev_enumerate_new) _udev_enumerate_new __attribute__((alias("udev_enumerate_new")));
__typeof__(udev_enumerate_unref) _udev_enumerate_unref __attribute__((alias("udev_enumerate_unref")));
__typeof__(udev_enumerate_add_match_subsystem) _udev_enumerate_add_match_subsystem __attribute__((alias("udev_enumerate_add_match_subsystem")));
__typeof__(udev_enumerate_add_match_property) _udev_enumerate_add_match_property __attribute__((alias("udev_enumerate_add_match_property")));
__typeof__(udev_enumerate_scan_devices) _udev_enumerate_scan_devices __attribute__((alias("udev_enumerate_scan_devices")));
__typeof__(udev_enumerate_get_list_entry) _udev_enumerate_get_list_entry __attribute__((alias("udev_enumerate_get_list_entry")));
__typeof__(udev_list_entry_get_next) _udev_list_entry_get_next __attribute__((alias("udev_list_entry_get_next")));
__typeof__(udev_list_entry_get_name) _udev_list_entry_get_name __attribute__((alias("udev_list_entry_get_name")));
__typeof__(udev_device_new_from_syspath) _udev_device_new_from_syspath __attribute__((alias("udev_device_new_from_syspath")));
__typeof__(udev_device_unref) _udev_device_unref __attribute__((alias("udev_device_unref")));
__typeof__(udev_device_get_devnode) _udev_device_get_devnode __attribute__((alias("udev_device_get_devnode")));
__typeof__(udev_device_get_syspath) _udev_device_get_syspath __attribute__((alias("udev_device_get_syspath")));
__typeof__(udev_device_get_subsystem) _udev_device_get_subsystem __attribute__((alias("udev_device_get_subsystem")));
__typeof__(udev_device_get_sysattr_value) _udev_device_get_sysattr_value __attribute__((alias("udev_device_get_sysattr_value")));
__typeof__(udev_device_get_property_value) _udev_device_get_property_value __attribute__((alias("udev_device_get_property_value")));
__typeof__(udev_device_get_action) _udev_device_get_action __attribute__((alias("udev_device_get_action")));
__typeof__(udev_monitor_new_from_netlink) _udev_monitor_new_from_netlink __attribute__((alias("udev_monitor_new_from_netlink")));
__typeof__(udev_monitor_enable_receiving) _udev_monitor_enable_receiving __attribute__((alias("udev_monitor_enable_receiving")));
__typeof__(udev_monitor_filter_add_match_subsystem_devtype) _udev_monitor_filter_add_match_subsystem_devtype __attribute__((alias("udev_monitor_filter_add_match_subsystem_devtype")));
__typeof__(udev_monitor_get_fd) _udev_monitor_get_fd __attribute__((alias("udev_monitor_get_fd")));
__typeof__(udev_monitor_receive_device) _udev_monitor_receive_device __attribute__((alias("udev_monitor_receive_device")));
__typeof__(udev_device_get_parent_with_subsystem_devtype) _udev_device_get_parent_with_subsystem_devtype __attribute__((alias("udev_device_get_parent_with_subsystem_devtype")));

/* remaining symbols SDL may request */
void udev_monitor_unref(udev_monitor_t m) {
    static void (*f)(void *) = 0;
    if (!f) f = sym("udev_monitor_unref");
    if (f) f(m);
}
const char *udev_device_get_sysnum(udev_device_t d) {
    static const char *(*f)(void *) = 0;
    if (!f) f = sym("udev_device_get_sysnum");
    return f ? f(d) : NULL;
}
const char *udev_device_get_devtype(udev_device_t d) {
    static const char *(*f)(void *) = 0;
    if (!f) f = sym("udev_device_get_devtype");
    return f ? f(d) : NULL;
}
udev_device_t udev_device_get_parent(udev_device_t d) {
    static void *(*f)(void *) = 0;
    if (!f) f = sym("udev_device_get_parent");
    return f ? f(d) : NULL;
}
udev_list_entry_t udev_list_entry_get_by_name(udev_list_entry_t le,
                                              const char *name) {
    static void *(*f)(void *, const char *) = 0;
    if (!f) f = sym("udev_list_entry_get_by_name");
    return f ? f(le, name) : NULL;
}
int udev_enumerate_add_match_sysattr(udev_enumerate_t e, const char *k,
                                     const char *v) {
    static int (*f)(void *, const char *, const char *) = 0;
    if (!f) f = sym("udev_enumerate_add_match_sysattr");
    return f ? f(e, k, v) : -1;
}
__typeof__(udev_monitor_unref) _udev_monitor_unref __attribute__((alias("udev_monitor_unref")));
__typeof__(udev_device_get_sysnum) _udev_device_get_sysnum __attribute__((alias("udev_device_get_sysnum")));
__typeof__(udev_device_get_devtype) _udev_device_get_devtype __attribute__((alias("udev_device_get_devtype")));
__typeof__(udev_device_get_parent) _udev_device_get_parent __attribute__((alias("udev_device_get_parent")));
__typeof__(udev_list_entry_get_by_name) _udev_list_entry_get_by_name __attribute__((alias("udev_list_entry_get_by_name")));
__typeof__(udev_enumerate_add_match_sysattr) _udev_enumerate_add_match_sysattr __attribute__((alias("udev_enumerate_add_match_sysattr")));

/* last two symbols from SDL's SDL_UDEV_SYM list */
typedef unsigned long long dev_t_u64;
udev_device_t udev_device_new_from_devnum(udev_t u, char type, dev_t_u64 num) {
    static void *(*f)(void *, char, unsigned long long) = 0;
    if (!f) f = sym("udev_device_new_from_devnum");
    return f ? f(u, type, (unsigned long long)num) : NULL;
}
const char *udev_device_get_devnum(udev_device_t d) {
    static const char *(*f)(void *) = 0;
    if (!f) f = sym("udev_device_get_devnum");
    return f ? f(d) : NULL;
}
__typeof__(udev_device_new_from_devnum) _udev_device_new_from_devnum __attribute__((alias("udev_device_new_from_devnum")));
__typeof__(udev_device_get_devnum) _udev_device_get_devnum __attribute__((alias("udev_device_get_devnum")));
