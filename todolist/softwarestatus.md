# ESP DRONE REMOTEID — Complete Code Analysis

## `ESP32_DRONE_REMOTE_ID_Firmware/`

### `main/main.c` — Entry point
- Calls `esp_rid_init()` + `esp_rid_start()`, prints splash
- **OK.** No issues.

### `main/CMakeLists.txt`
- Compiles `main.c`
- **OK.**

### `CMakeLists.txt` (firmware root)
- Defines components, includes `mavlink/`, registers `esp_remote_id`
- **OK.**

### `idf_component.yml` — ESP-IDF registry manifest
- Version `1.0.0-beta`, wrong description ("Scanner" instead of "Transmitter")
- ❌ **Outdated description**: says "OpenDroneID WiFi Beacon Scanner"

### `sdkconfig` — Generated ESP-IDF config (3461 lines)
- ❌ **Generated file tracked in git**. `sdkconfig` is already in `.gitignore` but the current one is committed. Should be removed from tracking.

### `sdkconfig.defaults` — Default configuration
- Only `CONFIG_BT_ENABLED=y`
- **OK.** The rest is set by CI.

### `partitions.csv` — 4MB partition table (ESP32/S3/C3/C6)
- factory 1920KB + ota_0 1920KB + nvs 20KB + coredump 8KB
- **OK.**

### `partitions_2mb.csv` — 2MB partition table (ESP32-C2)
- factory 960KB + ota_0 960KB + nvs 20KB + coredump 8KB
- **OK.**

---

## `components/esp_remote_id/include/`

### `esp_remote_id.h` — Public API, structs, enums
- Defines `rid_config_t`, `rid_state_t`, `rid_gps_data_t`, `rid_identity_t`
- **`altitude_baro`** (line 37): field defined in `rid_gps_data_t` but **never populated** by any parser. Always stays 0.
- **`RID_OPT_FORCE_ARM_OK`** (line 12): redundant logic in `esp_remote_id.c:286`

### `opendroneid.h` — Intel ODID library header (762 lines)
- All ASTM enums + encode/decode API
- **OK.**

### `odid_wifi.h` — IEEE 802.11 packed structs + NAN frame definitions
- Packed structures for beacon/NAN/service discovery
- **OK.**

### `wifi_tx.h`
- Declarations: `wifi_tx_init()`, `wifi_tx_transmit()`, `wifi_tx_transmit_nan()`
- **OK.**

### `ble_tx.h`
- Declarations: `ble_tx_init()`, `ble_tx_transmit_legacy()`, `ble_tx_transmit_lr()`
- **OK.**

### `mav2odid.h` — Intel MAVLink-to-ODID conversion header
- `m2o_*` declarations for converting MAVLink RID messages to ODID
- ❌ **Never called at runtime.** `esp_remote_id.c` uses `mavlink_parser.c` directly. Module compiled but dead.

### `protocol_detect.h`, `nmea_parser.h`, `msp_parser.h`, `mavlink_parser.h`
- Serial parser headers
- **OK.**

### `web_config.h`
- `web_config_init()`
- **OK.**

### `nvs_storage.h`
- `nvs_storage_init/save/load/erase`
- **OK.**

### `led_status.h`
- `led_status_init()`, `led_status_update()`
- **OK.**

---

## `components/esp_remote_id/src/`

### `esp_remote_id.c` — Main orchestrator (350 lines)
- Init sequence, task loop, `update_transmissions()`, rate limiting
- **Rate limiting** — FIXED: `rate_allowed()` uses `esp_timer_get_time()` to respect `rate_hz`
- **WiFi NAN** — FIXED: now called in `update_transmissions()`
- **GPS staleness** — FIXED: `g_state.gps_valid` set to false after 10s without updates
- **LED integration** — FIXED: `led_status_update()` called after each tx
- ❌ **Redundant `force_tx` logic** (lines 284-286): `if (gps_data.fix_type >= 2 && ...)` then `if (force_tx || gps_data.fix_type >= 2)` — the second condition is **always true** because the first already guarantees it. The `force_tx` flag never takes effect.

