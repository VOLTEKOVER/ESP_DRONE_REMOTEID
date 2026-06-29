# Innovation / DIY — ESP32 Remote ID vs Dronetag

## Feature Comparison

### vs Mini 4G (with LTE)

| Feature | ESP32 DIY | Dronetag Mini 4G |
|---|---|---|
| Broadcast RID (DRI) | WiFi Beacon + WiFi NAN + BLE 4 + BLE 5 | BLE only |
| Network RID (NRI) | — | LTE Cat 1 bis |
| Standard | ASTM F3411 / prEN 4709-002 | ASTM F3411 / prEN 4709-002 |
| FC Protocols | MAVLink + MSP + NMEA (auto) | Not specified |
| Config | Web UI + Serial CLI | Mobile app |
| OTA | Web UI + SHA-256 verify | Mobile app |
| Security | ECDSA sign + eFuse lock | Not documented |
| LED | RGB PWM state machine (7 states) | Status LEDs |
| HW Cost | < 5€ (ESP32) | ~350€ |
| Open source | YES | NO |
| Subscription | NO | YES (LTE data plan) |

### vs DRI (OEM integrator module)

| Feature | ESP32 DIY | Dronetag DRI |
|---|---|---|
| Broadcast RID (DRI) | WiFi Beacon + WiFi NAN + BLE 4 + BLE 5 | BLE 4 + BLE 5 LR only |
| BLE Power | 18 dBm max (configurable) | 8 dBm |
| RF range claimed | — | 3 km (BLE) |
| Position data | MAVLink / MSP / NMEA (auto) | MAVLink 2 only |
| Power supply | 3.3V / 5V via USB or pin | 3.3 – 17V (internal buck regulator) |
| Avg consumption | ~80-150 mA (ESP32 + GPS) | **3 mA** |
| Max consumption | ~500 mA | 10 mA |
| UART forwarding | No (occupies one UART) | Yes (integrated bypass) |
| Dimensions | Depends on board | 22.5 × 16 × 5 mm |
| Weight | Depends on board | 0.5 – 1.5 g |
| Operating temp | Typical 0-85°C | -40°C to +85°C |
| Serial number | Generated from MAC | ANSI/CTA-2063-A pre-programmed |
| Certifications | — | FCC/CE (approved radio module) |
| Config | Web UI + CLI (no app) | Dronetag mobile app |
| Open source | YES | NO |
| Cost | < 5€ (ESP32) | ~50-100€ (estimated) |

The **DRI is the most direct competitor** — same use case (FC → RID), but with huge advantages in consumption (3mA vs 80+), form factor, and certifications. We beat them on multi-link WiFi+BLE, multi-protocol, OTA, security, and cost.

### vs BS gen.2 (budget standalone, our direct competitor)

| Feature | ESP32 DIY | Dronetag BS gen.2 |
|---|---|---|
| Broadcast RID | WiFi Beacon + WiFi NAN + BLE 4 + BLE 5 | BLE 4 Legacy + BLE 5 LR only |
| Range claimed | — | 3 km (BLE) |
| BLE Power | 18 dBm max (configurable) | 8 dBm |
| GNSS | **External via UART** (single constellation) | **Integrated u-blox M10** multi-constellation |
| Power supply | 3.3V / 5V (USB or pin) | 3.3 – 17V with buck regulator |
| Avg consumption | ~80-150 mA + external GPS | **15 mA** |
| Max consumption | ~500 mA | **50 mA** |
| Dimensions | Depends on board + GPS | **18 × 14 × 5 mm** |
| Weight | Depends | **1.5g** (with heat shrink) / **3g** (with enclosure) |
| Operating temp | Typical 0-85°C | **-40°C to +85°C** |
| Config | Web UI + CLI (no app) | Dronetag mobile app only |
| OTA | Web UI with SHA-256 | Mobile app |
| Antennas | WiFi/BLE printed + external GPS | 2× U.FL (GNSS + BLE, external) |
| GNSS timekeeping | None | **Supercapacitor** — maintains fix ~8 min during power cycle |
| Spektrum XBUS / SRXL2 | No | **Yes** (direct telemetry on Spektrum radios) |
| Betaflight GNSS input | No | **Yes** — BS acts as GPS for Betaflight |
| Flight data logging | No | In development (internal flash) |
| Telemetry module (Futaba/Spektrum) | No | In development |
| SAW filters | No | **Yes** (GNSS interference immunity) |
| Target user | Technical / developer | Experienced pilots (soldering required) |
| Open source | **YES** | NO |
| Cost | < 5€ (ESP32) + 10€ GPS | ~**99€** |

