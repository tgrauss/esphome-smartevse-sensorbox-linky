SmartEVSE SensorBox Linky (ESP32-S3)
====================================

Ce projet est une version modifiée de la SensorBox SmartEVSE pour ESPHome.
Il permet de remplacer le matériel de la sensorbox d’origine par :

- 3 sondes de courant SCT013 (100A/50mA) connectées via un ADS1115 :
  - A0 : référence 1 V
  - A1 : Phase 1
  - A2 : Phase 2
  - A3 : Phase 3
- Un compteur Linky connecté en Modbus sur GPIO44 (RX de l’ESP32-S3).
- Un bus Modbus SmartEVSE (où SmartEVSE est maître).
- Un bus Modbus esclave supplémentaire pour communiquer avec d’autres ESPHome.
- Exposition des valeurs à Home Assistant via API ESPHome.

Compatibilité SmartEVSE
-----------------------
Le composant expose **tous les registres Modbus de la Sensorbox‑V2** :
- Adresse esclave : 0x0A, vitesse 9600 bps.
- Input Registers (FC=04) : version, DSMR info, tensions, courants CT, WiFi, heure/date, IP, MAC.
- Holding Registers (FC=06) : rotation champ / config 3‑4 fils, mode WiFi.

---

## Exemple YAML pour ESP32‑S3‑Zero

```yaml
esphome:
  name: smartevse-sensorbox-zero
  platform: ESP32
  board: esp32-s3-devkitc-1

# I²C pour ADS1115
i2c:
  sda: GPIO6
  scl: GPIO5
  scan: true

ads1115:
  - id: ads1115_ct
    address: 0x48

# UART pour Linky (maître Modbus, lecture sur GPIO44)
uart:
  id: uart_linky
  rx_pin: GPIO44
  baud_rate: 9600
  stop_bits: 1

modbus:
  id: modbus_linky
  uart_id: uart_linky

modbus_controller:
  - id: linky_ctrl
    address: 1
    modbus_id: modbus_linky
    update_interval: 2s

# UART pour SmartEVSE (esclave Modbus)
uart:
  id: uart_smartevse
  tx_pin: GPIO11
  rx_pin: GPIO13
  baud_rate: 9600

modbus_server:
  id: mb_smartevse
  uart_id: uart_smartevse
  enable_pin: GPIO12

# UART pour Modbus esclave (autres ESPHome)
uart:
  id: uart_esphome
  tx_pin: GPIO7
  rx_pin: GPIO9
  baud_rate: 9600

modbus_server:
  id: mb_esphome
  uart_id: uart_esphome
  enable_pin: GPIO8

smartevse_sensorbox:
  ct_phase_a: ct_a_in
  ct_phase_b: ct_b_in
  ct_phase_c: ct_c_in
  ads_ref: ads_ref_in
  linky_power: linky_power_in
  linky_energy: linky_energy_in
  nominal_voltage: 230
  power_factor: 0.95
  three_phase: true
  prefer_linky_power: true
  autocalibration: true
  rotation: 0
  wire_mode: 1
  wifi_enabled: true
  update_interval: 1s
  
```
---

## Exemple YAML pour ESP32‑S3‑ETH (POE Ethernet)

```yaml
esphome:
  name: smartevse-sensorbox-eth
  platform: ESP32
  board: esp32-s3-devkitc-1

# I²C pour ADS1115
i2c:
  sda: GPIO1
  scl: GPIO2
  scan: true

ads1115:
  - id: ads1115_ct
    address: 0x48

# UART pour Linky (maître Modbus, lecture sur GPIO44)
uart:
  id: uart_linky
  rx_pin: GPIO44
  baud_rate: 9600
  stop_bits: 1

modbus:
  id: modbus_linky
  uart_id: uart_linky

modbus_controller:
  - id: linky_ctrl
    address: 1
    modbus_id: modbus_linky
    update_interval: 2s

# UART pour SmartEVSE (esclave Modbus)
uart:
  id: uart_smartevse
  tx_pin: GPIO40
  rx_pin: GPIO42
  baud_rate: 9600

modbus_server:
  id: mb_smartevse
  uart_id: uart_smartevse
  enable_pin: GPIO41

# UART pour Modbus esclave (autres ESPHome)
uart:
  id: uart_esphome
  tx_pin: GPIO37
  rx_pin: GPIO39
  baud_rate: 9600

modbus_server:
  id: mb_esphome
  uart_id: uart_esphome
  enable_pin: GPIO38

smartevse_sensorbox:
  ct_phase_a: ct_a_in
  ct_phase_b: ct_b_in
  ct_phase_c: ct_c_in
  ads_ref: ads_ref_in
  linky_power: linky_power_in
  linky_energy: linky_energy_in
  nominal_voltage: 230
  power_factor: 0.95
  three_phase: true
  prefer_linky_power: true
  autocalibration: true
  rotation: 0
  wire_mode: 1
  wifi_enabled: false   # Désactiver le WiFi si Ethernet PoE est utilisé
  update_interval: 1s
```

---

Notes
-----
- Les pins sont adaptés aux deux variantes matérielles : **ESP32‑S3‑Zero** et **ESP32‑S3‑ETH**.  
- Sur le bus SmartEVSE, c’est bien **SmartEVSE qui est maître**, ton ESP32‑S3 est esclave.  
- Sur le bus Linky, ton ESP32‑S3 est maître pour interroger le compteur.  
- Sur le bus esclave pour autres ESPHome, ton ESP32‑S3 est esclave.  
- Les CT calibrés sont exposés sur 0x000E, 0x0010, 0x0012.  
- Les autres registres (version, DSMR, tensions, WiFi, etc.) sont également disponibles.  
- Les paramètres `rotation`, `wire_mode` et `wifi_enabled` sont configurables via YAML.  
- Par défaut : **rotation droite (0)**, **3 fils (1)** pour SCT013, et **WiFi activé**.

Licence
-------
Ce projet est basé sur ESPHome et adapté pour SmartEVSE.  
Licence : MIT.