### `wifi.c` — IEEE 802.11 frame builder (614 lines)
- Builds beacon + NAN action frame
- **OK.** Stable code from Intel ODID library.

### `wifi_tx.c` — WiFi transmission (204 lines)
- `wifi_tx_init()`: AP + promiscuous mode, random MAC
- `wifi_tx_transmit()`: builds and sends beacon via `esp_wifi_80211_tx()`
- `wifi_tx_transmit_nan()`: builds and sends NAN action frame
- **OK.** Duplicates `g_uas_data` population — could be refactored into a common function.

### `ble_tx.c` — BLE transmission (183 lines)
- BLE 4.0 legacy + BLE 5.0 LR Coded PHY
- ❌ **BLE 5.0 LR requires specific hardware** (ESP32-S3/C3 for Coded PHY). Marked Beta in UI.
- ❌ **`ble_tx_transmit_legacy()` uses `ADV_TYPE_NONCONN_IND`** — may not be detected by all receivers. Consider `ADV_SCAN_IND`.

### `protocol_detect.c` — UART protocol auto-detect (75 lines)
- Reads first bytes from serial: `$M<`=MSP, `$G/$N`=NMEA, `0xFE/0xFD`=MAVLink
- **OK.** Works.

### `nmea_parser.c` — NMEA parsing (136 lines)
- GGA + RMC. Extracts lat/lon/alt/speed/heading/fix/sat
- ❌ **Does not populate `altitude_baro`**

### `msp_parser.c` — MSP parsing (126 lines)
- MSP_RAW_GPS (106), MSP_ATTITUDE (108), MSP_STATUS (101)
- ❌ **Does not populate `altitude_baro`**

### `mavlink_parser.c` — MAVLink parsing (180 lines)
- GLOBAL_POSITION_INT, GPS_RAW_INT, VFR_HUD, ATTITUDE, HEARTBEAT
- **AHRS2** — FIXED: support for ArduPilot AHRS2 message (secondary heading)
- **OPEN_DRONE_ID_LOCATION** — FIXED: parsing and GPS data population (lat/lon/alt/vel/heading + altitude_baro)
- **OPEN_DRONE_ID_BASIC_ID** — FIXED: extracts UAS ID, id_type, ua_type in identity
- **OPEN_DRONE_ID_OPERATOR_ID** — FIXED: extracts operator_id in identity
- **OPEN_DRONE_ID_SYSTEM** — FIXED: extracts operator lat/lon for GPS fallback
- **`altitude_baro`** — FIXED: populated from OPEN_DRONE_ID_LOCATION.altitude_barometric
- **`mavlink_parser_get_identity()`** — new function to get identity from ODID messages
- Dialect: changed from `common/mavlink.h` to `ardupilotmega/mavlink.h` (includes common + ArduPilot messages)
- Supports 5s timeout on GPS, 10s on identity

### `mav2odid.c` — Intel MAVLink-to-ODID conversion (636 lines)
- FIXED: now COMPILED and ACTIVE: `mavlink_parser.c` includes `mav2odid.h`
- Include path changed to `ardupilotmega/mavlink.h` for consistency

### `opendroneid.c` — Intel ODID encode/decode library (1477 lines)
- **OK.** Stable, tested.

### `web_config.c` — HTTP server + REST API + OTA + logs (463 lines)
- Serves `config.html`, API `/api/config`, `/api/status`, `/api/command`, OTA
- **Lock level** — FIXED: basic check implemented (blocks destructive commands at level ≥ 1, OTA at level ≥ 2)
- ❌ **Lock level 2 eFuse burn** not implemented: level 2 is handled as "level 1 + OTA blocked", but no eFuse is burned. Lock is reversible via factory reset.
- ❌ **`handle_ota`** does not validate checksum/authentication of uploaded firmware. Any file is accepted.

### `nvs_storage.c` — NVS persistence (174 lines)
- Saves/loads `rid_config_t` in NVS namespace `esp_rid`
- **OK.**

