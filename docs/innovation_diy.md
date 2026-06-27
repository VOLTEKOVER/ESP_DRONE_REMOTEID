# Innovation / DIY — ESP32 Remote ID vs Dronetag

## Confronto feature

### vs Mini 4G (con LTE)

| Feature | ESP32 DIY | Dronetag Mini 4G |
|---|---|---|
| Broadcast RID (DRI) | WiFi Beacon + WiFi NAN + BLE 4 + BLE 5 | BLE only |
| Network RID (NRI) | — | LTE Cat 1 bis |
| Standard | ASTM F3411 / prEN 4709-002 | ASTM F3411 / prEN 4709-002 |
| Protocolli FC | MAVLink + MSP + NMEA (auto) | Non specificato |
| Config | Web UI + CLI seriale | Mobile app |
| OTA | Web UI + SHA-256 verify | Mobile app |
| Sicurezza | ECDSA sign + eFuse lock | Non documentato |
| Led | RGB PWM state machine (7 stati) | LED indicatori |
| Costo HW | < 5€ (ESP32) | ~350€ |
| Open source | SI | NO |
| Abbonamento | NO | SI (LTE data plan) |

### vs DRI (modulo OEM per integratori)

| Feature | ESP32 DIY | Dronetag DRI |
|---|---|---|
| Broadcast RID (DRI) | WiFi Beacon + WiFi NAN + BLE 4 + BLE 5 | BLE 4 + BLE 5 LR only |
| Potenza BLE | 18 dBm max (config.) | 8 dBm |
| RF range dichiarato | — | 3 km (BLE) |
| Dati posizione | MAVLink / MSP / NMEA (auto) | MAVLink 2 only |
| Alimentazione | 3.3V / 5V via USB o pin | 3.3 – 17V (regolatore buck interno) |
| Consumo medio | ~80-150 mA (ESP32 + GPS) | **3 mA** |
| Consumo max | ~500 mA | 10 mA |
| UART forwarding | No (occupa una UART) | Si (bypass UART integrato) |
| Dimensioni | Dipende dalla board | 22.5 × 16 × 5 mm |
| Peso | Dipende dalla board | 0.5 – 1.5 g |
| Temp. operativa | Tipica 0-85°C | -40°C a +85°C |
| Serial number | Generato da MAC | ANSI/CTA-2063-A pre-programmato |
| Certificazioni | — | FCC/CE (modulo radio approvato) |
| Config | Web UI + CLI (nessuna app) | Mobile app Dronetag |
| Open source | SI | NO |
| Costo | < 5€ (ESP32) | ~50-100€ (stima) |

Il **DRI è il competitor più diretto** — stesso use case (FC → RID), ma con vantaggi enormi su consumo (3mA vs 80+), formato e certificazioni. Lo battere solo su multi-link WiFi+BLE, multi-protocollo, OTA, sicurezza e costo.

### vs BS gen.2 (standalone economico, nostro competitor diretto)

