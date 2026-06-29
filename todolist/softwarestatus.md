# ESP DRONE REMOTEID — Complete Software Status

> Last updated: 2026-06-29 (v5)
> Scope: all 80+ source files in repository (excluding build artifacts, __pycache__)

---

## `ESP32_DRONE_REMOTE_ID_Firmware/` — Firmware Root

### `CMakeLists.txt` (18 lines)
- Root CMake. Sets project name `esp_remote_id`, registers main component + `esp_remote_id`, includes `mavlink/` as an extra component directory.
- **OK.** No changes needed.

### `sdkconfig.defaults` (5 lines)
- `CONFIG_BT_ENABLED=y`, `CONFIG_MBEDTLS_SHA256_ENABLED=y`, plus `CONFIG_*_IGNORE_MAC_CRC_ERROR=y` for 3 targets.
- **OK.** Minimal baseline; CI overrides per target.

### `sdkconfig` (3461 lines, generated)
- ✅ Ignored via `.gitignore`, not tracked in git. CI regenerates it fresh per target.
- Informational reference for local builds only.

### `partitions.csv` (7 lines)
- 4 MB flash layout: nvs 20K + otadata 8K + phy_init 4K + factory 1920K + ota_0 1920K.
- **OK.** Standard OTA dual-slot.

### `partitions_2mb.csv` (7 lines)
- 2 MB flash layout: nvs 20K + otadata 8K + phy_init 4K + factory 960K + ota_0 960K.
- Used by ESP32-C2 in CI (`ESP_DRONE_REMOTE_ID_Firmware/build.yml`).
- **OK.**

### `idf_component.yml` (12 lines)
- ESP-IDF registry manifest. Version `1.0.0-beta`.
- ✅ Description fixed to `"ESP32 Remote ID Transmitter"` (was `"Scanner"`).
- **OK.**

---

### `main/CMakeLists.txt` (3 lines)
- Registers `main.c`, requires `esp_remote_id` component.
- **OK.**

### `main/Kconfig.projbuild`
- **Does not exist.** No main-level kconfig. All config is in `components/esp_remote_id/Kconfig.projbuild`.
- **OK** by design.

### `main/main.c` (107 lines)
- Entry point: `app_main()` → `fix_mac_if_needed()` → `esp_rid_init()` → `esp_rid_start()` → `print_splash()`.
- ✅ ASCII art splash with MAC AP, version, config URL.
- **OK.**

---

## `components/esp_remote_id/` — Core Component

### `CMakeLists.txt` (49 lines)
- Registers all .c files in `src/` (23 sources). REQUIRES `nvs_flash`, `efuse`, `esp_wifi`, `esp_bt`, `esp_driver_ledc`, `esp_driver_rmt`, `esp_driver_twai`, `mbedtls`, etc.
- ✅ Added `src/rid_kalman.c`, `src/rid_mavlink_tx.c`, `src/rid_auth.c`, `src/rid_ota.c`, `src/led_ws2812.c`, `src/rid_lighting.c`, `src/rid_dronecan.c`, `src/rid_mavlink_usb.c` to SRCS.
- ✅ Added `esp_driver_rmt` for WS2812 addressable LED.
- ✅ Added `esp_driver_twai` for DroneCAN (CAN bus).
- ✅ `-Wno-error=address-of-packed-member` suppresses MAVLink struct packing warnings.

### `Kconfig.projbuild` (27 lines)
- Menuconfig: `RID_LED_R_GPIO`, `RID_LED_G_GPIO`, `RID_LED_B_GPIO` (int, default -1/disabled).
- **OK.** Allows configuring RGB LED pins without code changes.

---

### `include/` — Headers

#### `esp_remote_id.h` (200 lines)
- Public API: `esp_rid_init()`, `esp_rid_start()`, `esp_rid_stop()`, `esp_rid_factory_reset()`, `esp_rid_set_config()`, `esp_rid_get_config()`, `esp_rid_get_state()`.
- Structs: `rid_config_t` (160 lines, major expansion), `rid_state_t`, `rid_gps_data_t`, `rid_identity_t`.
- Options flags: `RID_OPT_FORCE_ARM_OK`, `RID_OPT_DONT_SAVE_BASIC_ID`, `RID_OPT_PRINT_RID_MAVLINK`, `RID_OPT_DEMO_MODE`, `RID_OPT_KALMAN_FILTER`, `RID_OPT_AUTH_ED25519`, `RID_OPT_MAVLINK_ARM_STATUS`, `RID_OPT_MAVLINK_OP_LOC_LOOP`, `RID_OPT_IDENTITY_READY_GATE`.
- `rid_identity_t` expanded: `has_self_id`, `self_id_desc_type`, `has_ext_auth`, `ext_auth_last_page`, `ext_auth_pages_received`, `ext_auth_pages[16][25]`.
- `rid_config_t` expanded: `ws2812_gpio`, `ws2812_brightness`, `lighting_pins[5]`, `lighting_patterns[5]`, `lighting_phase_offsets[5]`, `dronecan_rx/tx_gpio`, `dronecan_bitrate`, `mavlink_usb_enable`, `ota_trigger_gpio`, `auth_private_key[512]`, `start_delay_ms`, `public_keys[5][256]`.
- `rid_state_t` expanded: `identity_ready`, `mavlink_armed`, `mavlink_sysid`, `operator_lat/lon/alt`, `operator_position_updated_ms`, `operator_location_type`, `auth_enabled`.

#### `opendroneid.h` (762 lines)
- Intel Open Drone ID C library header: all ASTM enums, message types, encode/decode API.
- **OK.** Upstream library, no modifications needed.

#### `odid_wifi.h` (106 lines)
- IEEE 802.11 packed structs: management frame header, beacon, SSID, rates, vendor-specific IE, NAN attributes.
- `ODID_service_info` — message pack container.
- **OK.**

#### `wifi_tx.h` (30 lines)
- Declares: `wifi_tx_init()`, `wifi_tx_transmit()`, `wifi_tx_transmit_nan()`.
- **OK.**

#### `ble_tx.h` (11 lines)
- Declares: `ble_tx_init()`, `ble_tx_transmit_legacy()`, `ble_tx_transmit_lr()`, `ble_tx_set_power()`.
- ✅ `ble_tx_set_power()` — configurable BLE advertising power.

#### `mav2odid.h` (30 lines)
- Intel MAVLink-to-ODID conversion: `m2o_*` functions.
- ✅ Compiled and active via `mavlink_parser.c` include.
- **Gap:** Only the high-level `m2o_convert_*` wrappers are used; lower-level scheduling helpers are dead code.

#### `protocol_detect.h` (10 lines)
- Declares: `protocol_detect_init()`, `protocol_detect_get_protocol()`.
- **OK.**

#### `nmea_parser.h` (15 lines)
- Declares: `nmea_parser_init()`, `nmea_parser_get_data()`.
- **OK.**

#### `msp_parser.h` (15 lines)
- Declares: `msp_parser_init()`, `msp_parser_get_data()`.
- **OK.**

#### `mavlink_parser.h` (17 lines)
- Declares: `mavlink_parser_init()`, `mavlink_parser_get()`, `mavlink_parser_get_identity()`, `mavlink_parser_set_sysid_filter()`.
- ✅ Expanded API: `mavlink_parser_get_sysid()`, `mavlink_parser_get_armed()`, `mavlink_parser_get_operator_location()`, `mavlink_parser_set_operator_location()`.

#### `web_config.h` (15 lines)
- Declares: `web_config_init()`.
- **OK.**

#### `nvs_storage.h` (20 lines)
- Declares: `nvs_storage_init()`, `nvs_storage_save()`, `nvs_storage_load()`, `nvs_storage_erase()`.
- **OK.**

#### `led_status.h` (30 lines)
- Declares: `led_status_init()`, `led_status_reconfigure()`, `led_status_set_state()`, `led_status_tx_flash()`, `led_status_tick()`.
- Enum: `rid_led_state_t` (BOOT, NO_GPS, GPS_OK, DEMO, LOCKED, OTA, ERROR).
- **OK.**

