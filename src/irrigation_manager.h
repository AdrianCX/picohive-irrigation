/* SPDX-License-Identifier: MPL-2.0

Copyright (c) 2026 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "os_platform.h"
#include "mqtt_handler.h"
#include "upgrade_helper.h"

struct Valve
{
    bool active = false;
    bool configured = false;

    uint8_t pin_fwd = 0;
    uint8_t pin_back = 0;
    uint8_t pin_sleep = 0;
    
    uint64_t shutoff_time_us = 0;
    uint64_t open_time_us = 0;
};

class Button
{
public:
    void init(uint8_t pin);
    bool is_pressed(uint64_t now_us);
    bool activated(uint64_t now_us);

private:
    uint8_t m_pin = 255;

    bool m_pressed = false;
    bool m_initialized = false;
    bool m_latched = true;

    uint64_t m_last_update_us = 0;
    uint64_t m_active_time_us = 0;
};

class IrrigationManager
{
public:
    static IrrigationManager *get_instance();

    bool set_valve(uint32_t id, uint8_t pin_fwd, uint8_t pin_back, uint8_t pin_sleep);

    bool open_valve(uint32_t id, uint32_t seconds, const char *reason);
    bool close_valve(uint32_t id, const char *reason);

    bool open_all(uint32_t seconds, const char *reason);
    bool close_all(const char *reason);

    void update(uint64_t now_us);

    void initialize();

    bool set_buttons(uint8_t pin_on, uint8_t pin_off);

    bool is_valve_configured(uint32_t id) const;
    bool is_valve_open(uint32_t id) const;
    uint32_t valve_seconds_left(uint32_t id, uint64_t now_us) const;

    static const int MAX_VALVES = 4;

    // The longest a valve may be asked to stay open, update() closes it at regardless of what it was asked for.
    static const uint32_t MAX_VALVE_SECONDS = 3600;

private:
    void set_active(bool active);
    bool is_active() { return m_active; }
    
    IrrigationManager() {};
    
    bool m_active = false;
    Valve m_valves[MAX_VALVES];

    Button m_green_button;
    Button m_red_button;
    
    static IrrigationManager *s_instance;
};
