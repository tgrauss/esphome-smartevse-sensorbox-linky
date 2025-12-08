# SmartEVSE SensorBox Linky (ESPHome)

Ce projet transforme un ESP32‑S3 (Zero ou ETH/W5500) en SensorBox compatible SmartEVSE‑2, et expose les données Linky (TIC) plus les mesures CT (ADS1115) sur deux bus Modbus esclaves.

## Fonctionnalités

- ADS1115: lecture CT sur A1/A2/A3 avec référence 1 V sur A0 pour autocalibration (single‑shot, PGA ±2.048 V, SPS 860).
- TIC Linky: mode standard via UART sur GPIO44 (9600 baud, 7 bits, parité even, 1 stop).
- Deux serveurs Modbus esclaves:
  - Bus SmartEVSE: 100 % compatible SensorBox‑2 (même map et registres).
  - Bus ESPHome: mapping Linky Modbus (adresses décimales), plus extras 2000–2049.
- Cadence: fast (1 s) pour valeurs instantanées, slow (2 s) pour énergies/labels, comme SensorBox‑2.
- Networking: OTA et webserver minimal (status, reboot). Aucune publication vers Home Assistant.

## Dépendances

Déclarez les composants externes dans le YAML:
```yaml
external_components:
  - source: github://tgrauss/esphome-smartevse-sensorbox-linky
    components: [smartevse_sensorbox, modbus_server]
    refresh: 0s
```

## Schéma matériel

- ADS1115 en 3.3 V:
  - A0: référence 1 V (ampli‑op).
  - A1/A2/A3: CT 100 A / 50 mA.
- Réseau analogique CT (par canal):
  - Burden 22 Ω (entre sortie CT et Vref 1 V).
  - 1 kΩ en série entre sortie CT et entrée ADS1115.
  - 1 kΩ entre entrée ADS1115 et Vref (1 V).
  - 100 nF entre entrée ADS1115 et GND.
- UART TIC: GPIO44 (RX).
- RS485 SmartEVSE: UART dédié (pins selon carte).
- RS485 ESPHome: UART dédié (pins selon carte).
- ESP32‑S3‑Zero: Wi‑Fi actif.
- ESP32‑S3‑ETH: Wi‑Fi désactivé, Ethernet W5500 actif.

## Mapping Modbus

Bus ESPHome (Linky, adresses décimales):
- 0: EAST (Wh, U_DWORD)
- 2: EAIT (Wh, U_DWORD)
- 4: EASF01 (Wh, U_DWORD)
- 6: EASF02 (Wh, U_DWORD)
- 8: EASF01 (Wh, U_DWORD) — EJP Normal
- 10: EASF02 (Wh, U_DWORD) — EJP Pointe
- 12: EASF01 (Wh, U_DWORD) — BLEU HC
- 14: EASF02 (Wh, U_DWORD) — BLEU HP
- 16: EASF03 (Wh, U_DWORD) — BLANC HC
- 18: EASF04 (Wh, U_DWORD) — BLANC HP
- 20: EASF05 (Wh, U_DWORD) — ROUGE HC
- 22: EASF06 (Wh, U_DWORD) — ROUGE HP
- 24: IRMS1 (A, U_WORD)
- 25: IRMS2 (A, U_WORD)
- 26: IRMS3 (A, U_WORD)
- 27: URMS1 (V, U_WORD)
- 28: URMS2 (V, U_WORD)
- 29: URMS3 (V, U_WORD)
- 30: SINSTS (VA, U_DWORD)
- 32: SINST1 (VA, U_DWORD)
- 34: SINST2 (VA, U_DWORD)
- 36: SINST3 (VA, U_DWORD)
- 38: SMAXSN (VA, U_DWORD)
- 40: SMAXSN1 (VA, U_DWORD)
- 42: SMAXSN2 (VA, U_DWORD)
- 44: SMAXSN3 (VA, U_DWORD)
- 46: PCOUP (VA, U_DWORD)
- 48: PREF (kVA, U_WORD)
- 49: CCASN (W, U_DWORD)
- 51: NTARF (U_WORD)
- 52: NJOURF (U_WORD)
- 53: NJOURF+1 (U_WORD)
- 54: NGTF (U_QWORD)
- 58: LTARF (U_QWORD)
- 62: PJOURF+1 (U_QWORD)
- 66: PPOINTE (U_QWORD)
- Extras:
  - 2000: CT1 (A, U_WORD)
  - 2001: CT2 (A, U_WORD)
  - 2002: CT3 (A, U_WORD)
  - 2003: CT total apparent power (VA, U_DWORD)
  - 2048: config bitfield (U_WORD) — bit0 rotation, bit1 3/4‑wire
  - 2049: wifi_mode (U_WORD) — 0 off, 1 on, 2 portal