#### `rid_patrol.h` (10 lines)
- Declares: `rid_patrol_tick()`.
- **OK.**

#### `cli.h` (7 lines)
- Declares: `cli_init()`.
- **OK.**

#### `rid_kalman.h` (39 lines)
- 1D Kalman filter struct (`rid_kalman_1d_t`), 3D aggregate (`rid_kalman_3d_t`).
- API: `init`, `update`, `predict`, `get` (lat/lon/alt/speed/climb/heading), `valid`, `valid_age` (3s timeout), `reset`.
- **OK.**

#### `rid_mavlink_tx.h` (10 lines)
- Declares: `rid_mavlink_tx_init()`, `rid_mavlink_tx_task()`.
- **OK.** Sends HEARTBEAT (1Hz) + operator-location SYSTEM (6s) on UART.

#### `rid_auth.h` (17 lines)
- Ed25519 authentication signing via mbedTLS PK.
- API: `rid_auth_init(pem_key)`, `rid_auth_enabled()`, `rid_auth_sign_message()`.
- Generates ASTM F3411-22a Authentication pages (4-byte header + 19-byte signature per page).
- **OK.**

#### `rid_ota.h` (9 lines)
- Declares: `rid_ota_check_and_run(cfg)`.
- **OK.** Wi-Fi AP mode OTA server with /update, /factory_reset, /rollback endpoints.

#### `led_ws2812.h` (12 lines)
- WS2812 addressable RGB LED via ESP RMT peripheral.
- API: `led_ws2812_init(gpio, brightness)`, `set_rgb(r,g,b)`, `set_hsv(hue,sat,val)`, `set_brightness(pct)`.
- **OK.**

#### `rid_lighting.h` (14 lines)
- 5-channel GPIO lighting outputs with configurable patterns.
- API: `rid_lighting_init(pins, patterns, phase_offsets)`, `tick()`, `set_state(armed, gps_valid)`.
- Patterns: OFF, SOLID, BLINK_SLOW, BLINK_FAST, BLINK_ARMED, FLASH_ON_GPS.
- **OK.**

#### `rid_dronecan.h` (11 lines)
- DroneCAN/CAN bus input via ESP TWAI controller.
- API: `rid_dronecan_init(rx, tx, bitrate)`, `rid_dronecan_get(gps)`, `rid_dronecan_is_active()`.
- Decodes Fix2 (lat/lon/alt/speed/heading/fix/sats) and AHRS.
- **OK.**

#### `rid_mavlink_usb.h` (8 lines)
- Declares: `rid_mavlink_usb_init()`.
- **OK.** MAVLink transport over USB Serial/JTAG (CDC-ACM).

---

### `src/` — Source Files

#### `esp_remote_id.c` (543 lines)
- Main orchestrator: init sequence, RTOS task loop, `update_transmissions()`, rate limiting.
- ✅ Kalman filter: `rid_kalman_update()` called after GPS update; `rid_kalman_predict()` before TX if GPS stale (< 3s); filtered values used when `RID_OPT_KALMAN_FILTER` enabled.
- ✅ Identity readiness gate: `update_identity_ready()` blocks TX until UAS ID + Operator ID are configured.
- ✅ MAVLink ARM_STATUS: `rid_mavlink_tx_init()` + periodic `rid_mavlink_tx_task()` sends HEARTBEAT on UART.
- ✅ Ed25519 auth init: `rid_auth_init()` called if `RID_OPT_AUTH_ED25519` and private key configured.
- ✅ OTA mode check: `rid_ota_check_and_run()` at boot before normal init.
- ✅ DroneCAN: `rid_dronecan_init()` if pins configured; `rid_dronecan_get()` polled in main loop.
- ✅ MAVLink USB: `rid_mavlink_usb_init()` if `mavlink_usb_enable` set.
- ✅ WS2812: `led_ws2812_init()` if GPIO configured; rainbow cycle during OTA.
- ✅ Lighting: `rid_lighting_init()` + `tick()` if pins configured.
- ✅ Operator location: `mavlink_parser_set_operator_location()` updates via MAVLink SYSTEM msg.
- ✅ Config get/set: `esp_rid_set_config()`, `esp_rid_get_config()` added for NVS round-trip.
- ✅ Factory reset: `esp_rid_factory_reset()` — erases NVS, falls back to defaults.

#### `wifi.c` (614 lines)
- IEEE 802.11 frame builder: NAN sync beacon, NAN action frame, legacy beacon, message pack.
- From Intel ODID library. Stable.
- **OK.**

#### `wifi_tx.c` (204 lines)
- `wifi_tx_init()`: SoftAP + promiscuous mode, random MAC generation.
- `wifi_tx_transmit()`: builds legacy beacon via `esp_wifi_80211_tx()`.
- `wifi_tx_transmit_nan()`: builds NAN action frame.
- ✅ Refactored: duplicate `g_uas_data` population → shared `populate_uas_data()`.
- **OK.**

#### `ble_tx.c` (198 lines)
- BLE 4.0 legacy advertising + BLE 5.0 Long Range (Coded PHY).
- ✅ `ble_tx_transmit_legacy()` uses `ADV_TYPE_SCAN_IND` for better compatibility.
- ✅ `ble_tx_set_power()` — configurable BLE TX power via `esp_ble_tx_power_set()`.
- ✅ Secondary Basic ID support (BasicIDValid[1]).
- ✅ Self-ID description relayed in BLE advertisements.
- **Gap:** BLE 5.0 LR Coded PHY requires ESP32-S3/C3 hardware — marked Beta.

#### `web_config.c` (735 lines)
- HTTP server: serves `config.html`, REST API (`/api/config`, `/api/status`, `/api/command`, `/api/logs`), OTA endpoint.
- ✅ Lock level: basic check blocks destructive commands at level ≥ 1, OTA at level ≥ 2.
- ✅ eFuse burn: writes `"RID!"` magic to `EFUSE_BLK3` on transition to level 2; `get_lock_level()` reads eFuse.
- ✅ OTA SHA-256: `X-Expected-SHA256` header mandatory. Rejects missing/mismatched hash.
- ✅ Signature verification: Level ≥ 1 requires `X-Signature` header with ECDSA P-256 signature over SHA-256 hash of JSON body. Uses `mbedtls/pk.h` for key parsing (PEM, DER, or `PUBLIC_KEYV1:` base64) and PSA for hashing.
- ✅ Command signature verification: `/api/command` now also verifies `X-Signature` for restart/reboot/reset/factory commands when locked. `/api/reset` endpoint also accepts signatures (body signed: `"factory_reset"`). Returns `invalid_signature` instead of `locked`.

#### `cli.c` (317 lines)
- Raw UART REPL (no external deps: no esp_console, no linenoise).
- 14 commands + simple argument parser via `fgets()` + `strcmp` dispatch.
- Commands: `help`, `status`, `config`, `restart`/`reboot`, `reset`/`factory`, `protocol`, `heap`, `log_level`, `patrol`, `transmit`, `mac`, `uptime`.
- Spawns `cli_task` (4 KB stack) and returns immediately.
- **OK.**

#### `protocol_detect.c` (75 lines)
- UART auto-detect: `$M<` → MSP, `$G/$N` → NMEA, `0xFE/0xFD` → MAVLink.
- **OK.**

#### `nmea_parser.c` (136 lines)
- Parses GGA + RMC sentences. Extracts lat/lon/alt/speed/heading/fix/sat.
- ✅ `altitude_baro` populated via MSL altitude fallback.
- **OK.**

#### `msp_parser.c` (126 lines)
- Parses MSP_RAW_GPS (106), MSP_ATTITUDE (108), MSP_STATUS (101).
- ✅ `altitude_baro` populated via MSL altitude fallback.
- **OK.**

