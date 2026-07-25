/*
 * trackpad-haptic — Force Touch trackpad pulses from the CLI.
 *
 * Apple never shipped a public haptic API for the trackpad. The same private
 * MultitouchSupport calls that drive click feedback work from userland if you
 * dlopen the framework and poke the right symbols. Those symbols are
 * undocumented and can move or vanish on a macOS update.
 *
 * I use this from agent stop-hooks (Cursor / Claude Code / Codex): when the
 * last busy agent goes idle, three taps hit the pad so I notice without
 * staring at tabs.
 *
 *   clang -O2 -o trackpad-haptic trackpad-haptic.c \
 *       -framework CoreFoundation -framework IOKit
 *   trackpad-haptic tap [waveform] [count] [interval_ms]
 */

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOReturn.h>

#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Not on the public SDK link line — load by path at runtime. */
#define MT_FW                                                              \
    "/System/Library/PrivateFrameworks/MultitouchSupport.framework/" \
    "MultitouchSupport"

/*
 * MTDevice is an opaque C++-ish object. Community reverse engineering (and
 * testing on Apple Silicon) puts the uint64_t actuator device id at +64.
 * If create/open start failing after an OS update, this offset is the first
 * thing to re-check.
 */
#define MTDEVICE_ID_OFFSET 64

/*
 * Private MultitouchSupport entry points (signatures from reverse engineering).
 *
 * Lifecycle for one pulse:
 *   create(device_id) → open → actuate(waveform, …) → close → CFRelease
 *
 * MTActuatorActuate's last three args are intensity-ish parameters. Some
 * writeups pass floats; on this machine float args made pulses feel weak,
 * while three zero uint32_t args match a solid system click. Leave them 0.
 */
typedef CFTypeRef (*MTActuatorCreateFromDeviceID_t)(uint64_t);
typedef IOReturn (*MTActuatorOpen_t)(CFTypeRef, uint32_t);
typedef IOReturn (*MTActuatorClose_t)(CFTypeRef);
typedef IOReturn (*MTActuatorActuate_t)(CFTypeRef, int32_t, uint32_t, uint32_t,
                                        uint32_t);
typedef CFMutableArrayRef (*MTDeviceCreateList_t)(void);

static void sleep_milliseconds(unsigned long milliseconds) {
    usleep((unsigned int)(milliseconds * 1000UL));
}

static int load_symbols(MTActuatorCreateFromDeviceID_t *create,
                        MTActuatorOpen_t *open, MTActuatorClose_t *close,
                        MTActuatorActuate_t *actuate,
                        MTDeviceCreateList_t *list) {
    void *lib = dlopen(MT_FW, RTLD_LAZY);
    if (lib == NULL) {
        fprintf(stderr, "dlopen MultitouchSupport failed: %s\n", dlerror());
        return 1;
    }

    /* Intentionally leak `lib` for process lifetime — one-shot CLI. */
    *create = dlsym(lib, "MTActuatorCreateFromDeviceID");
    *open = dlsym(lib, "MTActuatorOpen");
    *close = dlsym(lib, "MTActuatorClose");
    *actuate = dlsym(lib, "MTActuatorActuate");
    *list = dlsym(lib, "MTDeviceCreateList");
    if (*create == NULL || *open == NULL || *close == NULL || *actuate == NULL ||
        *list == NULL) {
        fprintf(stderr, "Missing MultitouchSupport actuator symbols\n");
        return 1;
    }
    return 0;
}

/*
 * Return the first multitouch device whose haptic actuator opens.
 *
 * MTDeviceCreateList includes things that are not Force Touch pads (e.g. older
 * external mice / touch surfaces). create() may succeed for junk ids; open()
 * is the real filter. First success wins — usually the built-in trackpad.
 */
static uint64_t find_actuator_device_id(MTActuatorCreateFromDeviceID_t create,
                                        MTActuatorOpen_t open,
                                        MTActuatorClose_t close,
                                        MTDeviceCreateList_t list) {
    CFMutableArrayRef devices = list();
    if (devices == NULL || CFArrayGetCount(devices) == 0) {
        if (devices != NULL) {
            CFRelease(devices);
        }
        return 0;
    }

    uint64_t found = 0;
    for (CFIndex i = 0; i < CFArrayGetCount(devices); i++) {
        void *device = (void *)CFArrayGetValueAtIndex(devices, i);
        uint64_t candidate = 0;
        memcpy(&candidate, (uint8_t *)device + MTDEVICE_ID_OFFSET,
               sizeof(candidate));

        CFTypeRef actuator = create(candidate);
        if (actuator == NULL) {
            continue;
        }
        if (open(actuator, 0) == kIOReturnSuccess) {
            found = candidate;
            close(actuator);
            CFRelease(actuator);
            break;
        }
        CFRelease(actuator);
    }

    CFRelease(devices);
    return found;
}