**BS gen.2 is the trickiest competitor** because:
- **Same use case**: standalone, no FC, GNSS + BLE
- **Unbeatable size and weight**: 18mm, 1.5-3g
- **15mA consumption** — flies for hours on a small battery
- **3.3-17V input range** with integrated regulator
- **Supercapacitor** — GNSS fix persists between power cycles (we don't)
- **Dual GNSS + RID function**: BS feeds GPS data to Betaflight via XBUS (UBLOX protocol). A single device serves as both GPS receiver for flight and RID transmitter. We only have GPS for RID, we don't forward it to FC.
- **4-pin JST SH connector**: VCC (3.3-17V), RX, TX, GND — same connector for power, Betaflight, and Spektrum XBUS
- **GNSS supercapacitor**: maintains fix ~8 min during battery swap (reduced TTFF). We reacquire from scratch.
- **Spektrum XBUS/SRXL2**: native telemetry on Spektrum RC radios (future: Futaba and others)

What we have that BS doesn't:
- **WiFi multi-link** (WiFi Beacon + NAN, in addition to BLE)
- **Open source**
- **Web UI + CLI** (no app needed)
- **Multi-FC protocol** (MAVLink, MSP, NMEA)
- **OTA via web** with SHA-256 verification
- **ECDSA signature + eFuse lock security**
- **Extensibility** (we can add mesh, LoRa, SD, CAN — BS is closed)

### vs Beacon gen.2 (BLE only, with battery)

| Feature | ESP32 DIY | Dronetag Beacon gen.2 |
|---|---|---|
| Broadcast RID (DRI) | WiFi Beacon + WiFi NAN + BLE 4 + BLE 5 | BLE only |
| GNSS | 1 constellation (via UART GPS) | GPS L1 + GLONASS + Galileo + BeiDou + SBAS + QZSS |
| Extra sensors | — | Barometer, accelerometer |
| BLE Power | Configurable 18 dBm max | 8 dBm |
| Avg consumption | ~80-150 mA (ESP32 + GPS) | 15 mA |
| Max consumption | ~500 mA | 100 mA |
| Battery | No (external power) | Li-Po 200 mAh, 10-18h |
| Dimensions | Depends on board | 37 x 26 x 16 mm |
| Weight | Depends on board | 17 g |
| IP rating | — | IP43 |
| Compliance | — | FAA accepted / EASA listed |
| HW Cost | < 5€ (ESP32) | ~200€ |
| Open source | YES | NO |

## Paper Advantages (against all)

- **Simultaneous multi-link**: WiFi beacon + NAN + BLE legacy + BLE LR — Dronetag transmits only BLE
- **No subscription**: WiFi/BLE are free, no SIM card
- **Transparent**: open code, modifiable, extensible
- **Web UI**: configure from browser, no app
- **Multi-FC**: works with ArduPilot, PX4, Betaflight, iNAV, native GPS
- **HW Security**: ECDSA signature + permanent eFuse burn
- **Total configurability**: power, channel, rate, GPIO — everything adjustable
- **Higher BLE power** (18 vs 8 dBm)
- **No barometric pressure dependency** — DRI requires SCALED_PRESSURE from FC to compose messages, we work with GPS + MAVLink only
- **Extra protocols** — DRI only supports MAVLink 2; we also support MSP and NMEA (direct GPS)
- **OTA updates via web** — DRI has no OTA, updates only via mobile app

## DRI — Deep Analysis from Integration Guide

### MAVLink messages DRI requires from flight controller
- ALTITUDE (pressure)
- SCALED_PRESSURE (barometric data)
- GPS_RAW_INT (GNSS position)
- SYSTEM_TIME (time reference)
- GLOBAL_POSITION_INT (global position)
- HEARTBEAT (system status)
- OPEN_DRONE_ID_BASIC_ID (overrides DRI serial)
- OPEN_DRONE_ID_LOCATION
- OPEN_DRONE_ID_SYSTEM
- OPEN_DRONE_ID_OPERATOR_ID
- OPEN_DRONE_ID_ARM_STATUS (locks takeoff if RID fails)

### Certifications — Dronetag's real moat

| Aspect | Dronetag DRI | Our ESP32 |
|---|---|---|
| **FAA Standard RID** | Compliance matrix + DoC ready | Would need to do everything from scratch |
| **FCC** | Uses u-blox ANNA-B4 certified module (FCC ID: XPYANNAB4) → "Contains FCC ID" | ESP32 has FCC ID from Espressif, but custom board requires new certification |
| **EU C-class** | Guides for Module A/B+C/H with Notified Body | No defined path |
| **ANSI/CTA-2063-A SN** | Pre-programmed, ICAO prefix managed | MAC address only |
| **Pre-approved antennas** | Official list of certified antennas | None |
| **Lab testing** | ASTM F3586-22 from accredited lab | Never done |
| **Estimated certification cost** | Already incurred (included in ~50-100€ price) | Thousands of € + long lead times |

**Takeaway**: certification is their hardest-to-beat advantage. For a DIY prototype it's not needed, but for a real product it's a significant barrier.

### DRI Strengths (to close the gap)

| Area | DRI | Our ESP32 | Gap |
|---|---|---|---|
| Consumption | 3 mA avg, 10 mA max | ~80-150 mA | **20-50x** |
| Power supply | 3.3-17V with buck regulator | 3.3V/5V via USB | Needs external regulator |
| Operating temp | -40°C to +85°C | Typical 0-85°C | Lower coverage |
| Certifications | FCC/CE pre-certified | None | Expensive to obtain |
| SN | ANSI/CTA-2063-A pre-programmed | Generated from MAC | Minor, can be added |
| UART forwarding | Yes (integrated bypass) | No | Occupies one UART |
| Arming failsafe | Locks takeoff if RID missing | No failsafe logic | To be implemented |
| Weight | 0.5-1.5g | Variable | Significant gap |
| Dimensions | 22.5×16×5 mm | Variable | Significant gap |

### What THEY CANNOT do (and we can)

- **Transmit over WiFi** — DRI is BLE only. WiFi has longer range and is receivable by more devices (phones, laptops, dedicated stations)
- **Support non-MAVLink protocols** — MSP (Betaflight/iNAV) and NMEA (direct GPS) are not supported
- **OTA firmware updates via web** — updates require mobile app + BLE
- **App-free configuration** — we have serial CLI + web UI, they only have mobile app
- **Mesh relay** — impossible via BLE, would require additional HW
- **WiFi telemetry bridge** — DRI uses only UART, no wireless telemetry output
- **Logging and debug** — no text or web interface for diagnostics
- **Full customization** — closed firmware, parameters limited to what the app exposes

## Innovative Ideas for DIY Prototype

### Goal
> An RID transmitter that works **without LTE, without SIM, without subscription**, surpassing Dronetag in features, flexibility, and cost, while remaining **relevant** for real-world applications.

### 1. ESP-NOW Mesh RID Relay ★★★★★
- Drones in flight relay RID messages between each other via ESP-NOW
- Drones beyond line of sight become visible if another drone has encountered them
- No commercial product offers mesh RID
- Complexity: medium — ESP-NOW is native on ESP32

### 2. Dual-band 2.4 + 5 GHz ★★★★☆
- ESP32-C5/S3 supports 5 GHz
- RID on 5 GHz is less congested, more reliable in flight
- No commercial RID uses 5 GHz today

### 3. WiFi Telemetry Bridge ★★★★☆
- Same WiFi interface used for RID also serves as telemetry hotspot
- Smartphone connects directly to the drone (no SIM, no cloud)
- Bidirectional MAVLink via UDP/WebSocket

### 4. LoRa Long-range Backup ★★★★☆
- LoRa module (SX1262) for 10+ km RID
- Dual transmission: WiFi/BLE for short range + LoRa for long range
- Lost drone? Last position on LoRa receivable from ground with cheap transceiver

### 5. SD Card Logging ★★★☆☆
- Full flight log on microSD: position, status, transmissions
- Offline geofence (no-fly zones loaded from SD)
- Dronetag has no SD

### 6. Ed25519 Authentication (F3411-22a) ★★★★★
- Sign all 4 mandatory ODID messages with Ed25519 per broadcast cycle
- 4 auth pages distributed alongside base messages
- PKI hierarchy: CA root → device certificate → per-device key
- NVS private key provisioning + optional compile-time embedded key
- Already implemented in peinser/esp-remoteid — port to this codebase
- No extra HW required (ESP32 has SHA256 + RNG)

### 7. MAVLink Bidirectional Status ★★★★★
- Transmit HEARTBEAT (component MAV_TYPE_ODID) back to flight controller
- Transmit OPEN_DRONE_ID_ARM_STATUS (GOOD_TO_ARM / PRE_ARM_FAIL)
- Enables ArduPilot DID pre-arm gating: no RID → no takeoff
- Operator-location freshness loop: re-stamp fixed operator position autonomously
- Requires TX UART GPIO (already present in config)
- Already implemented in peinser/esp-remoteid

### 8. OTA Update Server ★★★★☆
- Wi-Fi AP mode on trigger (GPIO button or always-on)
- HTTP server with endpoints: /status, /update, /nvs, /rollback, /factory-reset
- OTA dual-slot partition management (already have ota_0/ota_1 layout)
- SHA-256 firmware validation (already mandatory in web_config.c)
- AP MAC address randomization for privacy
- Already implemented in peinser/esp-remoteid

### 9. DroneCAN Input ★★★☆☆
- TWAI/CAN controller receive DroneCAN frames
- Decode uavcan.equipment.gnss.Fix2 for position
- Decode custom com.peinser.remoteid.Identity for UAS/Operator ID
- CAN transceiver required (~3€ external module)
- Already implemented in peinser/esp-remoteid

### 10. WS2812 RGB LED (RMT) ★★★☆☆
- Replace or augment LEDC PWM LED driver with addressable RGB
- Single GPIO drives RGB LED with RMT waveform
- Simplifies wiring (3 GPIOs → 1 GPIO)
- Enables addressable patterns not possible with PWM
- Already implemented in peinser/esp-remoteid

### 11. External GPIO Lighting Outputs ★★★☆☆
- Up to 5 independently configurable trigger outputs
- Patterns: solid, beacon 1Hz short/50%, single/double/triple strobe, fast strobe
- Separate from onboard RGB — transistor/MOSFET drivers for aviation lights
- Already implemented in peinser/esp-remoteid

### 12. Flash Encryption (eFuse) ★★★★☆
- AES-256 flash encryption on first boot
- Development mode (reversible) → Release mode (permanent)
- Protects NVS private key and firmware at rest
- OTA update still works in encrypted mode
- Already implemented in peinser/esp-remoteid

### 13. Cloudbuild Web Configurator ★★★☆☆
- Browser form → sdkconfig overlay → GitHub Actions dispatch → ready-to-flash ZIP
- No local ESP-IDF toolchain required for users
- Uses GitHub Actions for free CI build minutes
- Already implemented in peinser/esp-remoteid

### 14. Devcontainer + Makefile Build System ★★★☆☆
- VS Code devcontainer with ESP-IDF pre-installed
- Makefile targets: build, flash, monitor, ota-flash, ota-status
- macOS serial bridge (socat) for Docker-based dev
- Already implemented in peinser/esp-remoteid

### 6. CAN Bus / DroneCAN bridge ★★★☆☆
- Direct CAN bus reading (CUAV, Zubax, etc.)
- Works without MAVLink — position, battery, motor status
- Useful for custom drones or FCs that don't expose MAVLink

### 7. ADSB Receiver ★★★☆☆
- RTL-SDR (R820T2) captures ADSB messages from aircraft
- Drone sees nearby aircraft and can avoid them
- Feature no commercial RID has

### 8. Edge ML Anti-spoofing ★★☆☆☆
- Compares GPS position with WiFi RTT, IMU, cell tower RSSI
- Detects spoofing and signs RID packets with attested key
- Complex but realistic on ESP32-S3 with ESP-NN

## Proposed Architecture

```
┌──────────────────────────────────────────────────────┐
│  ESP32 (S3 / C6)                                     │
│                                                      │
│  ┌──────────┐ ┌───────────┐ ┌───────────┐ ┌───────┐ │
│  │ RID      │ │ MAVLink   │ │ WiFi      │ │ BLE   │ │  ← done
│  │ Beacon   │ │ /MSP/NMEA │ │ Hotspot   │ │ 4+5   │ │
│  └──────────┘ └───────────┘ └───────────┘ └───────┘ │
│  ┌──────────┐ ┌───────────┐ ┌───────────┐ ┌───────┐ │
│  │ Kalman   │ │ Ed25519   │ │ MAVLink   │ │ OTA   │ │  ← new
│  │ Predict  │ │ Auth      │ │ ARM_STATUS│ │ Server│ │
│  └──────────┘ └───────────┘ └───────────┘ └───────┘ │
│  ┌──────────┐ ┌───────────┐ ┌───────────┐ ┌───────┐ │
│  │ ESP-NOW  │ │ LoRa      │ │ SD Card   │ │ Flash │ │  ← TODO
│  │ Mesh     │ │ SX1262    │ │ Logging   │ │ Crypt │ │
│  └──────────┘ └───────────┘ └───────────┘ └───────┘ │
│  ┌──────────┐ ┌───────────┐ ┌───────────┐           │
│  │ DroneCAN │ │ ADSB      │ │ Edge ML   │           │  ← nice to have
│  │ CAN bus  │ │ RTL-SDR   │ │ Anti-spoof│           │
│  └──────────┘ └───────────┘ └───────────┘           │
│  ┌──────────┐ ┌───────────┐                          │
│  │ GPIO     │ │ Cloudbuild│                          │  ← infra
│  │ Lighting │ │ UI        │                          │
│  └──────────┘ └───────────┘                          │
└──────────────────────────────────────────────────────┘
```

## Certification — What's Needed for a Real Product

If you ever want to turn the prototype into a product:

### USA (FAA + FCC)
1. **Means of Compliance (MoC)** — ASTM F3411 + ASTM F3586-22 test from accredited lab
2. **Declaration of Compliance (DoC)** — formal FAA submission
3. **FCC Certification** — EMC/RF testing from accredited lab + TCB
4. **ANSI/CTA-2063-A SN** — request ICAO prefix + unique serials
5. **"Contains FCC ID"** — if using pre-certified module (e.g., ESP32 already has FCC ID)

### EU (EASA)
1. **Conformity assessment** — Module B+C or H with Notified Body
2. **CE marking + C-class label** (C0-C6)
3. **EN 4709-002 compliance**
4. **Complete technical documentation**

### Minimum for Technical Credibility (even without certification)
- [ ] OPEN_DRONE_ID_ARM_STATUS → lock takeoff if RID fails
- [ ] GNSS failsafe → RTH if position lost
- [ ] Broadcast continuity → last valid data if GCS disconnects
- [ ] Configurable ANSI/CTA-2063-A SN
- [ ] Operator location from GCS (for FAA)
- [ ] Tamper-resistant storage for SN (eFuse already present)

## Positioning — Where to Really Beat Them

| Dronetag does | We do better |
|---|---|
| BLE only | **WiFi + BLE** — none of their products have WiFi |
| Mobile app only | **Web UI + CLI** — no app to install |
| Closed source | **Open source** — anyone can contribute |
| No documented security | **ECDSA signatures + eFuse lock** |
| Fixed firmware | **OTA via web** verified with SHA-256 |
| MAVLink only (DRI) | **MAVLink + MSP + NMEA** — more protocols |
| Finished product, period | **Extensible platform** — mesh, LoRa, SD, CAN |

**Our thesis**: an RID transmitter shouldn't be a closed product. It should be an **open platform** to build upon: mesh relay, telemetry, logging, integration with any drone ecosystem. Dronetag sells devices. We sell **flexibility**.

## Implementation Priority

| Prio | Feature | Impact | Complexity | HW Needed | Status |
|------|---------|--------|------------|-----------|--------|
| 1 | **Kalman Position Predictor** | ★★★★★ | ★★☆☆☆ | None | ✅ Just implemented |
| 2 | **MAVLink ARM_STATUS** | ★★★★★ | ★★☆☆☆ | TX GPIO (already have) | 📥 Port from peinser |
| 3 | **Ed25519 Auth (F3411-22a)** | ★★★★★ | ★★★☆☆ | None | 📥 Port from peinser |
| 4 | **ESP-NOW Mesh** | ★★★★★ | ★★★☆☆ | None | 🔜 Next after Kalman |
| 5 | **OTA Update Server** | ★★★★☆ | ★★★☆☆ | Button GPIO | 📥 Port from peinser |
| 6 | **Flash Encryption** | ★★★★☆ | ★★☆☆☆ | None (efuse) | 📥 Port from peinser |
| 7 | **WiFi Telemetry Bridge** | ★★★★☆ | ★★☆☆☆ | Already have WiFi | 🟡 Partial |
| 8 | **LoRa backup** | ★★★★☆ | ★★★☆☆ | SX1262 (~5€) | 🔜 Future |
| 9 | **DroneCAN Input** | ★★★☆☆ | ★★★☆☆ | CAN transceiver (~3€) | 📥 Port from peinser |
| 10 | **SD Card + geofence** | ★★★☆☆ | ★★☆☆☆ | SD slot | 🔜 Future |
| 11 | **GPIO Lighting Outputs** | ★★★☆☆ | ★☆☆☆☆ | MOSFET driver | 📥 Port from peinser |
| 12 | **WS2812 RGB LED (RMT)** | ★★☆☆☆ | ★☆☆☆☆ | None (1 GPIO) | 📥 Port from peinser |
| 13 | **Dual-band 5 GHz** | ★★☆☆☆ | ★★☆☆☆ | ESP32-S3/C6 | 🟡 Hardware limit |
| 14 | **Devcontainer + Makefile** | ★★☆☆☆ | ★☆☆☆☆ | None | 📥 Port from peinser |
| 15 | **Cloudbuild Web UI** | ★★☆☆☆ | ★★★★☆ | None (GitHub infra) | 📥 Port from peinser |
| 16 | **CAN Bus / ADSB / ML** | ★★☆☆☆ | ★★★★★ | Various | 🔜 Evaluate later |

## Executive Summary

> **Open Source RID Mesh** — multi-link transmitter (WiFi + BLE + LoRa) with ESP-NOW mesh relay, native WiFi telemetry, SD logging, zero subscriptions. BOM cost <20€. Open, extensible, superior.
