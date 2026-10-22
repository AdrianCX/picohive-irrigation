/* SPDX-License-Identifier: MPL-2.0

Copyright (c) 2026 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "command_registration.h"

struct Stop
{
    static constexpr std::string_view name = "stop";
    static constexpr std::string_view help = "stop - close every valve";
    static constexpr std::string_view ai_help = "stop/<valve> - stop watering: 1, 2, or 0 for both";

    bool on_header(CommandContext& ctx);
};