| Feature | ESP32 DIY | Dronetag BS gen.2 |
|---|---|---|
| Broadcast RID | WiFi Beacon + WiFi NAN + BLE 4 + BLE 5 | BLE 4 Legacy + BLE 5 LR only |
| Range dichiarato | — | 3 km (BLE) |
| Potenza BLE | 18 dBm max (config.) | 8 dBm |
| GNSS | **Esterna via UART** (1 costellazione) | **Integrata u-blox M10** multi-costellazione |
| Alimentazione | 3.3V / 5V (USB o pin) | 3.3 – 17V con buck regulator |
| Consumo medio | ~80-150 mA + GPS esterno | **15 mA** |
| Consumo max | ~500 mA | **50 mA** |
| Dimensioni | Dipende da board + GPS | **18 × 14 × 5 mm** |
| Peso | Dipende | **1.5g** (con heat shrink) / **3g** (con enclosure) |
| Temp. operativa | Tipica 0-85°C | **-40°C a +85°C** |
| Config | Web UI + CLI (nessuna app) | Solo mobile app Dronetag |
| OTA | Web UI con SHA-256 | Mobile app |
| Antenne | WiFi/BLE stampata + GPS esterna | 2× U.FL (GNSS + BLE, esterne) |
| GNSS timekeeping | Nessuno | **Supercapacitor** — mantiene fix ~8 min durante power cycle |
| Spektrum XBUS / SRXL2 | No | **Sì** (telemetria diretta su radio Spektrum) |
| Betaflight GNSS input | No | **Sì** — BS funge da GPS per Betaflight |
| Flight data logging | No | In sviluppo (su flash interna) |
| Telemetry module (Futaba/Spektrum) | No | In sviluppo |
| SAW filters | No | **Sì** (immunità interferenze GNSS) |
| Target utente | Tecnico / sviluppatore | Piloti esperti (saldatura richiesta) |
| Open source | **SI** | NO |
| Costo | < 5€ (ESP32) + 10€ GPS | ~**99€** |

**BS gen.2 è il competitor più insidioso** perché:
- **Stesso use case**: standalone, senza FC, GNSS + BLE
- **Formato e peso imbattibili**: 18mm, 1.5-3g
- **Consumo 15mA** — vola per ore con batteria piccola
- **Range input 3.3-17V** con regolatore integrato
- **Supercapacitor** — fix GNSS persiste tra power cycle (noi no)
- **Doppia funzione GNSS + RID**: BS alimenta Betaflight con dati GPS via XBUS (UART, protocollo UBLOX). Un solo device fa sia da ricevitore GPS per il volo che da trasmettitore RID. Noi abbiamo GPS solo per RID, non lo inoltriamo al FC.
- **Connettore 4-pin JST SH**: VCC (3.3-17V), RX, TX, GND — stesso connettore per alimentazione, Betaflight e Spektrum XBUS
- **Supercapacitor GNSS**: mantiene fix ~8 min durante cambio batteria (TTFF ridotto). Noi riacquisiamo da zero.
- **Spektrum XBUS/SRXL2**: telemetria nativa su radio RC Spektrum (futuro: Futaba e altri)

Cosa noi abbiamo e BS non ha:
- **WiFi multi-link** (WiFi Beacon + NAN, in aggiunta a BLE)
- **Open source**
- **Web UI + CLI** (nessuna app)
- **Multi-protocollo FC** (MAVLink, MSP, NMEA)
- **OTA via web** con verifica SHA-256
- **Sicurezza firme ECDSA + eFuse lock**
- **Estensibilità** (possiamo aggiungere mesh, LoRa, SD, CAN — BS è chiuso)

### vs Beacon gen.2 (solo BLE, con batteria)

| Feature | ESP32 DIY | Dronetag Beacon gen.2 |
|---|---|---|
| Broadcast RID (DRI) | WiFi Beacon + WiFi NAN + BLE 4 + BLE 5 | BLE only |
| GNSS | 1 costellazione (via UART GPS) | GPS L1 + GLONASS + Galileo + BeiDou + SBAS + QZSS |
| Sensori extra | — | Barometro, accelerometro |
| Potenza BLE | Config. 18 dBm max | 8 dBm |
| Consumo medio | ~80-150 mA (ESP32 + GPS) | 15 mA |
| Consumo max | ~500 mA | 100 mA |
| Batteria | No (alimentazione esterna) | Li-Po 200 mAh, 10-18h |
| Dimensioni | Dipende dalla board | 37 x 26 x 16 mm |
| Peso | Dipende dalla board | 17 g |
| IP | — | IP43 |
| Conformità | — | FAA accepted / EASA listed |
| Costo HW | < 5€ (ESP32) | ~200€ |
| Open source | SI | NO |

## Vantaggi su carta (contro tutti)

