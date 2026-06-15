# ESP DRONE REMOTEID — Analisi Completa del Codice

## `ESP32_DRONE_REMOTE_ID_Firmware/`

### `main/main.c` — Entry point
- Chiama `esp_rid_init()` + `esp_rid_start()`, stampa splash
- **OK.** Nessun problema.

### `main/CMakeLists.txt`
- Compila `main.c`
- **OK.**

### `CMakeLists.txt` (root firmware)
- Definisce componenti, include `mavlink/`, registra `esp_remote_id`
- **OK.**

### `idf_component.yml` — ESP-IDF registry manifest
- Versione `1.0.0-beta`, descrizione errata ("Scanner" invece di "Transmitter")
- ❌ **Descrizione obsoleta**: dice "OpenDroneID WiFi Beacon Scanner"

### `sdkconfig` — Config ESP-IDF generata (3461 righe)
- ❌ **File generato tracciato in git**. `sdkconfig` è già in `.gitignore` ma quello attuale è committato. Rimuovere dal tracking.

### `sdkconfig.defaults` — Default configurazione
- Solo `CONFIG_BT_ENABLED=y`
- **OK.** Il resto è settato dal CI.

### `partitions.csv` — Tabella partizioni 4MB (ESP32/S3/C3/C6)
- factory 1920KB + ota_0 1920KB + nvs 20KB + coredump 8KB
- **OK.**

### `partitions_2mb.csv` — Tabella partizioni 2MB (ESP32-C2)
- factory 960KB + ota_0 960KB + nvs 20KB + coredump 8KB
- **OK.**

---

## `components/esp_remote_id/include/`

### `esp_remote_id.h` — API pubblica, struct, enum
- Definisce `rid_config_t`, `rid_state_t`, `rid_gps_data_t`, `rid_identity_t`
- **`altitude_baro`** (riga 37): campo definito in `rid_gps_data_t` ma **mai popolato** da nessun parser. Rimane sempre 0.
- **`RID_OPT_FORCE_ARM_OK`** (riga 12): logica ridondante in `esp_remote_id.c:286`

### `opendroneid.h` — Intel ODID library header (762 righe)
- Tutte le enum ASTM + encode/decode API
- **OK.**

### `odid_wifi.h` — Struct IEEE 802.11 packed + NAN frame definitions
- Strutture packed per beacon/NAN/service discovery
- **OK.**

### `wifi_tx.h`
- Dichiarazioni: `wifi_tx_init()`, `wifi_tx_transmit()`, `wifi_tx_transmit_nan()`
- **OK.**

### `ble_tx.h`
- Dichiarazioni: `ble_tx_init()`, `ble_tx_transmit_legacy()`, `ble_tx_transmit_lr()`
- **OK.**

### `mav2odid.h` — Intel MAVLink-to-ODID conversion header
- Dichiarazioni `m2o_*` per convertire messaggi MAVLink RID in ODID
- ❌ **Mai chiamato a runtime.** `esp_remote_id.c` usa `mavlink_parser.c` direttamente. Il modulo è compilato ma morto.

### `protocol_detect.h`, `nmea_parser.h`, `msp_parser.h`, `mavlink_parser.h`
- Header dei parser seriali
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

### `esp_remote_id.c` — Orchestratore principale (350 righe)
- Init sequenza, task loop, `update_transmissions()`, rate limiting
- **Rate limiting** — FISSATO: `rate_allowed()` usa `esp_timer_get_time()` per rispettare `rate_hz`
- **WiFi NAN** — FISSATO: ora chiamato in `update_transmissions()`
- **GPS staleness** — FISSATO: `g_state.gps_valid` impostato a false dopo 10s senza aggiornamenti
- **LED integration** — FISSATO: `led_status_update()` chiamato dopo ogni tx
- ❌ **Logica ridondante `force_tx`** (riga 284-286): `if (gps_data.fix_type >= 2 && ...)` poi `if (force_tx || gps_data.fix_type >= 2)` — la seconda condizione è **sempre vera** perché la prima già la garantisce. Il flag `force_tx` non ha mai effetto.

### `wifi.c` — Frame builder IEEE 802.11 (614 righe)
- Costruisce beacon + NAN action frame
- **OK.** Codice stabile da Intel ODID library.

### `wifi_tx.c` — Trasmissione WiFi (204 righe)
- `wifi_tx_init()`: AP + promiscuous mode, MAC random
- `wifi_tx_transmit()`: costruisce e invia beacon con `esp_wifi_80211_tx()`
- `wifi_tx_transmit_nan()`: costruisce e invia NAN action frame
- **OK.** Duplica la popolazione di `g_uas_data` — si potrebbe refactorizzare in una funzione comune.