Bus SmartEVSE (SensorBox‑2, adresses hex):
- 0x0100–0x0102: CT1/2/3 (A, U_WORD)
- 0x0110–0x0112: URMS1/2/3 (V, U_WORD)
- 0x0120: total apparent power (VA, U_DWORD)
- 0x0800: config bitfield (rotation, wire_mode)
- 0x0801: wifi_mode (0 off, 1 on, 2 portal)

## Exemple YAML (Zero par défaut, ETH en commentaires)

```yaml
esphome:
  name: smartevse-sensorbox
  platform: ESP32
  board: esp32-s3-devkitc-1

external_components:
  - source: github://tgrauss/esphome-smartevse-sensorbox-linky
    components: [smartevse_sensorbox, modbus_server]
    refresh: 0s

ota:
web_server:
  port: 80

wifi:
  ssid: "YOUR_SSID"
  password: "YOUR_PASSWORD"

# ethernet:
#   type: W5500
#   clk_pin: GPIO13
#   mosi_pin: GPIO11
#   miso_pin: GPIO12
#   cs_pin: GPIO14
#   reset_pin: GPIO9
#   interrupt_pin: GPIO10

i2c:
  # ESP32-S3-Zero
  sda: GPIO6
  scl: GPIO5
  # ESP32-S3-ETH (PoE)
  # sda: GPIO1
  # scl: GPIO2
  scan: true

ads1115:
  - id: ads1115_ct
    address: 0x48

uart:
  # UART TIC Linky
  - id: uart_tic
    rx_pin: GPIO44
    baud_rate: 9600
    parity: EVEN
    data_bits: 7
    stop_bits: 1

teleinfo:
  id: linky_tic
  uart_id: uart_tic
  update_interval: 2s
  historical_mode: false

uart:
  # UART SmartEVSE RS485
  - id: uart_smartevse
    # ESP32-S3-Zero
    tx_pin: GPIO11
    rx_pin: GPIO13
    # ESP32-S3-ETH
    # tx_pin: GPIO40
    # rx_pin: GPIO42
    baud_rate: 9600

uart:
  # UART ESPHome RS485
  - id: uart_esphome
    # ESP32-S3-Zero
    tx_pin: GPIO7
    rx_pin: GPIO9
    # ESP32-S3-ETH
    # tx_pin: GPIO37
    # rx_pin: GPIO39
    baud_rate: 9600

modbus_server:
  - id: mb_smartevse
    uart_id: uart_smartevse
    # ESP32-S3-Zero
    enable_pin: GPIO12
    # ESP32-S3-ETH
    # enable_pin: GPIO41
    address: 10
    baud_rate: 9600

  - id: mb_esphome
    uart_id: uart_esphome
    # ESP32-S3-Zero
    enable_pin: GPIO8
    # ESP32-S3-ETH
    # enable_pin: GPIO38
    address: 100
    baud_rate: 9600

smartevse_sensorbox:
  id: sensorbox
  ads1115_id: ads1115_ct
  teleinfo_id: linky_tic
  modbus_smartevse_id: mb_smartevse
  modbus_esphome_id: mb_esphome
  three_phase: false
  prefer_linky_power: true
  rotation: 0
  wire_mode: 1
  wifi_mode: 1   # mettre 0 si ETH
  # Gains logiciels recommandés (CT 100 A / 50 mA, burden 22 Ω): ~90.9 A/V
  ct_gain_a: 90.9
  ct_gain_b: 90.9
  ct_gain_c: 90.9
  ct_offset_a: 0.0
  ct_offset_b: 0.0
  ct_offset_c: 0.0
  ads_ref_voltage: 1.0
  nominal_voltage: 230.0
  power_factor: 0.95
  fast_interval_ms: 1000
  slow_interval_ms: 2000