- **Multi-link simultaneo**: WiFi beacon + NAN + BLE legacy + BLE LR — Dronetag trasmette solo BLE
- **Nessun abbonamento**: WiFi/BLE gratis, nessuna SIM
- **Trasparente**: codice aperto, modificabile, espandibile
- **Web UI**: configuri da browser, nessuna app
- **Multi-FC**: parla con ArduPilot, PX4, Betaflight, iNAV, GPS nativi
- **Sicurezza HW**: firma ECDSA + eFuse burn permanente
- **Configurabilità totale**: potenza, canale, rate, gpio, tutto modificabile
- **Potenza BLE** superiore (18 vs 8 dBm)
- **Nessuna dipendenza da pressione barometrica** — DRI richiede SCALED_PRESSURE dal FC per comporre i messaggi, noi funzioniamo con GPS + MAVLink e basta
- **Protocolli extra** — DRI supporta solo MAVLink 2; noi anche MSP e NMEA (GPS diretto)
- **OTA aggiornabile via web** — DRI non ha OTA, si aggiorna solo via app mobile

## DRI — analisi approfondita dall'integration guide

### MAVLink messages che DRI richiede dal flight controller
- ALTITUDE (pressione)
- SCALED_PRESSURE (dati barometrici)
- GPS_RAW_INT (posizione GNSS)
- SYSTEM_TIME (riferimento temporale)
- GLOBAL_POSITION_INT (posizione globale)
- HEARTBEAT (stato sistema)
- OPEN_DRONE_ID_BASIC_ID (sovrascrive seriale DRI)
- OPEN_DRONE_ID_LOCATION
- OPEN_DRONE_ID_SYSTEM
- OPEN_DRONE_ID_OPERATOR_ID
- OPEN_DRONE_ID_ARM_STATUS (blocco decollo se RID non funziona)

### Certificazioni — il vero moat di Dronetag

| Aspetto | Dronetag DRI | Nostro ESP32 |
|---|---|---|
| **FAA Standard RID** | Compliance matrix + DoC pronti | Dovremmo fare tutto da zero |
| **FCC** | Usa modulo u-blox ANNA-B4 già certificato (FCC ID: XPYANNAB4) → "Contains FCC ID" | ESP32 ha FCC ID Espressif, ma custom board richiede nuova certificazione |
| **EU C-class** | Guide per Modulo A/B+C/H con Notified Body | Nessun percorso definito |
| **SN ANSI/CTA-2063-A** | Pre-programmato, ICAO prefix gestito | Solo MAC address |
| **Antenne pre-approvate** | Lista ufficiale di antenne certificate | Nessuna |
| **Lab testing** | ASTM F3586-22 da laboratorio accreditato | Mai fatto |
| **Costo stimato certificazione** | Già sostenuto (incluso nel prezzo ~50-100€) | Migliaia di € + tempi lunghi |

**Morale**: la certificazione è il loro vantaggio più difficile da battere. Per un prototipo DIY non serve, ma per un prodotto reale è un ostacolo significativo.

### Punti di forza DRI (da colmare)

| Area | DRI | Nostro ESP32 | Gap |
|---|---|---|---|
| Consumo | 3 mA avg, 10 mA max | ~80-150 mA | **20-50x** |
| Alimentazione | 3.3-17V con regolatore buck | 3.3V/5V via USB | Richiede regolatore esterno |
| Temp. operativa | -40°C a +85°C | Tipica 0-85°C | Copertura inferiore |
| Certificazioni | FCC/CE pre-certificato | Nessuna | Costoso da ottenere |
| SN | ANSI/CTA-2063-A pre-programmato | Generato da MAC | Minimo, si può aggiungere |
| UART forwarding | Sì (bypass integrato) | No | Occupa una UART |
| Arming failsafe | Blocca decollo se RID assente | No logica failsafe | Da implementare |
| Peso | 0.5-1.5g | Variabile | Gap notevole |
| Dimensioni | 22.5×16×5 mm | Variabile | Gap notevole |

