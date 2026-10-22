/* SPDX-License-Identifier: MPL-2.0

Copyright (c) 2026 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <stdio.h>

#include "os_platform.h"

#include "irrigation_manager.h"
#include "status.h"

bool Status::on_header(CommandContext& ctx)
{
    IrrigationManager *irrigation = IrrigationManager::get_instance();
    uint64_t now_us = to_us_since_boot(get_absolute_time());

    char text[192] = {};
    int pos = 0;

    for (uint32_t id = 0; (id < IrrigationManager::MAX_VALVES) && (pos < (int)sizeof(text)); ++id)
    {
        if (!irrigation->is_valve_configured(id))
        {
            continue;
        }

        int written = 0;

        if (irrigation->is_valve_open(id))
        {
            uint32_t left = irrigation->valve_seconds_left(id, now_us);

            written = snprintf(&text[pos], sizeof(text) - pos, "valve %u: open, %um%02us left\n",
                               (unsigned int)(id + 1), (unsigned int)(left / 60), (unsigned int)(left % 60));
        }
        else
        {
            written = snprintf(&text[pos], sizeof(text) - pos, "valve %u: closed\n", (unsigned int)(id + 1));
        }

        if (written < 0)
        {
            break;
        }

        pos += written;
    }

    return ctx.reply((pos > 0) ? text : "no valves configured");
}

MQTT_COMMAND(Status);
WS_COMMAND(Status);
TELEGRAM_COMMAND(Status);
