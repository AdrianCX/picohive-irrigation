/* SPDX-License-Identifier: MPL-2.0

Copyright (c) 2026 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <string.h>
#include <time.h>

#include "os_platform.h"
#include "pico_logger.h"

#include "irrigation_manager.h"
#include "telegram_manager.h"

#define IRRIGATION_CLOSED 0xde0fb33f
#define IRRIGATION_OPEN 0x10203040

uint32_t __uninitialized_ram(last_irrigation_state);


IrrigationManager *IrrigationManager::s_instance = nullptr;

IrrigationManager *IrrigationManager::get_instance()
{
    if (s_instance == NULL)
    {
        s_instance = new IrrigationManager();
    }

    return s_instance;
}

void IrrigationManager::initialize()
{
    // on boot we want to make sure irrigation is off in case we crashed or power out or anything in between.
    // should use an LFS file to check if it's already closed, but safe enough for now in case we get rebooted constantly
    if (last_irrigation_state != IRRIGATION_CLOSED)
    {
        close_all(nullptr);
        
        last_irrigation_state = IRRIGATION_CLOSED;
    }
}

void IrrigationManager::set_active(bool active)
{
    m_active = active;

    if (active)
    {
        // If at any point we were active, set flag so we close pumps on next restart for safety.
        last_irrigation_state = IRRIGATION_OPEN;
    }
}

bool IrrigationManager::set_valve(uint32_t id, uint8_t pin_fwd, uint8_t pin_back, uint8_t pin_sleep)
{
    if (id >= MAX_VALVES)
    {
        trace("IrrigationManager::set_valve: this=%p, failed to set valve with id[%d] > MAX_VALVES[%d]", this, id, MAX_VALVES);
        return false;
    }

    bool init_pin_sleep = true;

    for (int i=0;i<MAX_VALVES;++i)
    {
        if (m_valves[i].configured)
        {
            if ((pin_fwd == m_valves[i].pin_fwd) ||
                (pin_fwd == m_valves[i].pin_back) ||
                (pin_fwd == m_valves[i].pin_sleep))
            {
                trace("IrrigationManager::set_valve: this=%p, pin_fwd[%d] is used for another binding[%d/%d/%d], environment cannot work", this, pin_fwd, i, m_valves[i].pin_fwd, m_valves[i].pin_back, m_valves[i].pin_sleep);
                return false;
            }
            
            if ((pin_back == m_valves[i].pin_fwd) ||
                (pin_back == m_valves[i].pin_back) ||
                (pin_back == m_valves[i].pin_sleep))
            {
                trace("IrrigationManager::set_valve: this=%p, pin_back[%d] is used for another binding[%d/%d/%d], environment cannot work", this, pin_back, i, m_valves[i].pin_fwd, m_valves[i].pin_back, m_valves[i].pin_sleep);
                return false;
            }
            
            if ((pin_sleep == m_valves[i].pin_fwd) ||
                (pin_sleep == m_valves[i].pin_back))
            {
                trace("IrrigationManager::set_valve: this=%p, pin_sleep[%d] is used for another binding[%d/%d/%d], environment cannot work", this, pin_sleep, i, m_valves[i].pin_fwd, m_valves[i].pin_back, m_valves[i].pin_sleep);
                return false;
            }
            
            if (pin_sleep == m_valves[i].pin_sleep)
            {
                init_pin_sleep = false;
            }
        }
    }

    m_valves[id].pin_fwd = pin_fwd;
    m_valves[id].pin_back = pin_back;
    m_valves[id].pin_sleep = pin_sleep;

    if (init_pin_sleep)
    {
        gpio_init(pin_sleep);
        gpio_set_dir(pin_sleep, GPIO_OUT);
        gpio_put(pin_sleep, 0);
    }
        
    gpio_init(pin_fwd);
    gpio_set_dir(pin_fwd, GPIO_OUT);
    gpio_put(pin_fwd, 0);
    
    gpio_init(pin_back);
    gpio_set_dir(pin_back, GPIO_OUT);
    gpio_put(pin_back, 0);

    m_valves[id].configured = true;
    
    return true;
}

void Button::init(uint8_t pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);

    m_pin = pin;
}

bool Button::is_pressed(uint64_t now_us)
{
    if (m_pin == 255)
    {
        return false;
    }

    if (!m_initialized)
    {
        m_initialized = true;
        m_last_update_us = now_us;
        m_active_time_us = 0;
        m_latched = true;
    }

    uint64_t diff_us = (now_us - m_last_update_us);

    if (diff_us > 1000000ULL)
    {
        trace("Button::is_pressed: this=%p, diff_us=%llu, now_us=%llu, last_update=%llu, update took too long, ignoring, check for performance issues", this, (unsigned long long)diff_us, (unsigned long long)now_us, (unsigned long long)m_last_update_us);
        diff_us = 0;
    }
    
    if (gpio_get(m_pin) == 0)
    {
        m_active_time_us = m_active_time_us + diff_us;

        if (m_active_time_us > 1200000LL)
        {
            m_active_time_us = 1200000LL;
        }
    }
    else
    {
        m_active_time_us = (diff_us > m_active_time_us) ? 0 : m_active_time_us - diff_us;
    }

    m_last_update_us = now_us;

    if (m_active_time_us > 1000000LL)
    {
        return true;
    }
    
    return false;
}

bool Button::activated(uint64_t now_us)
{
    bool over = is_pressed(now_us);
    bool edge = over && !m_latched;
    
    if (over)
    {
        m_latched = true;
    }

    if (m_active_time_us == 0)
    {
        m_latched = false;
    }
    
    return edge;
}


bool IrrigationManager::set_buttons(uint8_t pin_on, uint8_t pin_off)
{
    trace("IrrigationManager::set_buttons: this=%p, on=%d off=%d", this, pin_on, pin_off);

    m_green_button.init(pin_on);
    m_red_button.init(pin_off);

    return true;
}

bool IrrigationManager::open_valve(uint32_t id, uint32_t seconds, const char *reason)
{
    if (id >= MAX_VALVES)
    {
        trace("IrrigationManager::open_valve: this=%p, bad id[%d]", this, id);
        return false;
    }

    if (!m_valves[id].configured || seconds == 0 || seconds > MAX_VALVE_SECONDS)
    {
        trace("IrrigationManager::open_valve: this=%p, id=%d, configured=%d, valve=%d/%d/%d, seconds=%d. Bad parameters given", this, id, m_valves[id].configured, m_valves[id].pin_fwd, m_valves[id].pin_back, m_valves[id].pin_sleep, seconds);
        return false;
    }

    trace("IrrigationManager::open_valve: this=%p, id=%d seconds=%d reason=%s", this, id, seconds, safestr(reason));

    set_active(true);
    
    m_valves[id].active = true;
    uint64_t time = to_us_since_boot(get_absolute_time());
    m_valves[id].shutoff_time_us = time + (static_cast<uint64_t>(seconds) * 1000000ULL);
    m_valves[id].open_time_us = time;
    
    gpio_put(m_valves[id].pin_back, 0);
    gpio_put(m_valves[id].pin_fwd, 1);
    gpio_put(m_valves[id].pin_sleep, 1);

	// It's more important to set pins back properly then to make this async.
    sleep_ms(200);

    gpio_put(m_valves[id].pin_sleep, 0);
    gpio_put(m_valves[id].pin_fwd, 0);
    gpio_put(m_valves[id].pin_back, 0);

    if (reason != nullptr)
    {
        TelegramManager::get().sendFormattedMessage("Opened valve: %d for %d%s, %s", id, (seconds % 60) ? seconds : (seconds / 60), (seconds % 60) ? "s" : "m", reason);
    }
    return true;
}

bool IrrigationManager::close_valve(uint32_t id, const char *reason)
{
    if (id >= MAX_VALVES)
    {
        trace("IrrigationManager::close_valve: this=%p, bad id[%d]", this, id);
        return false;
    }

    if (!m_valves[id].configured)
    {
        trace("IrrigationManager::close_valve: this=%p, id=%d, unconfigured valve[%d/%d/%d]", this, id, m_valves[id].pin_fwd, m_valves[id].pin_back, m_valves[id].pin_sleep);
        return false;
    }
    
    trace("IrrigationManager::close_valve: this=%p, id=%d reason=%s", this, id, safestr(reason));
    
    m_valves[id].active = false;
    m_valves[id].shutoff_time_us = 0;

    gpio_put(m_valves[id].pin_fwd, 0);
    gpio_put(m_valves[id].pin_back, 1);
    gpio_put(m_valves[id].pin_sleep, 1);

	// It's more important to set pins back properly then to make this async.
    sleep_ms(200);
    
    gpio_put(m_valves[id].pin_sleep, 0);
    gpio_put(m_valves[id].pin_fwd, 0);
    gpio_put(m_valves[id].pin_back, 0);

    if (reason != nullptr)
    {
        TelegramManager::get().sendFormattedMessage("Closed valve: %d, %s", id, reason);
    }
    return true;

}

bool IrrigationManager::open_all(uint32_t seconds, const char *reason)
{
    if (seconds == 0 || seconds > MAX_VALVE_SECONDS)
    {
        trace("IrrigationManager::open_all: this=%p, seconds=%d. Bad parameters given", this, seconds);
        return false;
    }

    trace("IrrigationManager::open_all: this=%p, seconds=%d, reason=%s", this, seconds, safestr(reason));

    set_active(true);

    uint64_t time = to_us_since_boot(get_absolute_time());

    for (int i=0;i<MAX_VALVES;++i)
    {
        if (m_valves[i].configured)
        {
            gpio_put(m_valves[i].pin_back, 0);
            gpio_put(m_valves[i].pin_fwd, 1);

            m_valves[i].active = true;
            m_valves[i].shutoff_time_us = time + (static_cast<uint64_t>(seconds) * 1000000ULL);
            m_valves[i].open_time_us = time;

        }
    }

    for (int i=0;i<MAX_VALVES;++i)
    {
        if (m_valves[i].configured)
        {
            gpio_put(m_valves[i].pin_sleep, 1);
        }
    }

	// It's more important to set pins back properly then to make this async.
    sleep_ms(200);
    
    for (int i=0;i<MAX_VALVES;++i)
    {
        if (m_valves[i].configured)
        {
            gpio_put(m_valves[i].pin_fwd, 0);
            gpio_put(m_valves[i].pin_back, 0);
            gpio_put(m_valves[i].pin_sleep, 0);
        }
    }

    if (reason != nullptr)
    {
        TelegramManager::get().sendFormattedMessage("Opened all valves for %d%s, %s", (seconds % 60) ? seconds : (seconds / 60), (seconds % 60) ? "s" : "m", reason);
    }
    return true;
}

bool IrrigationManager::close_all(const char *reason)
{
    trace("IrrigationManager::close_all: this=%p reason=%s", this, safestr(reason));
    
    for (int i=0;i<MAX_VALVES;++i)
    {
        if (m_valves[i].configured)
        {
            gpio_put(m_valves[i].pin_fwd, 0);
            gpio_put(m_valves[i].pin_back, 1);

            m_valves[i].active = false;
            m_valves[i].shutoff_time_us = 0;
            m_valves[i].open_time_us = 0;
        }
    }

    for (int i=0;i<MAX_VALVES;++i)
    {
        if (m_valves[i].configured)
        {
            gpio_put(m_valves[i].pin_sleep, 1);
        }
    }

	// It's more important to set pins back properly then to make this async.
    sleep_ms(200);
    
    for (int i=0;i<MAX_VALVES;++i)
    {
        if (m_valves[i].configured)
        {
            gpio_put(m_valves[i].pin_fwd, 0);
            gpio_put(m_valves[i].pin_back, 0);
            gpio_put(m_valves[i].pin_sleep, 0);
        }
    }

    set_active(false);

    if (reason != nullptr)
    {
        TelegramManager::get().sendFormattedMessage("Closed all valves, %s", reason);
    }
    return true;

}

bool IrrigationManager::is_valve_configured(uint32_t id) const
{
    return (id < MAX_VALVES) && m_valves[id].configured;
}

bool IrrigationManager::is_valve_open(uint32_t id) const
{
    return is_valve_configured(id) && m_valves[id].active;
}

uint32_t IrrigationManager::valve_seconds_left(uint32_t id, uint64_t now_us) const
{
    if (!is_valve_open(id))
    {
        return 0;
    }

    int64_t left_us = static_cast<int64_t>(m_valves[id].shutoff_time_us - now_us);

    return (left_us > 0) ? static_cast<uint32_t>(left_us / 1000000ULL) : 0;
}

void IrrigationManager::update(uint64_t now_us)
{
    static constexpr uint64_t MAX_RUNTIME_US = MAX_VALVE_SECONDS * 1000000ULL;

    bool active = false;
        
    for (int i=0;i<MAX_VALVES;++i)
    {
        if (m_valves[i].configured && m_valves[i].active)
        {
            bool past_deadline = static_cast<int64_t>(now_us - m_valves[i].shutoff_time_us) > 0;
            bool past_ceiling  = now_us - m_valves[i].open_time_us > MAX_RUNTIME_US;
            if (past_deadline || past_ceiling)
            {
                trace("IrrigationManager::update: this=%p, id=%d, finished shutoff", this, i);
                close_valve(i, "finished");
            }
        }
        
        active |= (m_valves[i].configured && m_valves[i].active);
    }

    bool red_pressed = m_red_button.activated(now_us);
    bool green_pressed = m_green_button.activated(now_us); 
    
    if (red_pressed)
    {
        trace("IrrigationManager::update: this=%p, red button pressed", this);

        close_all("Red Button");
    }
    else if (green_pressed && !active)
    {
        trace("IrrigationManager::update: this=%p, green button pressed", this);

        open_all(1800, "Green Button");
    }
    else
    {
        set_active(active);
    }
}