#### `mavlink_parser.c` (276 lines)
- Parses GLOBAL_POSITION_INT, GPS_RAW_INT, VFR_HUD, ATTITUDE, HEARTBEAT, AHRS2.
- ✅ OPEN_DRONE_ID_LOCATION, BASIC_ID, OPERATOR_ID, SELF_ID, AUTHENTICATION, SYSTEM, MESSAGE_PACK.
- ✅ `mavlink_parser_get_identity()` — returns parsed ODID identity (basic, operator, self-id, auth pages).
- ✅ `mavlink_parser_get_sysid()` — returns MAVLink system ID from first valid message.
- ✅ `mavlink_parser_get_armed()` — returns armed status from HEARTBEAT base_mode.
- ✅ `mavlink_parser_get_operator_location()` / `set_operator_location()` — operator position with 30s timeout.
- ✅ OPEN_DRONE_ID_SYSTEM: operator lat/lon extracted and stored.
- ✅ OPEN_DRONE_ID_SELF_ID: self-id description + type relay.
- ✅ OPEN_DRONE_ID_AUTHENTICATION: auth pages relayed (up to 16 pages, bound checked).
- ✅ OPEN_DRONE_ID_MESSAGE_PACK: sub-message unpacking (9 messages max).
- ✅ `mavlink_parser_set_sysid_filter()` — optional sysid filtering (0 = accept all).
- Dialect: `ardupilotmega/mavlink.h`.
- 5s GPS timeout, 10s identity timeout, 30s operator location timeout.

#### `mav2odid.c` (636 lines)
- Intel MAVLink-to-ODID conversion library. Scheduling, buffer management, message conversion.
- ✅ Compiled and linked via `mavlink_parser.c` include.
- **Gap:** High-level scheduling logic unused; only `m2o_convert_*` wrappers reachable.

#### `opendroneid.c` (1477 lines)
- Intel ODID encode/decode library. All ASTM message types.
- **OK.** Upstream stable.

#### `nvs_storage.c` (190 lines)
- NVS persistence for `rid_config_t` in namespace `esp_rid`.
- ✅ Options field changed from `uint8_t` to `uint32_t` (`store_u32`/`load_u32_def`) for expanded option flags.
- ✅ Saves/loads: `options`, `public_keys[5]`, `operator_lat/lon/alt`, all per-rate/per-power floats.

#### `led_status.c` (199 lines)
- RGB LED via 3 configurable GPIOs (Kconfig) with LEDC PWM (5kHz, 8-bit).
- State machine: 7 states with color + pattern encoding.
- Patterns: SOLID, BLINK_1Hz, BLINK_4Hz, DOUBLE_BLINK, PULSE (triangular 2s), RAINBOW (cycling).
- TX flash: 80ms white flash superimposed on base state.
- States: BOOT (blue pulse), NO_GPS (yellow 1Hz), GPS_OK (green solid), DEMO (purple pulse), LOCKED (red double-blink), OTA (rainbow), ERROR (red 4Hz).
- **OK.**

#### `rid_patrol.c` (31 lines)
- Demo GPS patrol simulation: Rome Colosseum (41.9028, 12.4964), 200m radius, 6 m/s, altitude drift, heading calc.
- **OK.**

#### `led_ws2812.c` (110 lines)
- WS2812 addressable RGB LED via ESP RMT TX channel (10 MHz resolution).
- Uses `rmt_new_bytes_encoder` with GRB byte ordering, brightness scaling.
- HSV→RGB conversion with saturation/value handling.
- **OK.**

#### `rid_kalman.c` (146 lines)
- 3 independent 1D Kalman filters: lat (1e-9 Qpos, 1e-8 Qvel, 1e-9 R), lon (1.5e-9, 1.5e-8, 1.5e-9), alt (1.0, 10.0, 25.0).
- Predict step: position/velocity covariance propagation with discrete-time model.
- Update step: Kalman gain computation, state/covariance correction.
- `rid_kalman_get()`: computes ground speed (lat→vn, lon→ve via DEG2M_LAT), climb rate, heading from velocity.
- 3s timeout via `RID_KALMAN_TIMEOUT_US`.
- **OK.**

#### `rid_mavlink_tx.c` (59 lines)
- MAVLink heartbeat transmitter on UART_NUM_1.
- Sends `HEARTBEAT` (MAV_TYPE_ODID, 1 Hz) + `OPEN_DRONE_ID_SYSTEM` (operator location, 6s).
- Used by ARMED_STATUS feature for ArduPilot pre-arm gating.
- **Gap:** UART not shared with MAVLink RX parser (separate port).

#### `rid_auth.c` (107 lines)
- Ed25519 authentication via mbedTLS PK API.
- `rid_auth_init()`: parses PEM private key, validates Ed25519 curve.
- `rid_auth_sign_message()`: signs ASTM F3411-22a message payload with Ed25519, splits signature across Auth pages (4-byte header + 19-byte data per page).
- Returns `page_count` for multi-page Auth message encoding.
- **OK.**

#### `rid_ota.c` (202 lines)
- Standalone OTA update server mode: Wi-Fi AP (`RemoteID-OTA`) + HTTP server.
- `/update` — multipart firmware upload via `esp_ota_write` / `esp_ota_set_boot_partition`.
- `/factory_reset` — erases NVS + reboots.
- `/rollback` — sets running partition as boot partition + reboots.
- Triggered by GPIO pull-low on `ota_trigger_gpio` at boot.
- **OK.**

#### `rid_lighting.c` (101 lines)
- 5-channel GPIO lighting outputs with 6 configurable patterns.
- Patterns: OFF, SOLID, BLINK_SLOW (1 Hz), BLINK_FAST (2 Hz), BLINK_ARMED (only when armed), FLASH_ON_GPS.
- Each channel has configurable phase offset for staggered timing.
- Uses `gpio_set_level()` with periodic `tick()` driven by `esp_timer_get_time()`.
- **OK.**

#### `rid_dronecan.c` (142 lines)
- DroneCAN (CAN bus) input via ESP TWAI (Two-Wire Automotive Interface) driver.
- Decodes `uavcan.equipment.gnss.Fix2` (CAN ID 2000): lat/lon/alt/height/speed/heading/fix/sats.
- Stub decoders for `uavcon.equipment.ahrs.Solution` and `org.drone_id.Identity`.
- 5s GPS freshness timeout.
- Bitrate auto-select: 1M/500K/250K bps (default 1M).
- **OK.**

#### `rid_mavlink_usb.c` (42 lines)
- MAVLink transport over USB Serial/JTAG (CDC-ACM).
- Configures console UART port for MAVLink bidirectional communication.
- Companion computer link for MAVLink ODID messages.
- **OK.**

### `webui/` — Embedded Configuration UI

#### `config.html` (~1520 lines)
- Full configuration and control web interface (inline JS + CSS + HTML).
- Dark/light theme, dashboard, forms, logs, OTA, terminal, CLI.
- ✅ Animated splash (Mac-style stepped progress bar).
- ✅ Responsive (off-canvas sidebar, 3 breakpoints).
- ✅ WiFi NAN checkbox, bar chart, counters, purple styling.
- ✅ Guide descriptions on all tabs.
- ✅ Bug fixes: `setAllTx()` single call, Restart with `.catch()`, log pane grid.
- ✅ Lock level + eFuse + signature verification implemented.
- ✅ Command signature support: restart/reset/reboot buttons use `sendSigCmd()`, terminal input includes hidden `X-Signature` field, dialog command supports headers.
- **Gap:** Inline only (~1520 lines) — optional refactor to separate files.

---

## `mavlink/` — MAVLink v2 C Dialect Headers (auto-generated by `mavlink/generate.sh`)

### Active (used in build)
| Path | Status |
|------|--------|
| `ardupilotmega/` (whole dialect) | ✅ Used (`mavlink_parser.c` includes `ardupilotmega/mavlink.h`) |
| `common/` | ✅ Included by `ardupilotmega/ardupilotmega.h` |
| `minimal/` | ✅ Included by `common/common.h` |
| `protocol.h` | ✅ Core MAVLink protocol |
| `mavlink_types.h` | ✅ Core types |
| `mavlink_get_info.h` | ✅ Included by `common/common.h` |
| `uAvionix/` | 🔶 Included transitively by `ardupilotmega.h` but no uAvionix messages parsed |
| `icarous/` | 🔶 Included transitively but unused |

