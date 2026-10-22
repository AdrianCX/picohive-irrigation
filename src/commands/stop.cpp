/* SPDX-License-Identifier: MPL-2.0

Copyright (c) 2026 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "irrigation_manager.h"
#include "pico_helper.h"
#include "stop.h"

bool Stop::on_header(CommandContext& ctx)
{
    uint32_t valve = 0;

    if (!PicoHelper::parse_number(ctx.args, valve))
    {
        return ctx.reply_fmt(96, "expected stop/<valve, got [%.*s]", (int)ctx.args.size(), ctx.args.data());
    }
	
    const char *reason = ctx.prefix.empty() ? "calendar" : "remote command";

	bool success = true;
	if (valve == 0)
	{
		success = IrrigationManager::get_instance()->close_all(reason);
	}
	else
	{
		success = IrrigationManager::get_instance()->close_valve(valve-1, reason);
	}

	if (!success)
	{
		return ctx.reply("closing the valves failed");
	}

    return true;
}

TELEGRAM_COMMAND(Stop);
CALENDAR_COMMAND(Stop);