### `ble_tx.c` — Trasmissione BLE (183 righe)
- BLE 4.0 legacy + BLE 5.0 LR Coded PHY
- ❌ **BLE 5.0 LR richiede hardware specifico** (ESP32-S3/C3 per Coded PHY). Contrassegnato Beta nella UI.
- ❌ **`ble_tx_transmit_legacy()` usa `ADV_TYPE_NONCONN_IND`** — potrebbe non essere rilevato da tutti i ricevitori. Valutare `ADV_SCAN_IND`.

### `protocol_detect.c` — Auto-detect protocollo UART (75 righe)
- Legge primi byte dal seriale: `$M<`=MSP, `$G/$N`=NMEA, `0xFE/0xFD`=MAVLink
- **OK.** Funziona.

### `nmea_parser.c` — Parsing NMEA (136 righe)
- GGA + RMC. Estrae lat/lon/alt/speed/heading/fix/sat
- ❌ **Non popola `altitude_baro`**

### `msp_parser.c` — Parsing MSP (126 righe)
- MSP_RAW_GPS (106), MSP_ATTITUDE (108), MSP_STATUS (101)
- ❌ **Non popola `altitude_baro`**

### `mavlink_parser.c` — Parsing MAVLink (180 righe)
- GLOBAL_POSITION_INT, GPS_RAW_INT, VFR_HUD, ATTITUDE, HEARTBEAT
- **AHRS2** — FISSATO: supporto per messaggio ArduPilot AHRS2 (heading secondario)
- **OPEN_DRONE_ID_LOCATION** — FISSATO: parsing e popolazione GPS data (lat/lon/alt/vel/heading + altitude_baro)
- **OPEN_DRONE_ID_BASIC_ID** — FISSATO: estrae UAS ID, id_type, ua_type in identity
- **OPEN_DRONE_ID_OPERATOR_ID** — FISSATO: estrae operator_id in identity
- **OPEN_DRONE_ID_SYSTEM** — FISSATO: estrae operator lat/lon per GPS fallback
- **`altitude_baro`** — FISSATO: popolato da OPEN_DRONE_ID_LOCATION.altitude_barometric
- **`mavlink_parser_get_identity()`** — nuova funzione per ottenere identity da messaggi ODID
- Dialect: cambiato da `common/mavlink.h` a `ardupilotmega/mavlink.h` (include common + messaggi ArduPilot)
- Supporta timeout 5s su GPS, 10s su identity

### `mav2odid.c` — Intel MAVLink-to-ODID conversion (636 righe)
- FISSATO: ora COMPILATO e IN FUNZIONE: `mavlink_parser.c` include `mav2odid.h`
- Include path cambiato a `ardupilotmega/mavlink.h` per consistenza

### `opendroneid.c` — Intel ODID encode/decode library (1477 righe)
- **OK.** Stabile, testato.

### `web_config.c` — Server HTTP + REST API + OTA + logs (463 righe)
- Serve `config.html`, API `/api/config`, `/api/status`, `/api/command`, OTA
- **Lock level** — FISSATO: controllo base implementato (blocca comandi distruttivi a level ≥ 1, OTA a level ≥ 2)
- ❌ **Lock level 2 eFuse burn** non implementato: il level 2 è gestito come "level 1 + OTA bloccato", ma non viene bruciato alcun eFuse. Il lock è reversibile via factory reset.
- ❌ **`handle_ota`** non valida checksum/autenticazione del firmware caricato. Qualunque file viene accettato.

### `nvs_storage.c` — Persistenza NVS (174 righe)
- Salva/carica `rid_config_t` in NVS namespace `esp_rid`
- **OK.**

### `led_status.c` — LED RGB (76 righe)
- 3 GPIO configurabili via Kconfig. Default: -1 (disabilitato)
- Rosso = no fix, Verde = fix OK, blink 80ms su TX
- **OK.** Funzionale.

### `CMakeLists.txt` (component)
- Elenca tutti i file .c inclusi
- ❌ Include `mav2odid.c` che non è mai chiamato (vedi sopra)

### `Kconfig.projbuild` — Menuconfig per LED
- **OK.** Aggiunto.

---

## `webui/`

### `config.html` — Web UI completa (1201 righe)
- Dark/light theme, dashboard, forms, logs, OTA, terminal
- **WiFi NAN** — FISSATO: checkbox abilitato, bar chart aggiunto, contatori dashboard aggiunti, stile viola
- ❌ **`public_keys` UI esiste ma lock level signature verification non implementata**
- ❌ **La UI è inline (JS + CSS dentro HTML)** — 1201 righe, difficile da mantenere. Refactoring html/css/js separati opzionale.

---

## `mavlink/` — MAVLink v2 C dialect libraries