### Unused (can be pruned)
| Path | Reason |
|------|--------|
| `standard/` | Only `development/` includes it |
| `development/` | Not included by any active dialect |
| `ASLUAV/` | Unused dialect |
| `matrixpilot/` | Unused dialect |
| `test/` | Test data only |
| `mavlink_helpers.h` | Signing not implemented |
| `mavlink_sha256.h` | Only in `mavlink_helpers.h` |
| `mavlink_conversions.h` | Only in `mavlink_helpers.h` |
| All `testsuite.h` | Test data |
| 8 of 11 `message_definitions/*.xml` | Unused definitions |

---

## `RID_Hub/` — Ground Station (Electron + Python)

### Overview
- **Renamed from `ESP_DRONE_REMOTEID_Analyzer` to `RID_Hub`**.
- **Electron wrapper** (`main.js` + `package.json` + `preload.js`) spawns Python backend.
- Python backend runs as `python -m rid_hub serve` (WebSocket server + WiFi capture).
- Electron loads `rid_hub/web/index.html` as the renderer, connects via WebSocket.

### Structure
```
RID_Hub/
├── package.json          # Electron project manifest
├── main.js               # Electron main process (spawns Python, creates window)
├── preload.js            # Exposes WS_URL to renderer
├── rid_hub/              # Python package (rename of ESP_DRONE_REMOTEID_Analyzer)
│   ├── __init__.py       # Version 1.0.0
│   ├── __main__.py       # Entry: `python -m rid_hub serve|gui|tool`
│   ├── capture.py        # WiFi beacon capture via Scapy
│   ├── decoder.py        # ASTM F3411-22a pure Python decoder
│   ├── server.py         # WebSocket broadcast server
│   ├── gui.py            # Legacy pywebview GUI (Electron recommended)
│   ├── rid_cli.py        # Headless CLI
│   ├── build.spec        # PyInstaller spec
│   ├── web/              # Frontend (HTML/JS/CSS)
│   └── tools/            # Ground tools
└── requirements.txt      # Python dependencies
```

### Python Backend (`rid_hub/`)

#### `__init__.py` (3 lines)
- Package marker. Exports `__version__ = "1.0.0"`.
- Renamed from `ESP_DRONE_REMOTEID_Analyzer` → `rid_hub`.

#### `__main__.py` (40 lines)
- Entry point: `python -m rid_hub <serve|gui|scanner|mapper|bridge|...>`.
- `serve` — starts WebSocket server + capture (for Electron).
- `gui` — legacy pywebview desktop app.
- Tools: `scanner`, `mapper`, `bridge`, `timing`, `ble-check`, `serial`, `provision`, `verify`.
- **OK.** Electron-friendly entry point.

#### `capture.py` (~200 lines)
- WiFi beacon capture via Scapy (`sniff` on Dot11Beacon frames).
- `RIDCapture` class: interface listing, monitor-mode helpers.
- PCAP batch write, multi-channel hopping.
- **Gap:** Requires monitor-mode WiFi adapter + root/admin privileges.

#### `decoder.py` (510 lines)
- Pure Python ASTM F3411-22a packet decoder.
- All message types: Basic ID, Location, Auth, Self-ID, System, Operator ID, Pack.
- **OK.**

#### `server.py` (~387 lines)
- WebSocket broadcast server.
- `RIDDevice` dataclass with 500-point trail, session recording, CSV/KML export.
- Snapshot on WS connect, full command dispatch.
- **OK.**

#### `rid_cli.py` (115 lines)
- Headless CLI: `capture`, `serve` subcommands.
- **OK.**

#### `gui.py` (~91 lines)
- Legacy pywebview desktop app (Electron now recommended).
- Starts backend + opens native window.
- **OK.**

#### `build.spec` (56 lines)
- PyInstaller spec for Windows exe (name: `RID_Hub`).
- **OK.**

### Web Frontend (`rid_hub/web/`)

#### `index.html` (~202 lines)
- RID Hub branding. 5 tabs: Devices, Map, Timeline, Statistics, About.
- Leaflet map, detail panel, recording controls.
- **OK.**

#### `app.js` (~834 lines)
- WebSocket client, device table, Leaflet map, Chart.js charts.
- Session recording, replay, CSV/KML export.
- **OK.**

#### `style.css` (~271 lines)
- Clean dark/light theme, responsive.
- **OK.**

### Electron Shell

#### `main.js` (90 lines)
- Spawns `python -m rid_hub serve` as child process.
- Creates BrowserWindow loading `rid_hub/web/index.html`.
- Graceful shutdown on window close / app quit.
- **OK.**

#### `package.json` (60 lines)
- Electron project config with electron-builder.
- Targets: Windows (NSIS), macOS (DMG), Linux (AppImage).
- **OK.**

#### `preload.js` (6 lines)
- Exposes `WS_CONFIG` via contextBridge to renderer.
- **OK.**

### Ground Tools (`rid_hub/tools/`)
| # | File | Status |
|---|------|--------|
| G0 | `openapi_spec.yaml` | ✅ |
| G1 | `scanner_wifi_ble.py` | 🟡 skeleton |
| G2 | `ble_validation.py` | 🟡 skeleton |
| G3 | `serial_bridge.py` | 🟡 skeleton |
| G4 | `timing_analysis.py` | 🟡 skeleton |
| G5 | `public_key_verify.py` | 🟡 skeleton |
| G6 | `nvs_provisioning.py` | 🟡 skeleton |
| G7 | `mesh_mapper.py` | 🟡 skeleton |
| G8 | `meshtastic_bridge.py` | 🟡 skeleton |

### Ground Tools (`tools/`)

#### `__init__.py` (3 lines)
- Tools package marker.

#### `scanner_wifi_ble.py` (placeholder)
- WiFi + BLE RID receiver scanner for ground-side drone detection.
- Decodes RID transmissions from nearby drones in real time.
- Requires monitor-mode WiFi adapter + BLE adapter.

#### `mesh_mapper.py` (placeholder)
- Mesh network map visualizer.
- Plots detected drone positions and ESP-NOW mesh nodes on a Leaflet/interactive map.
- Reads session recordings or live WebSocket feed.

#### `meshtastic_bridge.py` (placeholder)
- Meshtastic LoRa bridge for long-range RID reception.
- Forwards decoded RID data to a Meshtastic device over serial.
- Enables km-range drone detection via LoRa mesh.

#### `timing_analysis.py` (placeholder)
- Packet interval timing analysis tool.
- Validates ASTM F3411-22a transmission rate compliance from PCAP/session data.

#### `ble_validation.py` (placeholder)
- Cross-platform BLE receiver test using `bleak`.
- Scans for RID BLE advertisements (0xFFFA service UUID) on macOS/Windows.

#### `serial_bridge.py` (placeholder)
- macOS serial bridge — socat wrapper for Docker ESP-IDF development.
- Bridges USB serial to TCP for container access.

#### `nvs_provisioning.py` (placeholder)
- NVS private key provisioning script.
- Flashes Ed25519 private keys + public keys to the NVS partition over serial.

#### `public_key_verify.py` (placeholder)
- ECDSA P-256 signature verification tool.
- Verifies signatures generated by the Web API lock system.

#### `openapi_spec.yaml` (85 lines)
- OpenAPI 3.0 specification for the ESP Remote ID Web API.
- Documents `/api/config`, `/api/status`, `/api/command`, `/api/logs`, `/update`, `/factory_reset`, `/rollback`.

### Analyzer Web UI (`web/`)

#### `index.html` (~140 lines)
- **v2:** 5 tabs: Devices (default), Map, Timeline, Statistics, About.
- **v2:** Toolbar with search/filter/export/replay controls.
- **v2:** Recording controls — Record/Stop buttons with pulsing REC badge.
- **v2:** Detail panel overlay — slides in from right on device click.
- **v2:** Replay file selector — dropdown to load previous sessions.
- **v2:** UA type distribution grid — metric cards in Statistics tab.
- Leaflet map, device table, packet log.
- **OK.**

