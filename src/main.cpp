/* SPDX-License-Identifier: MPL-2.0

Copyright (c) 2026 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <string.h>

#include "common_functions.h"
#include "listener.h"
#include "tls_listener.h"
#include "pico_logger.h"
#include "session.h"

#include "app_config.h"
#include "calendar_commands.h"
#include "command_request_handler.h"
#include "general_config.h"
#include "irrigation_manager.h"
#include "telegram_commands.h"
#include "telegram_manager.h"
#include "wifi.h"

#include "rc/index.html.gz.hex.h"

static const TelegramButton IRRIGATION_MENU[] = {
    { "LFT", "/left" },
    { "OFF", "/stop/1" },
    { "30s", "/water/1/30s" },
    { "10m", "/water/1/10m" },
    { "30m", "/water/1/30m" },

    { "RGT", "/right" },
    { "OFF", "/stop/2" },
    { "30s", "/water/2/30s" },
    { "10m", "/water/2/10m" },
    { "30m", "/water/2/30m" },

    { "ALL", "/all" },
    { "OFF", "/stop/0" },
    { "30s", "/water/0/30s" },
    { "10m", "/water/0/10m" },
    { "30m", "/water/0/30m" },
};

void on_boot()
{
    IrrigationManager::get_instance()->set_valve(0, 16, 17, 15);
    IrrigationManager::get_instance()->set_valve(1, 18, 19, 15);
    IrrigationManager::get_instance()->set_buttons(13, 12);
    IrrigationManager::get_instance()->initialize();
}

// Access point if not configured - otherwise user needs to use universal flash nuke to get rid of the config.
bool is_access_point()
{
    return (get_config().wifi_ssid[0] == 0);
}

void on_wifi_failed()
{
}

bool on_wifi_ready()
{
	// NULL means - use the built-in roots (os/config/tls_domains.txt or src/config/tls_domains.txt)
    Session::create_client_tls_config(NULL, 0);

    // Configure the embedded index.html page to be served by default
    CommandRequestHandler::set_config(__rc_index_html_gz, __rc_index_html_gz_len);

    Listener *listener = new Listener();
    listener->listen(80, CommandRequestHandler::create);

    // When in access point mode - device is accessible without username/pass over HTTP on the created wifi, intended
    if (!is_access_point())
    {
        CommandRequestHandler::require_auth(true);

        // RP2040 is pretty slow, HTTPS still works but it's annoying, enable if this is acceptable
#if !defined(PICO_RP2040) || (PICO_RP2040 == 0)
        TLSListener *tls_listener = new TLSListener();
        tls_listener->listen(443, CommandRequestHandler::create);
#endif
    }

	// Irrigation listens for telegram commands so get that loop going with its commands
    TelegramCommands::enable("Irrigation", IRRIGATION_MENU, sizeof(IRRIGATION_MENU) / sizeof(IRRIGATION_MENU[0]), 5);

    return true;
}

void on_update(uint64_t now_us)
{
    IrrigationManager::get_instance()->update(now_us);
    TelegramManager::get().update(now_us);
    CalendarCommands::get().update(now_us);
}

void on_config_update(const JsonView& config)
{
    TelegramManager::get().load_config(config);
    CalendarCommands::get().load_config(config);
}

// App hooks from the OS
void get_app_info(AppConfig *config)
{
    config->on_boot = on_boot;
    config->access_point = is_access_point;
    config->on_wifi_failed = on_wifi_failed;
    config->on_wifi_ready = on_wifi_ready;
    config->on_update = on_update;
    config->on_config_update = on_config_update;

	// How this build advertises itself in logs if configured.
    config->name = "build/" PICO_BOARD "/irrigation";

    // Defaults to git describe of this repository.
    // config->version = "1.0";

    // watchdog timer, milliseconds, 0 is off. Board can be stuck a couple of seconds for TLS negotiations so it needs to be large enough to accomodate.
    config->watchdog_timeout = 60000;
}
