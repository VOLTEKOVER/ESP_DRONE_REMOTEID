<p align="center">
  <img src="docs/images/logo%20con%20scritta.svg" alt="ESP DRONE REMOTEID" width="420">
</p>

<p align="center">
  <a href="https://github.com/VOLTEKOVER/ESP_DRONE_REMOTEID/actions/workflows/build.yml"><img src="https://img.shields.io/github/actions/workflow/status/VOLTEKOVER/ESP_DRONE_REMOTEID/build.yml?logo=github&label=build" alt="Build"></a>
  <a href="https://VOLTEKOVER.github.io/ESP_DRONE_REMOTEID/"><img src="https://img.shields.io/badge/BETA-000?logo=esphome&color=f9a825" alt="BETA"></a>
  <a href="https://github.com/VOLTEKOVER/ESP_DRONE_REMOTEID/releases"><img src="https://img.shields.io/github/v/release/VOLTEKOVER/ESP_DRONE_REMOTEID?include_prereleases&logo=github&label=version" alt="Release"></a>
  <a href="https://www.espressif.com/"><img src="https://img.shields.io/badge/ESP32%20|%20S3%20|%20C6-000?logo=espressif" alt="Platform"></a>
  <a href="LICENSE"><img src="https://img.shields.io/github/license/VOLTEKOVER/ESP_DRONE_REMOTEID?color=blue" alt="License"></a>
</p>

<p align="center">
  <img src="docs/images/ardupilot_logo.webp" height="28" alt="ArduPilot">&nbsp;&nbsp;
  <img src="docs/images/betaflight_logo.svg" height="28" alt="Betaflight">&nbsp;&nbsp;
  <img src="docs/images/inav_logo.png" height="28" alt="INAV">
</p>

<p align="center">
  <b>ASTM F3411-22a</b> Open DroneID transmitter.<br>
  Parses MAVLink · MSP · NMEA · broadcasts via <b>WiFi Beacon + NAN + BLE 4.0/5.0</b>.<br>
  Targets: ESP32, ESP32-S3, ESP32-C6.<br>
  See the <a href="https://VOLTEKOVER.github.io/ESP_DRONE_REMOTEID/">online wiki</a> for wiring, flashing, and configuration.
</p>

<p align="center">
  <a href="https://VOLTEKOVER.github.io/ESP_DRONE_REMOTEID/"><b>Wiki & Demo</b></a>
</p>

---

## Quick Start

