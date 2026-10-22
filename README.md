# Pico Irrigation

Standalone Raspberry Pi Pico W Irrigation\
Controlled via Telegram/Google Calendar or an internal web page.

<img src="docs/pictures/01_main.jpg" alt="1. Main box" width="400">

| 2. Cover open | 3. Wires before the sheath | 4. USB power connector |
|---|---|---|
| ![2. Open](docs/pictures/02_open.jpg) | ![3. Wires](docs/pictures/03_before_sheathe.jpg) | ![4. USB](docs/pictures/04_usb_connector.jpg) |

Videos: [1](https://youtube.com/shorts/xUNLjCEPuNw), [2](https://youtube.com/shorts/3z41MsT0E34)

## History

This started as a MicroPython project controlled from a separate Pi 4\
That seemed wasteful, why not put telegram/google calendar directly on the pico?\
... *famous last words* ...

Some time later, it's working, efficiently, securely, all on pico\
To do that, I had to set up a whole firmware to handle things [os](https://github.com/AdrianCX/picohive-firmware)

While doing that, another idea, have a nice (optional) central web page to help manage things:
- Deploy firmware updates (encrypted/signed)
- View device logs (have devices send logs encrypted via UDP)
- View resolved crash callstacks and get alarms when devices crash

That's available at: https://picohive.com, signups free with device limits, will be open source if I ever close it down

As things usually go, why not add a linux build? valgrind/gdb would be nice, this will surely go faster ... \
... *famous last words again* ... See Linux simulator to build/run with hardware mocks

## How it looks

Pictures of web page in dark mode (it follows system settings, light mode is available as well)

| 5. General | 6. Telegram | 7. Calendar |
|---|---|---|
| ![5. General](docs/pictures/05_web_general_dark.png) | ![6. Telegram](docs/pictures/06_web_telegram_dark.png) | ![7. Calendar](docs/pictures/07_web_calendar_dark.png) |

The page has skins, because why not:

| 8. Original | 9. Xenoform | 10. Fallout |
|---|---|---|
| ![8. Original](docs/pictures/09_skin_original.png) | ![9. Xenoform](docs/pictures/10_skin_xenoform.png) | ![10. Fallout](docs/pictures/11_skin_fallout.png) |

### Telegram menu

Type `/start` in the chat and the bot replies with a menu (pin it to keep it handy): one row per valve (left, right, all), with off, 30s, 10m and 30m buttons.\
It also sends a message whenever a valve opens or closes.

![11. Telegram menu](docs/pictures/12_telegram_menu.jpg)

### Google Calendar

Any iCal compatible calendar will work, showing off google calendar since that's what I use:
- Event title issues commands, format: `<device id>/water/<valve>/<time>`
- Event start is what's used to trigger command, length doesn't matter, all day events are ignored
- Example: `garden/water/1/30m` opens valve 1 for 30 minutes, `garden/water/0/30s` opens all valves for 30s
- Hardcoded maximum limit of 60 minutes to prevent mishaps
- Only commands in the **Allowed** setting run (ex: `water stop`, or `water/1` for just valve 1)
- Recurring events work, delete a single occurrence to skip a day

![12. Google Calendar](docs/pictures/21_google_calendar.jpg)

## How do I set it up

#### What you may need before setting up

1. **Picohive** *(optional)*\
  Add Device on [picohive](https://picohive.com) (generate the UUID, MQTT password and log key, pick a name)\
  Open the device, go to **Device Config**, press **Export** and copy the line for step 4 below\

| 13. Add Device | 14. Device Config | 15. Export |
|---|---|---|
| ![13. Add Device](docs/pictures/13_picohive_add_device.png) | ![14. Device Config](docs/pictures/14_picohive_device_config.png) | ![15. Export](docs/pictures/15_picohive_export.png) |

2. **Telegram bot** *(optional)*\
  Create a bot: `/newbot` to BotFather, save the token, add bot to a group, type `/start choose-a-code`

3. **Google calendar** *(optional)*\
  Create a new calendar (that doesn't accept invites), save the private iCal address

#### Device setup

1. **Flash** a pico, it starts as access point `picow_config` / `randompassword` (yes, I know)
2. **Connect** to the access point, the page opens by itself, if not, use http://192.168.4.1
3. **Configure wifi** and a user/pass for the web ui
4. **Optional**:\
4.1. **Remote server** enter line from picohive for logging/OTA/crash reporting\
4.2. **Telegram** enter token, enable **Use the first chat...**, enter `choose-a-code` from setup\
4.3. **Calendar** enter private iCal address, a device id (ex: `garden`) and the allowed commands (ex: `water stop`)\
5. **Reboot** - everything works as configured - logs in picohive, events from telegram and calendar
6. **Reconfigure** any time from its new ip address (find it in picohive logs or on router)

| 16. Wifi and login | 17. Remote server | 18. Telegram | 19. Calendar |
|---|---|---|---|
| ![16. Wifi and login](docs/pictures/17_setup_general.png) | ![17. Remote server](docs/pictures/18_setup_server.png) | ![18. Telegram](docs/pictures/19_setup_telegram.png) | ![19. Calendar](docs/pictures/20_setup_calendar.png) |

> [!TIP]
> If you need to wipe, just use the [universal `flash_nuke.uf2`](https://github.com/Gadgetoid/pico-universal-flash-nuke) to wipe the flash then re-flash the irrigation firmware

### Buttons

- 🟢 **Green** opens both valves for 30 minutes
- 🔴 **Red** closes everything

## Building

```bash
git clone --recursive <this repo>
cd irrigation
os/build_via_podman.sh pico2_w      # or pico_w
```

Flash `build/<board>/irrigation.uf2` over USB\
Later updates for OTA need the encrypted file `irrigation.uf2.encrypted`\
Before you build your own, create your own OTA keys and certificate, as described in [os/docs/ota.md](os/docs/ota.md).

Layout:
- `src/main.cpp`: valve/button pins, Telegram menu, listeners
- `src/irrigation_manager.cpp`: valve timers and button handling
- `src/commands/`: `water`, `stop`, `status`
- `src/web/`: page and text config

## Linux simulator

The same app builds as a Linux program, with the hardware mocked\
You can use it to work on the web page and the command logic without flashing anything\
You can use gdb or valgrind as needed

```bash
os/build_via_podman.sh linux        # build/linux/irrigation
os/run_via_podman.sh                # http://localhost:8080, https://localhost:8443
os/run_via_podman.sh --valgrind     # or --gdb
```

## Memory usage

From the Pico W in my garden (HTTPS server is off on RP2040).\
MinFree is likely the peak: 3 TLS connections open at once (MQTT over TLS, Telegram, Calendar) plus the web page

```
[2026-10-20 19:29:33] 192.168.100.11 60 app[build/pico_w/irrigation] version[5087e39-dirty] os[v2.03-92-gadb6f5e-dirty] buildid[1b3204487b8a90a1c59ad4310d3cb7ebb97cc9ed] Heap[19144/228872] Free[209728] MinFree[131072] LargestFree[135124] Stack[2688/16384] lastRestartReason[reboot_command]
```

| | |
|---|---|
| Heap in use | 19KB of 224KB (8%) |
| Lowest free since restart | 128KB |
| Largest free block | 132KB, so fragmentation is low |
| Stack peak | 2.6KB of 16KB |
| Last restart | reboot command, not a crash |

## Hardware

Pictures 1-4 at the top.

![20. Wiring](docs/pictures/16_wiring.jpg)

Wiring:
- DRV8833: valve 1 on GPIO 16/17, valve 2 on GPIO 18/19, sleep on GPIO 15
- Buttons: green on GPIO 13, red on GPIO 12, other leg to GND (internal pull-up)

These are the defaults in `src/main.cpp`, change them there if you wire it differently

### Parts

I mostly used parts I already had, so there are simpler options if you're starting from scratch\
I would advise Pico2 W for more RAM / flash / secure boot / OTP

Main box:
- [Waterproof junction box](https://www.amazon.nl/-/en/Distributor-Waterproof-Surface-Mounted-Junction-Diameter/dp/B0BQBP43H2/)
- [Raspberry Pi Pico W](https://www.digikey.nl/nl/products/detail/raspberry-pi/SC0918/16608263)
- [DRV8833 motor driver](https://www.tinytronics.nl/en/mechanics-and-actuators/motor-controllers-and-drivers/stepper-motor-controllers-and-drivers/drv8833-bipolar-stepper-motor-and-dc-motor-motor-controller)
- [Perf board](https://www.digikey.nl/nl/products/detail/adafruit-industries-llc/1609/5353655), [pin headers](https://www.tinytronics.nl/en/cables-and-connectors/connectors/pin-headers/male/40-pins-header-male-long), [screw terminals](https://www.tinytronics.nl/en/cables-and-connectors/connectors/screw-terminals/2-pin-screw-terminal-block-connector-2.54mm-distance), [solid core wire](https://www.digikey.nl/nl/products/detail/adafruit-industries-llc/1311/6198255)
- [Green button](https://www.digikey.nl/nl/products/detail/adafruit-industries-llc/3487/7364334), [red button](https://www.digikey.nl/nl/products/detail/adafruit-industries-llc/3489/7349495)

Power:
- A long 2-wire cable carrying 5V ([example](https://www.amazon.nl/-/en/CARLITS-2x0-8mm²-Extension-Electric-Lighting/dp/B08F7TS37H/))
- 2x [polyfuses](https://www.tinytronics.nl/en/components/fuses/self-restoring-fuse-pptc-polyfuse-2000ma-through-hole) on a [proto board](https://www.digikey.nl/nl/products/detail/digikey/DKS-SOLDERBREAD-02/15970925) with screw terminals, one on 5V and one on GND (probably not needed, but in case of a short)
- 3500uF electrolytic capacitor across VSYS/GND, 10V or higher
- A USB power supply and a USB cable with one end cut off

Valves and plumbing:
- 2x [5V latching solenoid valves, G1/2](https://www.tinytronics.nl/en/mechanics-and-actuators/solenoids/solenoid-valves/solenoid-valve-latching-5v-brass-g1-2)
- [Heat shrink](https://www.amazon.nl/-/en/Preciva-Electric-Assortment-Electrical-Insulation/dp/B0778D22WM/) and 4m+ of [fiberglass sleeve](https://www.distrelec.biz/en/insulating-sleeve-6mm-red-brown-glass-fibre-silicone-bourgeois-pf03-062-08-321/p/15505284) for the wires
- [Water hammer arrestor](https://www.obadis.com/en/caleffi-antishock-wasserschlagdampfer-525150-uberwurfmutter-3-4-ig-x-3-4-ag-messing-gehause-verchromt.html). **This is important**: valves that snap shut can damage your pipes.
- [2-way manifold](https://www.amazon.nl/-/en/Manifold-Divider-Splitter-Connector-Separate/dp/B08XBPBCYF), [3/4" to 1/2" adapters](https://www.amazon.nl/dp/B09FKBYY5X), [Gardena tap connectors](https://www.amazon.nl/-/en/GARDENA-Premium-tap-connection-21mm/dp/B076WVH17N/), [hose ends](https://www.amazon.nl/-/en/18215-20/dp/B00PNUC9K6/)
- Rubber seals for every joint: [1/2"](https://www.amazon.nl/-/en/Prasacco-Shower-Sealing-Washers-Connection/dp/B0B9NGWQYR/), [3/4"](https://www.amazon.nl/-/en/Lvcky-Garden-Shower-Rings-Rubber/dp/B07GX8Z72J/)

## License

MIT for this app. The `os/` submodule is MPL-2.0, see its LICENSE.