### Cosa loro NON possono fare (e noi sì)

- **Trasmettere su WiFi** — il DRI è solo BLE. WiFi ha portata maggiore ed è ricevibile da più dispositivi (telefoni, laptop, stazioni dedicate)
- **Supportare protocolli non-MAVLink** — MSP (Betaflight/iNAV) e NMEA (GPS diretto) non sono supportati
- **Ota firmware updates via web** — l'aggiornamento richiede app mobile + BLE
- **Configurazione senza app** — noi abbiamo CLI seriale + web UI, loro solo app mobile
- **Mesh relay** — impossibile via BLE, richiederebbe HW aggiuntivo
- **Telemetry bridge WiFi** — il DRI usa solo UART, nessuna uscita telemetrica wireless
- **Logging e debug** — nessuna interfaccia testuale o web per diagnostica
- **Piena personalizzazione** — firmware closed, parametri limitati a ciò che l'app espone

## Idee innovative per prototipo DIY

### Obiettivo
> Trasmettitore RID che funziona **senza LTE, senza SIM, senza abbonamento**, superando Dronetag in features, flessibilità e costo, pur rimanendo **rilevante** per applicazioni reali.

### 1. ESP-NOW Mesh RID Relay ★★★★★
- I droni in volo relayano i messaggi RID tra loro via ESP-NOW
- Droni oltre la linea di vista risultano visibili se un altro drone li ha incontrati
- Nessun prodotto commerciale offre mesh RID
- Complessità: media — ESP-NOW è nativo su ESP32

### 2. Dual-band 2.4 + 5 GHz ★★★★☆
- ESP32-C5/S3 supporta 5 GHz
- RID su 5 GHz è meno congestionato, più affidabile in aria
- Nessun RID commerciale usa 5 GHz oggi

### 3. WiFi Telemetry Bridge ★★★★☆
- Stessa interfaccia WiFi usata per RID la si usa come hotspot telemetry
- Smartphone si connette direttamente al drone (nessuna SIM, nessun cloud)
- MAVLink bidirezionale via UDP/WebSocket

### 4. LoRa Long-range Backup ★★★★☆
- Modulo LoRa (SX1262) per RID a 10+ km
- Doppia trasmissione: WiFi/BLE per corto raggio + LoRa per lungo raggio
- Drone perso? Ultima posizione su LoRa ricevibile da terra con ricetrasmettitore cheap

### 5. SD Card Logging ★★★☆☆
- Log completo volo su microSD: posizione, stato, trasmissioni
- Geofence offline (no-fly zone caricate su SD)
- Dronetag non ha SD

### 6. CanBus / DroneCAN bridge ★★★☆☆
- Lettura diretta CAN bus drone (CUAV, Zubax, ecc.)
- Funziona senza MAVLink — posizione, batteria, stato motori
- Utile per droni custom o FC che non espongono MAVLink

### 7. ADSB Receiver ★★★☆☆
- RTL-SDR (R820T2) cattura messaggi ADSB da aerei
- Il drone vede aerei vicini e può evitarli
- Feature che nessun RID commerciale ha

### 8. Edge ML anti-spoofing ★★☆☆☆
- Confronta posizione GPS con WiFi RTT, IMU, cell tower RSSI
- Rileva spoofing e firma i pacchetti RID con chiave attestata
- Complesso ma realistica su ESP32-S3 con ESP-NN

## Architettura proposta