#### `app.js` (~850 lines)
- WebSocket client, device table rendering, Leaflet map markers.
- **v2:** Drone trail polylines on map — 500-point path per device.
- **v2:** Detail panel — click device shows full detail, RSSI history, message timeline.
- **v2:** Session recording — Record/Stop/Replay UI logic.
- **v2:** CSV/KML download — fetch blob from server, trigger save.
- **v2:** UA-type search filter, RSSI quality filter (good/warn/bad), active-only toggle.
- **v2:** Desktop notifications — alerts for new devices when tab hidden.
- **v2:** Drone count over time — scrolling line chart.
- **v2:** Statistics tab — UA type distribution + top RSSI leaderboard.
- Dark mode toggle, stats (devices, IDs, packets/min).
- **OK.**

#### `style.css` (~320 lines)
- **v2:** Detail panel — fixed right-side slide-in panel with scroll.
- **v2:** Toolbar layout — horizontal button bar with search/filter controls.
- **v2:** Stat grid + metric cards — responsive grid for Statistics tab.
- **v2:** Responsive breakpoints — adapts to small windows.
- Dark/light theme, badge/tab/table styles.
- **OK.**

---

## `.github/` — CI/CD & Community

### `workflows/build.yml` (66 lines)
- CI on push/PR to main. Matrix: 3 targets (esp32, esp32s3, esp32c6).
- All targets use `partitions.csv` + 4MB flash (esp32c2/esp32s2 conditionals removed).
- BT_NIMBLE enabled for esp32s3/esp32c6 only (esp32 uses classic BT).
- ccache, Python manifest generation, GitHub Pages deploy.
- **OK.**

### `workflows/release.yml` (308 lines)
- Trigger: tag `v*`. Same 3-target matrix (esp32, esp32s3, esp32c6).
- Generates zip per target + `manifest.json`, GitHub Release with auto-changelog.
- GitHub Pages deploy.
- **OK.**

### `dependabot.yml` (13 lines)
- Weekly updates for GitHub Actions. Europe/Rome timezone.
- **OK.**

### `ISSUE_TEMPLATE/bug_report.md` (45 lines)
- Hardware (ESP32 model, board, antenna, FC, GPS), firmware (version, target, protocol), config, console output.
- **OK.**

### `ISSUE_TEMPLATE/feature_request.md` (25 lines)
- Problem, solution, alternatives, affected protocols (MAVLink/MSP/NMEA/All).
- **OK.**

### `PULL_REQUEST_TEMPLATE.md` (38 lines)
- Checklist: coding style, hardware test, docs, build, warnings.
- **OK.**

---

## `.vscode/` — Editor Configuration

### `settings.json` (30 lines)
- Clangd config + IDF extension config.
- ✅ Absolute paths removed; uses `${workspaceFolder}` and extension variables.

### `launch.json` (10 lines)
- "Eclipse CDT GDB Adapter" attach config only.
- 🟢 Gitignored, local only. No OpenOCD or JTAG debug profiles.

### `c_cpp_properties.json` (20 lines)
- ESP-IDF IntelliSense config. `compileCommands`, include paths.
- ✅ Gitignored, local only. Hardcoded compiler path removed locally.

---

## `docs/` — GitHub Pages Site

### `index.html` (~901 lines)
- Quick Start landing page + WebSerial ESP Web Tools installer + built-in wiki sections 1–2.
- CSS custom properties design system (`--surface`, `--text`, `--accent`, `--shadow-*`, `--radius-*`).
- Inter font from Google Fonts, full-width layout, dark/light mode.
- Guide sections with gradient `::before` border-left pseudo-element, box-shadow hover.
- Tables: `border-radius` + `overflow:hidden`, `border-collapse:separate`, sticky headers.
- Callouts (info/warning/tip) with colored left border, `var()` backgrounds.
- Code blocks: dark `#1a1a2e` background with light `#e4e4ef` text even in light mode.
- Wiring cards: `box-shadow` + hover `translateY(-1px)`.
- Mermaid diagrams with animations (fade-in, dashed edge flow, hover glow).
- Release bar (version timeline), reading progress bar, TOC panel, lightbox for diagrams.
- Responsive (3 breakpoints), breadcrumbs, footer.
- Dark mode uses neutral black/gray (`#0d0d0d`, `#141414`, `#e0e0e0` etc.).
- **Gap:** Inline CSS/JS — optional refactor.

### `guide.html` (~1864 lines)
- Technical wiki: sections 3–16 (protocol detection, MAVLink/MSP/NMEA details, BLE, WiFi NAN, Web UI, OTA, API reference, lock security, troubleshooting).
- Same CSS design system as index.html with 16-color gradient palette for section headers.
- Protocol auto-detection Mermaid diagram with correct NMEA "No" path.
- Wiring diagrams, support matrices, table of contents, search.
- Dark mode uses same neutral black/gray as index.html.
- Mermaid animations, anchor links, copy buttons.
- **Gap:** Inline CSS/JS — optional refactor.

### `config(demo).html` (~2546 lines)
- Standalone offline demo of config UI. Full telemetry simulation:
  - GPS circular patrol (Rome Colosseum, 200m radius, ±15m altitude drift)
  - Battery drain (12.2V→10.8V), per-protocol counters, RSSI/noise, CPU/HEAP
  - Dynamic logs from 12 templates, simulated OTA progress bar
  - Fetch interceptor covers all `/api/*` endpoints
- ✅ Splash, responsive, icons, descriptions ported from config.html.
- **Gap:** Must be manually synced with `config.html` changes.

### `manifest.json` (57 lines)
- ESP Web Tools firmware manifest: 3 chip families (ESP32, ESP32-S3, ESP32-C6).
- Version `1.0.0-beta`, config URL `http://192.168.4.1`.
- **Gap:** Hardcoded version (should be auto-generated from CI).

### `innovation_diy.md` (294 lines)
- Competitive analysis vs Dronetag Mini 4G, DRI, BS gen.2, Beacon gen.2.
- Full innovation roadmap: ESP-NOW mesh, Kalman predictor, LoRa, ADSB, Edge ML.
- Comparison tables: features, power, size, cost, certifications.
- **OK.**

### `prototype_bom.md` (68 lines)
- Zero-solder BOM: Seeed XIAO ESP32-C6 + L76K GNSS, total ~15$.
- Pinout, wiring guide, stackable connector references.
- **OK.**

### `images/` (5 files)
- `logo.svg`, `logo con scritta.svg` — project logos.
- `ardupilot_logo.webp`, `betaflight_logo.svg`, `inav_logo.png` — FC compatibility icons.
- **OK.**

---

## Root Files

### `README.md` (~136 lines)
- Project overview, CI badges, 3 supported platforms (ESP32/S3/C6), quick start, wiring, build.
- ✅ Updated: protocol table (13 rows, 4 categories), XIAO ESP32-C6 recommended.
- ✅ Links to `docs/innovation_diy.md` and `docs/prototype_bom.md`.
- **Gap:** Mentions "no extra GPS needed" — firmware actually requires FC with GPS or demo mode.