static int pulse_once(MTActuatorCreateFromDeviceID_t create,
                      MTActuatorOpen_t open, MTActuatorClose_t close,
                      MTActuatorActuate_t actuate, uint64_t device_id,
                      int32_t waveform) {
    /*
     * Reusing one actuator across pulses often no-ops or weakens later taps.
     * Treat the handle as single-shot: fresh create/open per pulse.
     */
    CFTypeRef actuator = create(device_id);
    if (actuator == NULL) {
        fprintf(stderr, "MTActuatorCreateFromDeviceID failed\n");
        return 1;
    }

    IOReturn open_rc = open(actuator, 0);
    IOReturn actuate_rc = kIOReturnError;
    IOReturn close_rc = kIOReturnSuccess;
    if (open_rc == kIOReturnSuccess) {
        actuate_rc = actuate(actuator, waveform, 0, 0, 0);
        close_rc = close(actuator);
    }
    CFRelease(actuator);

    if (open_rc != kIOReturnSuccess || actuate_rc != kIOReturnSuccess ||
        close_rc != kIOReturnSuccess) {
        fprintf(stderr,
                "Actuate failed open=0x%x actuate=0x%x close=0x%x\n", open_rc,
                actuate_rc, close_rc);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "tap") != 0) {
        fprintf(stderr,
                "Usage: trackpad-haptic tap [waveform] [count] [interval_ms]\n"
                "\n"
                "Waveforms: 1 weak, 2 strong click, 3 buzz, 4 light, 5 medium,\n"
                "           6 strong tap, 15 soft thud, 16 strong thud\n"
                "Defaults: waveform=6 count=3 interval_ms=400\n");
        return 2;
    }

    /*
     * Waveform ids are Apple-internal enums, not a public table. Values below
     * were felt out on an M-series MacBook; YMMV. Defaults = "agent done"
     * triple-tap that cuts through without sounding like a system alert.
     */
    int32_t waveform = 6;          /* strong tap */
    unsigned long count = 3;
    unsigned long interval = 400;  /* ms between pulses */

    if (argc >= 3) {
        char *end = NULL;
        unsigned long value = strtoul(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || value < 1 || value > 32) {
            fprintf(stderr, "Waveform must be between 1 and 32\n");
            return 2;
        }
        waveform = (int32_t)value;
    }

    if (argc >= 4) {
        char *end = NULL;
        count = strtoul(argv[3], &end, 10);
        if (end == argv[3] || *end != '\0' || count < 1 || count > 20) {
            fprintf(stderr, "Count must be between 1 and 20\n");
            return 2;
        }
    }

    if (argc >= 5) {
        char *end = NULL;
        interval = strtoul(argv[4], &end, 10);
        if (end == argv[4] || *end != '\0' || interval < 50 ||
            interval > 2000) {
            fprintf(stderr, "Interval must be between 50 and 2000 ms\n");
            return 2;
        }
    }

    MTActuatorCreateFromDeviceID_t create = NULL;
    MTActuatorOpen_t open = NULL;
    MTActuatorClose_t close = NULL;
    MTActuatorActuate_t actuate = NULL;
    MTDeviceCreateList_t list = NULL;
    if (load_symbols(&create, &open, &close, &actuate, &list) != 0) {
        return 1;
    }

    uint64_t device_id = find_actuator_device_id(create, open, close, list);
    if (device_id == 0) {
        fprintf(stderr, "No Force Touch trackpad actuator found\n");
        return 1;
    }

    for (unsigned long pulse = 0; pulse < count; pulse++) {
        if (pulse_once(create, open, close, actuate, device_id, waveform) !=
            0) {
            return 1;
        }
        if (pulse + 1 < count) {
            sleep_milliseconds(interval);
        }
    }

    return 0;
}