```
┌─────────────────────────────────────────┐
│  ESP32 (S3 / C5 / C6)                   │
│                                         │
│  ┌─────────┐ ┌──────────┐ ┌──────────┐ │
│  │ RID     │ │ MAVLink  │ │ WiFi     │ │  ← già fatto
│  │ Beacon  │ │ /MSP/NMEA│ │ Hotspot  │ │
│  └─────────┘ └──────────┘ └──────────┘ │
│  ┌─────────┐ ┌──────────┐ ┌──────────┐ │
│  │ LoRa    │ │ ESP-NOW  │ │ SD Card  │ │  ← da fare
│  │ SX1262  │ │ Mesh     │ │ Logging  │ │
│  └─────────┘ └──────────┘ └──────────┘ │
│  ┌─────────┐ ┌──────────┐              │
│  │ ADSB    │ │ DroneCAN │              │  ← nice to have
│  │ RTL-SDR │ │ CAN bus  │              │
│  └─────────┘ └──────────┘              │
└─────────────────────────────────────────┘
```

## Certificazione — cosa serve per un prodotto reale

Se un giorno volessi trasformare il prototipo in prodotto, servono:

### USA (FAA + FCC)
1. **Means of Compliance (MoC)** — ASTM F3411 + test ASTM F3586-22 da lab accreditato
2. **Declaration of Compliance (DoC)** — sottomissione formale FAA
3. **FCC Certification** — test EMC/RF da lab accreditato + TCB
4. **SN ANSI/CTA-2063-A** — richiedere ICAO prefix + seriali unici
5. **"Contains FCC ID"** — se usi modulo pre-certificato (es. ESP32 già ha FCC ID)

### EU (EASA)
1. **Conformity assessment** — Modulo B+C o H con Notified Body
2. **CE marking + C-class label** (C0-C6)
3. **EN 4709-002 compliance**
4. **Technical documentation** completa

### Minimo per credibilità tecnica (anche senza certificazione)
- [ ] OPEN_DRONE_ID_ARM_STATUS → blocco decollo se RID non funziona
- [ ] Failsafe GNSS → RTH se perde fix
- [ ] Broadcast continuity → ultimi dati validi se GCS si disconnette
- [ ] SN ANSI/CTA-2063-A configurabile
- [ ] Operatore location da GCS (per FAA)
- [ ] Tamper-resistant storage per SN (eFuse già presente)

## Posizionamento — dove batterli davvero

| Dronetag fa | Noi facciamo meglio |
|---|---|
| BLE only | **WiFi + BLE** — nessun loro prodotto ha WiFi |
| Solo mobile app | **Web UI + CLI** — niente app da installare |
| Closed source | **Open source** — chiunque può contribuire |
| Nessuna sicurezza documentata | **Firme ECDSA + eFuse lock** |
| FW fisso | **OTA via web** verificato con SHA-256 |
| Solo MAVLink (DRI) | **MAVLink + MSP + NMEA** — più protocolli |
| Prodotto finito, punto | **Piattaforma estensibile** — mesh, LoRa, SD, CAN |

**La nostra tesi**: un RID transmitter non deve essere un prodotto chiuso. Deve essere una **piattaforma aperta** su cui costruire: mesh relay, telemetria, logging, integrazione con qualsiasi ecosistema drone. Dronetag vende dispositivi. Noi vendiamo **flessibilità**.

## Priorità di implementazione

1. **ESP-NOW Mesh** — impatto massimo, complessità media, nessuna dipendenza HW extra
2. **WiFi Telemetry Bridge** — già parzialmente possibile, sfrutta WiFi esistente
3. **LoRa backup** — impatto alto, richiede HW ma è economico (modulo 3-5€)
4. **SD Card + geofence** — impatto medio, richiede HW ma standard
5. **Dual-band 5 GHz** — impatto medio, richiede ESP32-S3/C5/C6
6. **CanBus / ADSB / ML** — impatto basso o complessità alta, da valutare dopo

## Proposta commerciale sintetica

> **RID Mesh Open Source** — trasmettitore multi-link (WiFi + BLE + LoRa) con relay mesh ESP-NOW, telemetria WiFi nativa, logging su SD, zero abbonamenti. Costo BOM <20€. Aperto, espandibile, superiore.