| Step | Action |
|------|--------|
| 1 | Connect ESP32 to USB |
| 2 | Open [VOLTEKOVER.github.io/ESP_DRONE_REMOTEID](https://VOLTEKOVER.github.io/ESP_DRONE_REMOTEID/) |
| 3 | Select your chip (**ESP32** / **ESP32-S3** / **ESP32-C6**) |
| 4 | Click **Install**, pick serial port |
| 5 | Connect to WiFi **ESP-RID** |
| 6 | Configure at `http://192.168.4.1` |

> <a href="https://VOLTEKOVER.github.io/ESP_DRONE_REMOTEID/config(demo).html">Try the live demo</a> — full web UI simulation without hardware.

## Features

| | |
|---|---|
| **Protocols** | MAVLink (incl. ArduPilot ODID), MSP, NMEA — auto-detect |
| **Radio** | WiFi Beacon + WiFi NAN + BLE 4.0 + BLE 5.0 Coded PHY |
| **GPS source** | Flight controller (MAVLink/MSP/NMEA) or direct GPS module |
| **Web UI** | Built-in AP + REST API + live telemetry |
| **OTA updates** | Upload firmware over WiFi with SHA-256 verification |
| **Config** | 26+ parameters: UAS ID, rates, power, public keys, lock levels |
| **Security** | 3 lock levels (normal / signed / eFuse permanent) |
| **Demo** | [Offline demo page](https://VOLTEKOVER.github.io/ESP_DRONE_REMOTEID/config(demo).html) |

## Protocolli

| Categoria | Protocollo | Direzione | Dettaglio |
|---|---|---|---|
| **Input** (posizione) | **MAVLink v2** | FC → ESP | ArduPilot/PX4: `GPS_RAW_INT`, `GLOBAL_POSITION_INT`, `HEARTBEAT`, `OPEN_DRONE_ID_*` |
| | **MSP** | FC → ESP | Betaflight/iNAV: `MSP_RAW_GPS` (106), `MSP_ATTITUDE` (108), `MSP_STATUS` (101) |
| | **NMEA 0183** | GPS → ESP | `$GPGGA`, `$GNGGA`, `$GPRMC`, `$GNRMC`, `$GPVTG`, `$GNVTG` |
| **Output** (broadcast RID) | **WiFi Beacon** (802.11 mgmt) | ESP → Aria | ASTM F3411-22a su WiFi Beacon |
| | **WiFi NAN** | ESP → Aria | Service Discovery Frame NAN |
| | **BLE 4.0 Legacy** | ESP → Aria | Bluetooth 4.0 advertising, non-connectable |
| | **BLE 5.0 Coded PHY** (LR) | ESP → Aria | Bluetooth 5.0 Long Range, `PHY_CODED` (S3/C6) |
| **Config/controllo** | **HTTP / Web UI** | Browser ↔ ESP | AP-mode `192.168.4.1`, REST API, telemetria live |
| | **CLI seriale** (UART0) | Terminale ↔ ESP | 14 comandi: `help`, `status`, `config`, `patrol`, `transmit`, ... |
| | **NVS storage** | ESP → Flash | Salvataggio persistente configurazione |
| **Futuro** | **ESP-NOW mesh** | Drone ↔ Drone | Relay RID tra droni (da fare) |
| | **LoRa SX1262** | ESP → Aria | Backup RID 10+ km (da fare) |
| | **DroneCAN / CAN bus** | FC → ESP | Lettura bus CAN drone (da fare) |

## Hardware

| ESP32 Pin | Connect to |
|-----------|-----------|
| `GPIO16` (UART2 RX) | FC TX (output) or GPS TX |
| `GND` | FC / GPS GND |
| `5V` (VIN) | FC BEC or USB |

> **&#x26a0;** NMEA clone (tap GPS TX): add **1 k&Omega; series resistor** on the tap line.

### Recommended board: Seeed XIAO ESP32-C6

Smallest & most innovative option:
- **ESP32-C6**: WiFi 6 (OFDMA), BT 5.3, 802.15.4
- **21 &times; 17.8 mm**, USB-C, LiPo charger
- **U.FL** + onboard ceramic antenna (selezionabile via SW)
- Stackabile con modulo **L76K GNSS** (multi-costellazione)
- Costo totale: **~15$**, zero saldature

> Vedi [`docs/prototype_bom.md`](docs/prototype_bom.md) per BOM completo.

## Build

Push a `main` &rarr; build automatico per ESP32, ESP32-S3, ESP32-C6 via GitHub Actions.  
[Ultimi builds](https://github.com/VOLTEKOVER/ESP_DRONE_REMOTEID/actions/workflows/build.yml)

### Locale (ESP-IDF v6.0.1)

```bash
git clone https://github.com/VOLTEKOVER/ESP_DRONE_REMOTEID.git
cd ESP_DRONE_REMOTEID
idf.py set-target esp32       # o esp32s3 / esp32c6
idf.py build
```

## Project Structure

```
ESP_DRONE_REMOTEID/
├── ESP32_DRONE_REMOTE_ID_Firmware/  # ESP-IDF project
│   ├── main/                        # app_main entry point
│   ├── components/esp_remote_id/    # Core: protocol detect, web UI, NVS config
│   │   ├── src/                     # Source files
│   │   ├── include/                 # Headers
│   │   ├── webui/config.html        # Embedded web interface
│   │   └── mavlink/                 # MAVLink v2 dialect headers
│   └── partitions*.csv              # OTA partition layouts
├── ESP_DRONE_REMOTEID_Analyzer/     # Python analyzer (standalone)
├── docs/                            # GitHub Pages site
│   ├── index.html                   # WebSerial flasher + wiki
│   ├── config(demo).html            # Offline config UI demo
│   ├── innovation_diy.md            # Feature comparison + innovation ideas
│   ├── prototype_bom.md             # Recommended hardware BOM
│   ├── manifest.json                # Firmware manifest (auto-generated by CI)
│   └── images/                      # Logo assets
├── .github/workflows/               # CI/CD (3-target matrix + release + Pages)
└── todolist/                        # Software status inventory
```

## License

Derived from [Intel Open Drone ID](https://github.com/opendroneid) (Apache 2.0).  
See [LICENSE](LICENSE) for full terms.
