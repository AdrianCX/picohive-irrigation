/* SPDX-License-Identifier: MPL-2.0

Copyright (c) 2026 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "irrigation_manager.h"
#include "pico_helper.h"
#include "water.h"

bool Water::on_header(CommandContext& ctx)
{
    size_t split = ctx.args.find('/');
    uint32_t valve = 0;
    uint32_t amount = 0;
    std::string_view duration = (split == std::string_view::npos) ? std::string_view() : ctx.args.substr(split + 1);

    uint32_t unit = 60;
    if (!duration.empty() && ((duration.back() == 's') || (duration.back() == 'm')))
    {
        unit = (duration.back() == 's') ? 1 : 60;
        duration.remove_suffix(1);
    }

    if ((split == std::string_view::npos) ||
        !PicoHelper::parse_number(ctx.args.substr(0, split), valve) ||
        !PicoHelper::parse_number(duration, amount))
    {
        return ctx.reply_fmt(128, "expected water/<valve>/<time> like water/1/60s or water/1/30m, got [%.*s]",
                             (int)ctx.args.size(), ctx.args.data());
    }

    IrrigationManager *irrigation = IrrigationManager::get_instance();

    if ((valve != 0) && ((valve < 1) || !irrigation->is_valve_configured(valve - 1)))
    {
        return ctx.reply_fmt(32, "there is no valve %u", (unsigned int)valve);
    }

    uint32_t seconds = amount * unit;

    if ((seconds < 1) || (seconds > IrrigationManager::MAX_VALVE_SECONDS))
    {
        return ctx.reply_fmt(64, "time must be 1s to %us, or 1m to %um", (unsigned int)IrrigationManager::MAX_VALVE_SECONDS, (unsigned int)(IrrigationManager::MAX_VALVE_SECONDS / 60));
    }

    const char *reason = ctx.prefix.empty() ? "calendar" : "remote command";

	bool success = true;
	if (valve == 0)
	{
		success = irrigation->open_all(seconds, reason);
	}
	else
	{
		success = irrigation->open_valve(valve - 1, seconds, reason);
	}

	if (!success)
    {
        return ctx.reply("Failed to open.");
    }

    return true;
}

//MQTT_COMMAND(Water);
//WS_COMMAND(Water);
TELEGRAM_COMMAND(Water);
CALENDAR_COMMAND(Water);
