/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * DFU bootloader main.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "omv_bootconfig.h"

bool tud_dfu_detached = true;

int main(void) {
    // Initialize low-level sub-systems.
    port_init();

    // Initialize TinyUSB.
    tusb_init();

    uint32_t start_ms = port_ticks_ms();

    while (true) {
        // Poll TinyUSB.
        tud_task();
        // Wait for the device to be connected and configured.
        if (tud_mounted() || (port_ticks_ms() - start_ms) > 500) {
            break;
        }
        port_led_blink(150);
    }

    // Restart timeout.
    start_ms = port_ticks_ms();

    while (tud_mounted()) {
        // Poll TinyUSB.
        tud_task();
        // Wait for first DFU command.
        if (tud_dfu_detached && ((port_ticks_ms() - start_ms) > OMV_BOOT_DFU_TIMEOUT)) {
            // Timeout, jump to main app.
            break;
        }
        if (!tud_dfu_detached) {
            port_led_blink(150);
        } else {
            port_led_blink(250);
        }
    }

    // Disconnect USB device.
    tud_disconnect();

    // Deinitialize low-level sub-systems.
    port_deinit();

    // JUMP!
    ((void (*) (void)) (*((uint32_t *) (OMV_BOOT_JUMP_ADDR + 4)))) ();
}