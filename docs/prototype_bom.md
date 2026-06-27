# DIY RID Prototype — BOM zero-saldatura

## Board principale

| Opzione | Chip | BLE | WiFi | Extra | Prezzo | Dove |
|---|---|---|---|---|---|---|
| **ESP32-DevKitC V4** (con IPEX) | ESP32 | 4.2 LR | 4 | Più economico, maturo | ~5€ | AliExpress, Amazon |
| **ESP32-S3-DevKitC-1** (con IPEX) | ESP32-S3 | 5.0 LR | 4 + 5 GHz | ESP-NN (ML), USB OTG | ~12€ | AliExpress, Amazon |
| **ESP32-C6-DevKitC-1** (con IPEX) | ESP32-C6 | 5.4 **Channel Sounding** | 6 (OFDMA) | 802.15.4 (Thread/Zigbee) | ~15€ | AliExpress, Amazon |

Tutte hanno versioni con **connettore IPEX/U.FL** per antenna esterna. Chi non lo trova, può prendere la versione PCB antenna e usare il board così com'è (funziona benissimo).

## Antenna esterna (opzionale, no saldatura)

| Cosa | Connettore | Prezzo |
|---|---|---|
| **Pigtail U.FL → SMA** | Plug & play | ~1€ |
| **Antenna 2.4GHz SMA** (3-5 dBi) | Si avvita a mano | ~2€ |

Si collega in 5 secondi, zero saldatura.

## Hardware aggiuntivo (solo Dupont, zero saldatura)

| Componente | Collegamento | Prezzo | Opzionale? |
|---|---|---|---|
| **WS2812B RGB LED** (Neopixel) | 5V, GND, DATA (3 pin Dupont) | ~1€ | No (status visivo) |
| **GPS BN-880 / NEO-6M / NEO-8M** | VCC, GND, TX, RX (4 pin Dupont) | ~6€ | Sconsigliato (RID senza GPS non ha senso) |
| **MPU6050 (IMU 6-axis)** | 3.3V, GND, SCL, SDA (4 pin Dupont) | ~2€ | Sì (anti-spoofing inerziale) |
| **Push button** | 3.3V, GPIO (2 fili) | ~0.5€ | Sì (start/stop volo manuale) |

## Alimentazione

| Come | Cosa serve |
|---|---|
| **USB-C** (test/sviluppo) | Power bank o caricatore smartphone |
| **Dal drone** (BEC / PDB) | Cavo USB o fili su pin 5V/GND |

## BOM totale

| Componente | Prezzo |
|---|---|
| ESP32 dev board (con IPEX) | 5-15€ |
| Pigtail U.FL → SMA | 1€ |
| Antenna SMA 2.4GHz | 2€ |
| WS2812B LED | 1€ |
| GPS BN-880 | 6€ |
| MPU6050 (opzionale) | 2€ |
| Dupont cable kit (40x) | 2€ |
| **Totale** | **18-29€** |

Tutto si collega con cavetti Dupont. **Niente saldatore.** Chiunque può replicarlo.

## Configurazione pin (via menuconfig o web UI)

```
GPS TX  → GPIO 18
GPS RX  → GPIO 17
LED DATA → GPIO 48 (WS2812B) o R/G/B su GPIO separati
IMU SCL → GPIO 9  (MPU6050, opzionale)
IMU SDA → GPIO 8  (MPU6050, opzionale)
BUTTON  → GPIO 0  (opzionale)
```

## Innovazione (tutta in software)

| Feature | Cosa serve | Fattibilità |
|---|---|---|
| ESP-NOW Mesh RID | Solo software | Oggi |
| Attestazione HW (Digital Signature) | Solo software (ESP32 built-in) | Oggi |
| BT Channel Sounding (ranging) | ESP32-C6 | Oggi |
| Predizione traiettoria (Kalman) | Solo software | Oggi |
| Anti-spoofing (GPS + IMU) | MPU6050 + software | Oggi |
| WiFi 6 OFDMA | ESP32-C6 | Oggi |
| WiFi 5 GHz | ESP32-S3 o C6 | Oggi |