### `led_status.c` — RGB LED (76 lines)
- 3 configurable GPIOs via Kconfig. Default: -1 (disabled)
- Red = no fix, Green = fix OK, 80ms blink on TX
- **OK.** Functional.

### `CMakeLists.txt` (component)
- Lists all .c files included
- ❌ Includes `mav2odid.c` which is never called (see above)

### `Kconfig.projbuild` — Menuconfig for LEDs
- **OK.** Added.

---

## `webui/`

### `config.html` — Complete Web UI (~1520 lines)
- Dark/light theme, dashboard, forms, logs, OTA, terminal, CLI
- **Animated splash** — Mac-style stepped bar (0→18%→42%→70%→100%), 72px logo, 6-step cycling loading text, ~3s duration
- **Responsive** — off-canvas sidebar on ≤800px, swipe right/left, 44px min touch targets, 3 breakpoints (800/600/400px)
- **WiFi NAN** — FIXED: checkbox enabled, bar chart, counters, purple styling
- **Guide descriptions** — improved across all tabs (Identity, Transmission, AP, FC, Hardware, System, Security, Compliance, Console, Dashboard)
- **Sidebar icons** — updated: Identity 🧑, FC 🛰️, Hardware 🔧, System ⚙️, Sensors 🔍 (unique, no duplicates)
- **Bug fixes** — `setAllTx()` calls `onTxModeChange()` once (was 4x), removed `<div class="clear-both">`, Restart with `.catch()`, `#log-pane-grid` collapses to 1fr 1fr at 600px
- ❌ **`public_keys` UI exists but lock level signature verification not implemented**
- ❌ **UI is inline (JS + CSS inside HTML)** — ~1520 lines, hard to maintain. Optional refactor to separate html/css/js.

---

## `mavlink/` — MAVLink v2 C dialect libraries

### Include chain (verified):
```
mavlink_parser.c  →  "ardupilotmega/mavlink.h"  →  ardupilotmega/ardupilotmega.h
                                                         ├── common/common.h
                                                         │     ├── protocol.h
                                                         │     ├── mavlink_types.h
                                                         │     ├── minimal/minimal.h
                                                         │     └── mavlink_get_info.h
                                                         ├── uAvionix/uAvionix.h
                                                         └── icarous/icarous.h
```

- **`mavlink/ardupilotmega/`**: ✅ USED (main dialect now: `#include "ardupilotmega/mavlink.h"`)
- **`mavlink/common/`**: ✅ USED (included by `ardupilotmega/ardupilotmega.h:1049`)
- **`mavlink/minimal/`**: ✅ USED (included by `common/common.h:2695`)
- **`mavlink/protocol.h`**: ✅ USED (by all dialects)
- **`mavlink/mavlink_types.h`**: ✅ USED (by `protocol.h`)
- **`mavlink/mavlink_get_info.h`**: ✅ USED (by `common/common.h`)
- **`mavlink/message_definitions/common.xml`**: ✅ USED
- **`mavlink/uAvionix/`**: 🔶 INCLUDED TRANSITIVELY (by `ardupilotmega.h:1050`) but no uAvionix messages parsed
- **`mavlink/icarous/`**: 🔶 INCLUDED TRANSITIVELY (by `ardupilotmega.h:1051`) but no icarous messages parsed

The following are still **UNUSED**:

- **`mavlink/standard/`**: ❌ (only `development/` includes it)
- **`mavlink/ASLUAV/`**: ❌
- **`mavlink/matrixpilot/`**: ❌
- **`mavlink/development/`**: ❌
- **`mavlink/test/`**: ❌
- **`mavlink/mavlink_helpers.h`**: ❌ (MAVLink signing only, not implemented)
- **`mavlink/mavlink_sha256.h`**: ❌ (only included by `mavlink_helpers.h`)
- **`mavlink/mavlink_conversions.h`**: ❌ (only included by `mavlink_helpers.h`)
- **All `testsuite.h`**: ❌
- **`mavlink/message_definitions/`**: 8 out of 11 XMLs unused