### `.gitignore` (140 lines)
- Covers: build/, sdkconfig, __pycache__, *.bin, .vscode/*.json.
- `!.vscode/settings.json` — intentionally re-includes.
- **OK.**

### `.gitattributes` (40 lines)
- LF normalization for C/PY/JS/JSON/MD/YML/YAML; CRLF for BAT/PS1.
- **OK.**

### `LICENSE`
- ✅ Apache 2.0 license added. Derived from [Intel Open Drone ID](https://github.com/opendroneid).
- Copyright notice credits both VOLTEKOVER (port) and Intel Corporation (original ODID).

---

## `todolist/` — Documentation & Assets

### `softwarestatus.md` (this file)
- Comprehensive per-file analysis.
- **OK** (self-referencing).

### `guide.md`, `build_guide.md`, `faq.md`, `firmware_flashing_guide.md`
- ❌ **Removed from tree.** No longer present in `todolist/` (only `softwarestatus.md` remains).
- Content may have been merged into `docs/index.html`.

### `3D_case/`
- ❌ **Not in current tree.** STL/F3D files for 3D-printable enclosure.
- May have been relocated or removed.

---

## Roadmap

### Recently Done
| Item | Status | Date |
|------|--------|------|
| Kalman position predictor (1D×3 lat/lon/alt w/ velocity) | ✅ DONE | 2026-06-29 |
| WS2812 RGB LED via RMT (standalone HSV/RGB/brightness) | ✅ DONE | 2026-06-29 |
| Ed25519 auth (F3411-22a) — mbedTLS PK sign, multi-page encoding | ✅ DONE | 2026-06-29 |
| OTA Update Server — Wi-Fi AP + HTTP /update /factory_reset /rollback | ✅ DONE | 2026-06-29 |
| DroneCAN Input — TWAI receive, Fix2 decode | ✅ DONE | 2026-06-29 |
| MAVLink USB Serial/JTAG transport — native USB CDC | ✅ DONE | 2026-06-29 |
| MAVLink ARM_STATUS — HEARTBEAT out on UART (ODID type) | ✅ DONE | 2026-06-29 |
| GPIO Lighting Outputs — 5-channel, 6 patterns, phase offsets | ✅ DONE | 2026-06-29 |
| Identity readiness gate — block TX until UAS ID + Operator ID set | ✅ DONE | 2026-06-29 |
| Self-ID + Auth message relay from MAVLink | ✅ DONE | 2026-06-29 |
| MAVLink MESSAGE_PACK unpacking — sub-message decode | ✅ DONE | 2026-06-29 |
| Operator-location freshness loop — SYSTEM republish every 6s | ✅ DONE | 2026-06-29 |
| BLE TX power control — `ble_tx_set_power()` API | ✅ DONE | 2026-06-29 |
| Ground Tools directory structure in Analyzer (`tools/`) | ✅ DONE | 2026-06-29 |
| CI green on all 3 targets (esp32/s3/c6) | ✅ DONE | 2026-06-28 |
| GCC 15.2.0 fixes (led_status.c, cli.c, web_config.c) | ✅ DONE | 2026-06-27 |
| LEDC PWM LED state machine (7 states) | ✅ DONE | 2026-06-27 |
| Raw UART CLI (14 commands, no esp_console/linenoise) | ✅ DONE | 2026-06-27 |
| All docs English + XIAO ESP32-C6 kit comparison | ✅ DONE | 2026-06-27 |

---

### 🛩️ AIRCRAFT MODULE — ESP32 Firmware (`ESP32_DRONE_REMOTE_ID_Firmware/`)
*Funzionalità che hanno senso sull'ESP in volo.*

#### Immediate Next (weeks 1-2)
| # | Item | Source | Effort |
|---|------|--------|--------|
| 1 | **ESP-NOW mesh relay** — scopre peer ESP-NOW, multi-hop range extension per RID | Write from scratch | ~4 days |
| 2 | **RGB LED brightness Kconfig** — menuconfig per luminosità WS2812 + LEDC | Port from peinser | ~0.5 day |
| 3 | **Startup delay Kconfig** — ritardo d'avvio configurabile per attach serial monitor | Port from peinser | ~0.5 day |
| 4 | **BLE5 Long Range** — piena integrazione LE Coded PHY su ESP32-C6 | Integration | ~2 days |

#### Short Term (weeks 3-6)
| # | Item | Source | Effort |
|---|------|--------|--------|
| 5 | **Flash Encryption** — modalità dev/release, eFuse AES-256 | Port from peinser | ~2 days |
| 6 | **LoRa backup (SX1262)** — link di emergenza a lungo raggio | Write from scratch | ~5 days |
| 7 | **SD Card + geofence** — log di volo, no-fly zones | Write from scratch | ~4 days |
| 8 | **MAVLink stream filter doc** — raccomandazioni mavlink-router AllowMsgIdOut | Port from peinser (doc) | ~0.5 day |
| 9 | **Comprehensive NVS config** — persiste tutti i campi di `rid_config_t` | Port from peinser | ~1 day |

#### Medium Term (weeks 7-12)
| # | Item | Source | Effort |
|---|------|--------|--------|
| 10 | **opendroneid-core-c git submodule** — sostituisce lib ODID venduta con submodule ufficiale | Port from peinser | ~1 day |
| 11 | **Devcontainer + Makefile** — modernizzazione build system (container ESP-IDF, socat bridge) | Port from peinser | ~1 day |
| 12 | **Dual-core pinning** — Core 0 (WiFi TX) / Core 1 (BLE + UI) esplicito | Sky-Spy | ~1 day |

---

### 🖥️ GROUND TOOLS — Python Analyzer (`ESP_DRONE_REMOTEID_Analyzer/`)
*Tutti gli strumenti di terra, scanner, bridge, visualizzazione.*

#### Already Structured (skeleton files in `tools/`)
| # | File | Descrizione | Stato |
|---|------|-------------|-------|
| G0 | `tools/openapi_spec.yaml` | Specifica OpenAPI 3.0 delle API REST | ✅ DONE |

#### Immediate Next (weeks 1-2)
| # | Item | File | Effort |
|---|------|------|--------|
| G1 | **WiFi+BLE RID Receiver** — scanner RID in tempo reale (monitor-mode + BLE) | `tools/scanner_wifi_ble.py` | ~4 days |
| G2 | **BLE Validation Script** — test ricevitore BLE (bleak) su macOS/Windows | `tools/ble_validation.py` | ~0.5 day |
| G3 | **macOS Serial Bridge** — wrapper socat per sviluppo Docker | `tools/serial_bridge.py` | ~0.5 day |

#### Short Term (weeks 3-6)
| # | Item | File | Effort |
|---|------|------|--------|
| G4 | **Timing Analysis Tool** — analisi intervalli pacchetti RID da PCAP/sessioni | `tools/timing_analysis.py` | ~1 day |
| G5 | **ECDSA Public Key Verification** — verifica firme Lock System da riga di comando | `tools/public_key_verify.py` | ~1 day |
| G6 | **NVS Provisioning Script** — flash chiave Ed25519 + chiavi pubbliche su NVS via seriale | `tools/nvs_provisioning.py` | ~1 day |
| G7 | **French RID Format Support** — parsing variante francese RID (DIFF) | `tools/scanner_wifi_ble.py` | ~1 day |

#### Medium Term (weeks 7-12)
| # | Item | File | Effort |
|---|------|------|--------|
| G8 | **Mesh Mapper Visualizer** — mappa interattiva droni + nodi mesh | `tools/mesh_mapper.py` | ~3 days |
| G9 | **Meshtastic Bridge** — forwarding RID verso rete LoRa Meshtastic | `tools/meshtastic_bridge.py` | ~2 days |
| G10 | **JSON Serial Output** — output strutturato JSON per tools esterni | `tools/` | ~1 day |
| G11 | **Audio Buzzer Alerts** — allarmi acustici a terra su rilevamento drone | `tools/` | ~1 day |

#### Longer Term (weeks 13+)
| # | Item | File | Effort |
|---|------|------|--------|
| G12 | **Cloudbuild Web Configurator** — browser→sdkconfig→CI→ZIP | Nuovo | ~5 days |
| G13 | **WiFi Telemetry Bridge** — hotspot WiFi esistente per telemetria | Nuovo | ~2 days |

### Port Sources
> `peinser/esp-remoteid` (https://github.com/peinser/esp-remoteid) — fork avanzato con Ed25519 auth, OTA server, DroneCAN, MAVLink ARM_STATUS/MESSAGE_PACK/USB/op-loop/Self-ID-Auth-relay/readiness gate, flash encryption, WS2812, GPIO lighting, BLE TX Kconfig, startup delay, OpenAPI, devcontainer. **Quasi tutto portato** — mancano solo flash encryption, startup delay Kconfig, devcontainer.

> `colonelpanichacks/Sky-Spy` (https://github.com/colonelpanichacks/Sky-Spy) — Rilevatore RID WiFi promiscuo + BLE scanning, allarmi audio, output JSON, dual-core pinning, mesh-mapper.py. Complemento lato ricevitore. **Funzionalità ground spostate in Analyzer tools/.**

> `JimZGChow/wifi-rid-to-mesh` (https://github.com/JimZGChow/wifi-rid-to-mesh) — Scanner RID → bridge LoRa Meshtastic via seriale. Supporto formato RID francese.

> `PeterJBurke/esp32-c3-remote-id` (https://github.com/PeterJBurke/esp32-c3-remote-id) — Trasmettitore RID Arduino-based con timing analysis e hex dump Python.

---

## Priority Summary

| Priority | Item | Location | Status |
|----------|------|----------|--------|
| 🔴 HIGH | `sdkconfig` tracked in git | root | ✅ Already in `.gitignore`, not tracked |
| 🔴 HIGH | `LICENSE` missing | root | ✅ Apache 2.0 added |
| 🔴 HIGH | `docs/manifest.json` missing targets | `docs/manifest.json` | ✅ All 3 targets present |
| 🟡 MEDIUM | Lock level command signature | `web_config.c` + `config.html` | ✅ ECDSA P-256 via mbedTLS PK |
| 🟡 MEDIUM | BLE 5.0 LR Coded PHY = Beta | `ble_tx.c` | 🟡 Needs testing on S3/C6 |
| 🟡 MEDIUM | Analyzer requires monitor mode + root | `capture.py` | 🟡 Hardware limitation |
| 🟢 LOW | `c_cpp_properties.json` hardcoded path | `.vscode/c_cpp_properties.json` | ✅ Gitignored, local only — fixed locally |
| 🟢 LOW | `launch.json` no debug profiles | `.vscode/launch.json` | ✅ Gitignored, local only |
| 🟢 LOW | `config(demo).html` manual sync needed | `docs/config(demo).html` | 🟡 Maintenance burden |
| 🟢 LOW | `config.html` inline (~2340 lines) | `webui/config.html` | Optional refactor |
| 🟢 LOW | `docs/index.html` inline (~901 lines) | `docs/index.html` | Optional refactor |
| 🟢 LOW | `docs/guide.html` inline (~1864 lines) | `docs/guide.html` | Optional refactor |
| ✅ DONE | Kalman position predictor (1D×3) | `rid_kalman.c` + `esp_remote_id.c` | ✅ 2026-06-29 |
| ✅ DONE | WS2812 RGB LED via RMT | `led_ws2812.c/h` | ✅ 2026-06-29 |
| ✅ DONE | Ed25519 auth sign (F3411-22a pages) | `rid_auth.c/h` | ✅ 2026-06-29 |
| ✅ DONE | OTA Update Server (Wi-Fi AP + HTTP) | `rid_ota.c/h` | ✅ 2026-06-29 |
| ✅ DONE | DroneCAN CAN bus input (TWAI) | `rid_dronecan.c/h` | ✅ 2026-06-29 |
| ✅ DONE | MAVLink USB Serial/JTAG transport | `rid_mavlink_usb.c/h` | ✅ 2026-06-29 |
| ✅ DONE | MAVLink ARM_STATUS (HEARTBEAT out) | `rid_mavlink_tx.c/h` | ✅ 2026-06-29 |
| ✅ DONE | GPIO lighting outputs (5-ch, 6 patterns) | `rid_lighting.c/h` | ✅ 2026-06-29 |
| ✅ DONE | Identity readiness gate | `esp_remote_id.c` | ✅ 2026-06-29 |
| ✅ DONE | Self-ID + Auth relay from MAVLink | `mavlink_parser.c` | ✅ 2026-06-29 |
| ✅ DONE | MAVLink MESSAGE_PACK unpacking | `mavlink_parser.c` | ✅ 2026-06-29 |
| ✅ DONE | Operator-location freshness loop | `rid_mavlink_tx.c` | ✅ 2026-06-29 |
| ✅ DONE | BLE TX power control | `ble_tx.c` | ✅ 2026-06-29 |
| ✅ DONE | `altitude_baro` populated by all parsers | All parsers | ✅ |
| ✅ DONE | `force_tx` redundant logic fixed | `esp_remote_id.c` | ✅ |
| ✅ DONE | MAVLink ODID identity override | `mavlink_parser.c` + `esp_remote_id.c` | ✅ |
| ✅ DONE | eFuse lock level 2 burn | `web_config.c` | ✅ |
| ✅ DONE | OTA SHA-256 mandatory | `web_config.c` | ✅ |
| ✅ DONE | WiFi NAN enabled + UI | `wifi_tx.c` + `config.html` | ✅ |
| ✅ DONE | BLE SCAN_IND (nonconn→scan) | `ble_tx.c` | ✅ |
| ✅ DONE | Duplicate UAS data refactored | `wifi_tx.c` | ✅ |
| ✅ DONE | `.vscode/` absolute paths removed | `.vscode/*.json` | ✅ |
| ✅ DONE | Demo GPS patrol mode | `rid_patrol.c` + `main.c` | ✅ |
| ✅ DONE | `idf_component.yml` description | `idf_component.yml` | ✅ |
| ✅ DONE | `docs/index.html` BETA status | `docs/index.html` | ✅ |
| ✅ DONE | `README.md` outdated notes removed | `README.md` | ✅ |
| ✅ DONE | `docs/config(demo).html` simulation engine | `docs/config(demo).html` | ✅ |
| ✅ DONE | Wiki split (index.html + guide.html) | `docs/` | ✅ |
| ✅ DONE | Dark mode color fix (blue→neutral black) | `docs/index.html` + `docs/guide.html` | ✅ |
| ✅ DONE | Diagram animations + Mermaid bug fixes | `docs/index.html` + `docs/guide.html` | ✅ |
| ✅ DONE | Command signature for restart/reset | `web_config.c` + `config.html` | ✅ |
| ✅ DONE | CI target matrix reduced to 3 (esp32/s3/c6) | `.github/workflows/build.yml` | ✅ |
| ✅ DONE | `esp_driver_ledc` added to REQUIRES in CMakeLists.txt | `CMakeLists.txt` | ✅ |
| ✅ DONE | Protocol table added to README | `README.md` | ✅ |
| ✅ DONE | GCC 15.2.0 fix: struct tag name in led_status.c | `led_status.c` | ✅ |
| ✅ DONE | GCC 15.2.0 fix: heap_free/heap_total rename in cli.c | `cli.c` | ✅ |
| ✅ DONE | GCC 15.2.0 fix: unused label removed in web_config.c | `web_config.c` | ✅ |
| ✅ DONE | `docs/innovation_diy.md` — competitive analysis | `docs/innovation_diy.md` | ✅ |
| ✅ DONE | `docs/prototype_bom.md` — zero-solder BOM | `docs/prototype_bom.md` | ✅ |

---

## File Inventory (complete)

| # | File | Lines | Status |
|---|------|-------|--------|
| 1 | `ESP32_DRONE_REMOTE_ID_Firmware/CMakeLists.txt` | 18 | ✅ |
| 2 | `ESP32_DRONE_REMOTE_ID_Firmware/sdkconfig.defaults` | 5 | ✅ |
| 3 | `ESP32_DRONE_REMOTE_ID_Firmware/sdkconfig` | 3461 | ❌ generated (tracked) |
| 4 | `ESP32_DRONE_REMOTE_ID_Firmware/partitions.csv` | 7 | ✅ |
| 5 | `ESP32_DRONE_REMOTE_ID_Firmware/partitions_2mb.csv` | 7 | ✅ |
| 6 | `ESP32_DRONE_REMOTE_ID_Firmware/idf_component.yml` | 12 | ✅ |
| 7 | `ESP32_DRONE_REMOTE_ID_Firmware/main/CMakeLists.txt` | 3 | ✅ |
| 8 | `ESP32_DRONE_REMOTE_ID_Firmware/main/main.c` | 107 | ✅ |
| 9 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/CMakeLists.txt` | 49 | ✅ |
| 10 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/Kconfig.projbuild` | 27 | ✅ |
| 11 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/esp_remote_id.h` | 200 | ✅ |
| 12 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/opendroneid.h` | 762 | ✅ |
| 13 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/odid_wifi.h` | 106 | ✅ |
| 14 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/wifi_tx.h` | 30 | ✅ |
| 15 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/ble_tx.h` | 11 | ✅ |
| 16 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/mav2odid.h` | 30 | ✅ |
| 17 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/protocol_detect.h` | 10 | ✅ |
| 18 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/nmea_parser.h` | 15 | ✅ |
| 19 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/msp_parser.h` | 15 | ✅ |
| 20 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/mavlink_parser.h` | 17 | ✅ |
| 21 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/web_config.h` | 15 | ✅ |
| 22 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/nvs_storage.h` | 20 | ✅ |
| 23 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/led_status.h` | 30 | ✅ |
| 24 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/rid_patrol.h` | 10 | ✅ |
| 25 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/cli.h` | 7 | ✅ |
| 26 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/rid_kalman.h` | 39 | ✅ |
| 27 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/rid_mavlink_tx.h` | 10 | ✅ |
| 28 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/rid_auth.h` | 17 | ✅ |
| 29 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/rid_ota.h` | 9 | ✅ |
| 30 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/led_ws2812.h` | 12 | ✅ |
| 31 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/rid_lighting.h` | 14 | ✅ |
| 32 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/rid_dronecan.h` | 11 | ✅ |
| 33 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/include/rid_mavlink_usb.h` | 8 | ✅ |
| 34 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/esp_remote_id.c` | 543 | ✅ |
| 35 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/cli.c` | 313 | ✅ |
| 36 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/wifi.c` | 614 | ✅ |
| 37 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/wifi_tx.c` | 168 | ✅ |
| 38 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/ble_tx.c` | 198 | ✅ |
| 39 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/web_config.c` | 660 | ✅ |
| 40 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/protocol_detect.c` | 65 | ✅ |
| 41 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/nmea_parser.c` | 115 | ✅ |
| 42 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/msp_parser.c` | 110 | ✅ |
| 43 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/mavlink_parser.c` | 276 | ✅ |
| 44 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/mav2odid.c` | 557 | ✅ (partial use) |
| 45 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/opendroneid.c` | 1356 | ✅ |
| 46 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/nvs_storage.c` | 190 | ✅ |
| 47 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/led_status.c` | 182 | ✅ |
| 48 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/rid_patrol.c` | 27 | ✅ |
| 49 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/rid_kalman.c` | 146 | ✅ |
| 50 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/rid_mavlink_tx.c` | 59 | ✅ |
| 51 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/rid_auth.c` | 107 | ✅ |
| 52 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/rid_ota.c` | 202 | ✅ |
| 53 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/led_ws2812.c` | 110 | ✅ |
| 54 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/rid_lighting.c` | 101 | ✅ |
| 55 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/rid_dronecan.c` | 142 | ✅ |
| 56 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/src/rid_mavlink_usb.c` | 42 | ✅ |
| 57 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/webui/config.html` | ~2234 | ✅ |
| 58-137 | `ESP32_DRONE_REMOTE_ID_Firmware/components/esp_remote_id/mavlink/**/*.h/.xml` | ~80 files | 🔶 many unused |
| 138 | `ESP_DRONE_REMOTEID_Analyzer/__init__.py` | 3 | ✅ |
| 139 | `ESP_DRONE_REMOTEID_Analyzer/__main__.py` | 5 | ✅ |
| 140 | `ESP_DRONE_REMOTEID_Analyzer/capture.py` | 176 | ✅ |
| 141 | `ESP_DRONE_REMOTEID_Analyzer/decoder.py` | 510 | ✅ |
| 142 | `ESP_DRONE_REMOTEID_Analyzer/server.py` | 197 | ✅ |
| 143 | `ESP_DRONE_REMOTEID_Analyzer/rid_cli.py` | 115 | ✅ |
| 144 | `ESP_DRONE_REMOTEID_Analyzer/gui.py` | 109 | ✅ |
| 145 | `ESP_DRONE_REMOTEID_Analyzer/build.spec` | 56 | ✅ |
| 146 | `ESP_DRONE_REMOTEID_Analyzer/requirements.txt` | 18 | ✅ |
| 146b | `ESP_DRONE_REMOTEID_Analyzer/tools/__init__.py` | 3 | ✅ |
| 146c | `ESP_DRONE_REMOTEID_Analyzer/tools/scanner_wifi_ble.py` | placeholder | 🟡 skeleton |
| 146d | `ESP_DRONE_REMOTEID_Analyzer/tools/mesh_mapper.py` | placeholder | 🟡 skeleton |
| 146e | `ESP_DRONE_REMOTEID_Analyzer/tools/meshtastic_bridge.py` | placeholder | 🟡 skeleton |
| 146f | `ESP_DRONE_REMOTEID_Analyzer/tools/timing_analysis.py` | placeholder | 🟡 skeleton |
| 146g | `ESP_DRONE_REMOTEID_Analyzer/tools/ble_validation.py` | placeholder | 🟡 skeleton |
| 146h | `ESP_DRONE_REMOTEID_Analyzer/tools/serial_bridge.py` | placeholder | 🟡 skeleton |
| 146i | `ESP_DRONE_REMOTEID_Analyzer/tools/nvs_provisioning.py` | placeholder | 🟡 skeleton |
| 146j | `ESP_DRONE_REMOTEID_Analyzer/tools/public_key_verify.py` | placeholder | 🟡 skeleton |
| 146k | `ESP_DRONE_REMOTEID_Analyzer/tools/openapi_spec.yaml` | 85 | ✅ |
| 147 | `ESP_DRONE_REMOTEID_Analyzer/web/index.html` | 94 | ✅ |
| 148 | `ESP_DRONE_REMOTEID_Analyzer/web/app.js` | 438 | ✅ |
| 149 | `ESP_DRONE_REMOTEID_Analyzer/web/style.css` | 176 | ✅ |
| 150 | `docs/index.html` | ~901 | ✅ (inline, wiki split) |
| 151 | `docs/guide.html` | ~1864 | ✅ (inline, technical wiki) |
| 152 | `docs/config(demo).html` | ~2546 | ✅ |
| 153 | `docs/manifest.json` | 57 | ✅ (3 targets: esp32/s3/c6) |
| 154 | `docs/images/logo.svg` | — | ✅ |
| 155 | `docs/images/logo con scritta.svg` | — | ✅ |
| 156 | `docs/images/ardupilot_logo.webp` | — | ✅ |
| 157 | `docs/images/betaflight_logo.svg` | — | ✅ |
| 158 | `docs/images/inav_logo.png` | — | ✅ |
| 159 | `docs/innovation_diy.md` | 294 | ✅ |
| 160 | `docs/prototype_bom.md` | 68 | ✅ |
| 161 | `.github/workflows/build.yml` | 66 | ✅ |
| 162 | `.github/workflows/release.yml` | 308 | ✅ |
| 163 | `.github/dependabot.yml` | 13 | ✅ |
| 164 | `.github/ISSUE_TEMPLATE/bug_report.md` | 45 | ✅ |
| 165 | `.github/ISSUE_TEMPLATE/feature_request.md` | 25 | ✅ |
| 166 | `.github/PULL_REQUEST_TEMPLATE.md` | 38 | ✅ |
| 167 | `.vscode/settings.json` | 30 | ✅ |
| 168 | `.vscode/launch.json` | 10 | 🟢 gitignored, local only |
| 169 | `.vscode/c_cpp_properties.json` | 20 | 🟢 gitignored, local only |
| 170 | `README.md` | ~136 | ✅ |
| 171 | `.gitignore` | 140 | ✅ |
| 172 | `.gitattributes` | 40 | ✅ |
| 173 | `todolist/softwarestatus.md` | — | ✅ (this file) |