### Catena di inclusione (verificata):
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

- **`mavlink/ardupilotmega/`**: ✅ USATO (dialect principale ora: `#include "ardupilotmega/mavlink.h"`)
- **`mavlink/common/`**: ✅ USATO (incluso da `ardupilotmega/ardupilotmega.h:1049`)
- **`mavlink/minimal/`**: ✅ USATO (incluso da `common/common.h:2695`)
- **`mavlink/protocol.h`**: ✅ USATO (da tutti i dialect)
- **`mavlink/mavlink_types.h`**: ✅ USATO (da `protocol.h`)
- **`mavlink/mavlink_get_info.h`**: ✅ USATO (da `common/common.h`)
- **`mavlink/message_definitions/common.xml`**: ✅ USATO
- **`mavlink/uAvionix/`**: 🔶 INCLUSO TRANSITIVAMENTE (da `ardupilotmega.h:1050`) ma nessun messaggio uAvionix parsato
- **`mavlink/icarous/`**: 🔶 INCLUSO TRANSITIVAMENTE (da `ardupilotmega.h:1051`) ma nessun messaggio icarous parsato

I seguenti sono **ancora INUTILIZZATI**:

- **`mavlink/standard/`**: ❌ (solo `development/` lo include)
- **`mavlink/ASLUAV/`**: ❌
- **`mavlink/matrixpilot/`**: ❌
- **`mavlink/development/`**: ❌
- **`mavlink/test/`**: ❌
- **`mavlink/mavlink_helpers.h`**: ❌ (solo per MAVLink signing, non implementato)
- **`mavlink/mavlink_sha256.h`**: ❌ (solo incluso da `mavlink_helpers.h`)
- **`mavlink/mavlink_conversions.h`**: ❌ (solo incluso da `mavlink_helpers.h`)
- **Tutti i `testsuite.h`**: ❌
- **`mavlink/message_definitions/`**: 8 XML su 11 inutilizzati

> `uAvionix/` e `icarous/` sono ora inclusi ma nessun codice ne usa i messaggi specifici — sono header innocui.

---

## `README.md` — Documentazione principale del repo
- Badge CI, version, piattaforma supportate
- Quick start, feature table, wiring guide, build instructions
- ❌ **Dice "Hardware testing pending"** in più punti. Da aggiornare dopo i test.
- ❌ **Sezione "Features"** elenca lock level come "code complete, untested" — parzialmente superato (lock level base è testabile)
- ❌ **Menzione "no extra GPS"** ma in realtà il firmware richiede una FC con GPS

---