> `uAvionix/` and `icarous/` are now included but no code uses their specific messages — harmless headers.

---

## `README.md` — Main repo documentation
- CI badge, version, supported platforms
- Quick start, feature table, wiring guide, build instructions
- ❌ **Says "Hardware testing pending"** in several places. Should be updated after testing.
- ❌ **"Features" section** lists lock level as "code complete, untested" — partially outdated (basic lock level is testable)
- ❌ **Mentions "no extra GPS"** but firmware actually requires an FC with GPS

---

## `.gitignore` — Git exclusion rules (140 lines)
- Covers build/, sdkconfig, __pycache__, .vscode/*.json, *.bin (with exceptions)
- ❌ `.vscode/*.json` is excluded but `!.vscode/settings.json` re-includes it — OK, intended behavior
- **OK.** Well structured.

## `.gitattributes` — EOL normalization (40 lines)
- LF for C/py/js/json/md, CRLF for bat/ps1
- **OK.**

---

## `docs/` — GitHub Pages site

### `index.html` — Landing page + WebSerial installer (1979 lines)
- WebSerial flasher with `esp-web-tools`, built-in wiki, step-by-step guide
- Native dark/light mode, responsive
- Sections: FC compatibility, build matrix, wiring guide, flashing, FAQ
- ❌ **Inline CSS/JS** — 1979 lines all in one file. Optional refactor to separate html/css/js.
- 🟡 **"DEV build" status** — should be updated after stable release

### `config(demo).html` — Standalone demo UI (~2546 lines)
- Offline copy of `config.html` for demonstration without hardware
- Dark/light theme, all fields, full telemetry simulation
- **Simulation engine** — `generateDemoStatus()` produces realistic status every 2s: circular GPS patrol (Rome Colosseum, 200m radius), ±15m altitude drift, ±1.2 m/s speed variation, progressive battery drain (12.2V→10.8V), per-protocol TX counters, RSSI/temperature/CPU/HEAP noise, all fields populated
- **Dynamic logs** — `getNextDemoLogs()` generates 1-2 new entries every 4s from 12 templates with real-time GPS/alt/heading values
- **OTA progress** — simulated progress bar with random 5-20% increments every 400ms
- **Fetch interceptor** — covers `/api/config`, `/api/status`, `/api/logs`, `/api/command`, `/api/reset`, `/ota` with realistic responses
- **Log format** — fixed to `{l, t, m}` (was `{ts, msg, level}`, incompatible with `renderTermTo`)
- **Config fixes** — `wifi_bcn_rate_hz: 1` (was 100), `tx_modes: 13` (was 7), `ua_type: 2` (was 1), added missing fields (`fw_version`, `build_date`, `cpu_load`, `battery`, `dop`, etc.)
- ❌ **Not automatically synchronized** with `config.html` — every change must be manually ported

### `manifest.json` — WebSerial firmware manifest (33 lines)
- 3 chipFamily (ESP32, ESP32-S3, ESP32-C3) with bootloader and partition offsets
- ❌ **Missing esp32s2, esp32c2, esp32c6** — CI generates dynamic manifest but this static file in docs/ is outdated
- ❌ **Hardcoded version** "0.1.0-dev" — not updated

### `images/` — Graphic assets (5 files)
- `logo.svg`, `logo con scritta.svg`, `ardupilot_logo.webp`, `betaflight_logo.svg`, `inav_logo.png`
- **OK.**

---

## `.github/`

### `workflows/build.yml` — CI build + Pages deploy (193 lines)
- Matrix 6 targets (esp32, esp32s2, esp32s3, esp32c2, esp32c3, esp32c6) ✅
- Partition/flash/BLE conditionals per target ✅
- ccache for faster builds ✅
- download-artifact with `firmware-*` pattern ✅
- Dynamically generated `manifest.json` with Python ✅
- GitHub Pages deploy on push to main ✅
- **OK.** Functional.

### `workflows/release.yml` — Release on tag v* (308 lines)
- Matrix 6 targets, aligned with `build.yml` ✅
- Generates zip per target + manifest.json ✅
- Release notes with auto-changelog (`git log`) ✅
- GitHub Pages deploy also on release ✅
- **OK.** Functional.

### `dependabot.yml` — Automatic updates
- `package-ecosystem: "github-actions"`, weekly schedule
- **OK.**

### `PULL_REQUEST_TEMPLATE.md` — PR template
- Checklist: coding style, hardware test, docs, build, warnings
- **OK.**

### `ISSUE_TEMPLATE/bug_report.md` — Bug report template
- Fields: hardware, firmware, config, console output
- **OK.**

### `ISSUE_TEMPLATE/feature_request.md` — Feature request template
- Fields: description, proposed solution, affected protocols
- **OK.**

---

## `.vscode/`

### `c_cpp_properties.json` — C/C++ IntelliSense
- Compiler path: local `xtensa-esp32-elf-gcc.exe`
- `compileCommands` from `build/compile_commands.json`
- ❌ **Windows absolute paths** — not portable to other systems

### `launch.json` — Debug config
- Only "Eclipse CDT GDB Adapter" attach
- ❌ **Minimal configuration** — missing real debug profiles (e.g. OpenOCD + gdb)

### `settings.json` — VSCode workspace settings
- Clangd config + IDF extension config
- `"idf.currentSetup": "C:\\esp\\v6.0.1\\esp-idf"`
- ❌ **Windows absolute paths** — not portable

---

## Priority Summary

| Priority | Item | Location | Status |
|----------|------|----------|--------|
| 🔴 HIGH | `altitude_baro` populated by MAVLink parser (missing NMEA/MSP) | `nmea_parser.c`, `msp_parser.c` | 🟡 Partial (MAVLink ODID only) |
| 🔴 HIGH | Redundant `force_tx` logic (lines 284-286) | `esp_remote_id.c` | ❌ To fix |
| 🟡 MEDIUM | Lock level 2: eFuse burn not implemented | `web_config.c` | ❌ To do |
| 🟡 MEDIUM | `sdkconfig` tracked in git (generated) | Remove from tracking | ❌ To do |
| 🟡 MEDIUM | `docs/manifest.json` hardcoded (missing targets) | `docs/manifest.json` | ❌ To do |
| 🟡 MEDIUM | `README.md` says "hardware testing pending" | `README.md` | ❌ To do |
| 🟡 MEDIUM | `docs/config(demo).html` not synced with `config.html` | Manual maintenance | ❌ To do |
| 🟡 MEDIUM | `idf_component.yml` wrong description ("Scanner") | `idf_component.yml` | ❌ To do |
| 🟢 LOW | BLE 4.0 uses `ADV_TYPE_NONCONN_IND` — compatibility testing | `ble_tx.c` | ❌ To test |
| 🟢 LOW | OTA does not validate checksum/authentication | `web_config.c` | ❌ To do |
| 🟢 LOW | Duplicate `g_uas_data` population in `wifi_tx.c` | Optional refactor | ❌ Optional |
| 🟢 LOW | `.vscode/` Windows absolute paths | `.vscode/*.json` | ❌ Optional |
| 🟢 LOW | `config.html` inline — optional refactor | `config.html` | ❌ Optional |
| 🟢 LOW | `docs/index.html` inline (1979 lines) | Optional refactor | ❌ Optional |
| ✅ RESOLVED | `mav2odid.c` integrated into `mavlink_parser.c` | `mavlink_parser.c` + `mav2odid.h` | ✅ Done |
| ✅ RESOLVED | `ardupilotmega/` now main MAVLink dialect | `mavlink_parser.c` | ✅ Done |
| ✅ RESOLVED | `AHRS2` secondary heading from ArduPilot | `mavlink_parser.c` | ✅ Done |
| ✅ RESOLVED | Identity from MAVLink ODID overrides config | `esp_remote_id.c` | ✅ Done |
| ✅ RESOLVED | `altitude_baro` populated by MAVLink ODID Location | `mavlink_parser.c` | ✅ Done |
