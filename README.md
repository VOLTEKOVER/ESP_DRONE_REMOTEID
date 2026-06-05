# ESP32 Drone ID

A modern, production-ready electronic ID system for unmanned aerial vehicles (UAVs) based on ESP32 microcontroller. This project provides remote identification capabilities compatible with international aviation regulations.

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)
![Status: Active Development](https://img.shields.io/badge/Status-Active%20Development-orange.svg)

## 🎯 Features

- **Multiple ID Standards Support**
  - OpenDroneID (EU/US compatible)
  - French WiFi UAV ID (id_france)
  - Extensible architecture for additional standards

- **Multi-Protocol Broadcasting**
  - Bluetooth Low Energy (BLE) 4.0
  - WiFi Beacon
  - WiFi Neighbor Awareness Networking (NAN)

- **Hardware Optimized**
  - Runs on affordable ESP32 dev boards
  - Dual-core processing support
  - Minimal power consumption

- **Flexible Configuration**
  - Easy ID parameter setup
  - Support for multiple transmission modes simultaneously

## 🚀 Quick Start

### Requirements
- ESP32 Development Board
- Platform.io or Arduino IDE
- OpenDroneID core library (for id_open module)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/VOLTEKOVER/ESP32_DRONE_ID.git
   cd ESP32_DRONE_ID
   ```

2. **Install dependencies**
   ```bash
   # For id_open module, download opendroneid core files
   # Copy opendroneid.c, opendroneid.h, odid_wifi.h, and wifi.c
   # from https://github.com/opendroneid/opendroneid-core-c
   # into the id_open/ directory
   ```

3. **Compile and upload**
   ```bash
   # Using Arduino IDE or Platform.io
   ```

## 📦 Project Modules

### id_open
Complete OpenDroneID implementation wrapper for ESP32.

**Supported Protocols:**
- BLE 4.0
- WiFi Beacon
- WiFi NAN

**Compatibility:** Opendroneid release 2.0+

**Note:** Known ESP32 limitation - some devices may experience reboots when both WiFi and Bluetooth are enabled simultaneously. If this occurs, use one protocol at a time.

### id_france
French regulatory-compliant WiFi UAV electronic ID.

**Status:** Compatible with Gendarmerie Nationale detection systems

**Details:**
- Works with `recepteur_balise.ino` receiver implementations
- 4-byte offset calibration from official python scripts
- Requires real-world testing validation

### id_scanner
Real-time drone ID scanner and receiver module for monitoring remote ID signals.

## ⚙️ ESP-IDF Migration (In Progress)

This project is being upgraded to **ESP-IDF** framework with the following improvements:

- **Graphical Configuration Tool** - Set drone IDs, registration info, and transmission parameters through an intuitive UI
- **Better Performance** - Direct hardware access and optimization
- **Enhanced Security** - Improved tamper resistance and authentication
- **Professional Standards** - Production-ready firmware

### Planned Features
- Web-based configuration interface
- Over-The-Air (OTA) updates
- Persistent configuration storage
- Multi-region regulatory support

## ⚠️ Regulatory Compliance Notice

When developing remote IDs for use in regulated airspace:

1. **ANSI/CTA Serial Numbers** - US and EU regulations require specific serial number formats
2. **Tamper Resistance** - FAA and EASA mandate tamper-resistant implementations
3. **Local Regulations** - Always verify compliance with your local aviation authority

Refer to:
- [OpenDroneID Regulatory Overview](https://github.com/opendroneid/opendroneid-core-c)
- [FAA Remote ID Requirements](https://www.faa.gov/uas/remote_id/)
- [EASA Remote ID Guidelines](https://www.easa.europa.eu/)

## 📚 Documentation

- [OpenDroneID Specification](https://github.com/opendroneid/opendroneid-core-c)
- [ESP32 Hardware Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- Module-specific READMEs in respective directories

## 🤝 Contributing

Contributions are welcome! Areas needing help:

- [ ] ESP-IDF migration and testing
- [ ] Graphical configuration tool development
- [ ] Regional regulatory compliance modules
- [ ] Hardware compatibility testing
- [ ] Documentation and examples

## 📋 Supported Hardware

- **ESP32-WROOM** (Recommended)
- **ESP32-WROVER**
- Any standard ESP32 dev board with WiFi + Bluetooth

## 📄 License

This project is licensed under the MIT License - see LICENSE file for details.

## 🔗 Related Projects

- [OpenDroneID Core (C)](https://github.com/opendroneid/opendroneid-core-c)
- [Gendarmerie Nationale Drone Reception System](https://github.com/GendarmerieNationale/ReceptionInfoDrone)
- [nRF52840 Remote ID (Bluetooth 5)](https://github.com/sxjack/remote_id_bt5)

## 📞 Support & Issues

- Report bugs and issues via GitHub Issues
- Check existing documentation and issues before submitting
- Provide hardware details and reproduction steps with bug reports

---

**Last Updated:** 2026
**Maintainer:** VOLTEKOVER