## `.gitignore` — Regole di esclusione git (140 righe)
- Copre build/, sdkconfig, __pycache__, .vscode/*.json, *.bin (con eccezioni)
- ❌ `.vscode/*.json` è escluso ma `!.vscode/settings.json` lo ri-include — OK quello è il comportamento voluto
- **OK.** Ben strutturato.

## `.gitattributes` — Normalizzazione EOL (40 righe)
- LF per C/py/js/json/md, CRLF per bat/ps1
- **OK.**

---

## `docs/` — GitHub Pages sito

### `index.html` — Landing page + WebSerial installer (1979 righe)
- WebSerial flasher con `esp-web-tools`, wiki integrata, guida step-by-step
- Dark/light mode nativa, responsive
- Sezioni: compatibilità FC, build matrix, guida wiring, flashing, FAQ
- ❌ **Inline CSS/JS** — 1979 righe tutto in un file. Refactoring html/css/js separati opzionale.
- 🟡 **Stato "DEV build"** — da aggiornare dopo rilascio stabile

### `config(demo).html` — UI demo standalone (1152 righe)
- Copia offline di `config.html` per dimostrazione senza hardware
- Dark/light theme, tutti i campi, simulazione telemetria
- ❌ **Non sincronizzata automaticamente** con `config.html` — ogni modifica va portata a mano

### `manifest.json` — Firmware manifest per WebSerial (33 righe)
- 3 chipFamily (ESP32, ESP32-S3, ESP32-C3) con offset bootloader e partizioni
- ❌ **Mancano esp32s2, esp32c2, esp32c6** — il CI genera il manifest dinamico ma questo file statico in docs/ è vecchio
- ❌ **Versione hardcoded** "0.1.0-dev" — non aggiornata

### `images/` — Asset grafici (5 file)
- `logo.svg`, `logo con scritta.svg`, `ardupilot_logo.webp`, `betaflight_logo.svg`, `inav_logo.png`
- **OK.**

---

## `.github/`

### `workflows/build.yml` — CI build + Pages deploy (193 righe)
- Matrix 6 target (esp32, esp32s2, esp32s3, esp32c2, esp32c3, esp32c6) ✅
- Partition/flash/BLE conditionals per target ✅
- ccache per build più veloci ✅
- download-artifact con pattern `firmware-*` ✅
- `manifest.json` generato dinamicamente con Python ✅
- Deploy GitHub Pages su push a main ✅
- **OK.** Funzionale.

### `workflows/release.yml` — Release su tag v* (308 righe)
- Matrix 6 target, allineato a `build.yml` ✅
- Genera zip per target + manifest.json ✅
- Release notes con changelog automatico (`git log`) ✅
- Deploy GitHub Pages anche in release ✅
- **OK.** Funzionale.

### `dependabot.yml` — Aggiornamenti automatici
- `package-ecosystem: "github-actions"`, schedule settimanale
- **OK.**

### `PULL_REQUEST_TEMPLATE.md` — Template PR
- Checklist: coding style, hardware test, docs, build, warnings
- **OK.**

### `ISSUE_TEMPLATE/bug_report.md` — Template bug report
- Campi: hardware, firmware, config, console output
- **OK.**

### `ISSUE_TEMPLATE/feature_request.md` — Template feature request
- Campi: descrizione, soluzione proposta, protocolli interessati
- **OK.**

---

## `.vscode/`

### `c_cpp_properties.json` — IntelliSense C/C++
- Compiler path: `xtensa-esp32-elf-gcc.exe` locale
- `compileCommands` da `build/compile_commands.json`
- ❌ **Percorsi assoluti Windows** — non portabile su altri sistemi

### `launch.json` — Debug config
- Solo "Eclipse CDT GDB Adapter" attach
- ❌ **Configurazione minimale** — mancano profile di debug veri (es. OpenOCD + gdb)

### `settings.json` — Workspace settings VSCode
- Clangd config + IDF extension config
- `"idf.currentSetup": "C:\\esp\\v6.0.1\\esp-idf"`
- ❌ **Percorsi assoluti Windows** — non portabile

---

## Riepilogo Priorità

| Priorità | Cosa | Dove | Stato |
|----------|------|------|-------|
| 🔴 ALTA | `altitude_baro` popolato da parser MAVLink (manca NMEA/MSP) | `nmea_parser.c`, `msp_parser.c` | 🟡 Parziale (solo MAVLink ODID) |
| 🔴 ALTA | Logica `force_tx` ridondante (riga 284-286) | `esp_remote_id.c` | ❌ Da fixare |
| 🟡 MEDIA | Lock level 2: eFuse burn non implementato | `web_config.c` | ❌ Da fare |
| 🟡 MEDIA | `sdkconfig` tracciato in git (generato) | Rimuovere dal tracking | ❌ Da fare |
| 🟡 MEDIA | `docs/manifest.json` hardcoded (mancano targets) | `docs/manifest.json` | ❌ Da fare |
| 🟡 MEDIA | `README.md` dice "hardware testing pending" | `README.md` | ❌ Da fare |
| 🟡 MEDIA | `docs/config(demo).html` non sincronizzato con `config.html` | Manutenzione manuale | ❌ Da fare |
| 🟡 MEDIA | `idf_component.yml` descrizione errata ("Scanner") | `idf_component.yml` | ❌ Da fare |
| 🟢 BASSA | BLE 4.0 usa `ADV_TYPE_NONCONN_IND` — test compatibilità | `ble_tx.c` | ❌ Da testare |
| 🟢 BASSA | OTA non valida checksum/autenticazione | `web_config.c` | ❌ Da fare |
| 🟢 BASSA | Duplicazione popolazione `g_uas_data` in `wifi_tx.c` | Refactoring opzionale | ❌ Opzionale |
| 🟢 BASSA | `.vscode/` percorsi assoluti Windows | `.vscode/*.json` | ❌ Opzionale |
| 🟢 BASSA | `config.html` inline — refactoring opzionale | `config.html` | ❌ Opzionale |
| 🟢 BASSA | `docs/index.html` inline (1979 righe) | Refactoring opzionale | ❌ Opzionale |
| ✅ RISOLTO | `mav2odid.c` integrato in `mavlink_parser.c` | `mavlink_parser.c` + `mav2odid.h` | ✅ Fatto |
| ✅ RISOLTO | `ardupilotmega/` ora dialect MAVLink principale | `mavlink_parser.c` | ✅ Fatto |
| ✅ RISOLTO | `AHRS2` heading secondario da ArduPilot | `mavlink_parser.c` | ✅ Fatto |
| ✅ RISOLTO | Identity da MAVLink ODID override configurazione | `esp_remote_id.c` | ✅ Fatto |
| ✅ RISOLTO | `altitude_baro` popolato da MAVLink ODID Location | `mavlink_parser.c` | ✅ Fatto |
