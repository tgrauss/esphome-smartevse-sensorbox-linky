SmartEVSE SensorBox Linky
=========================

Ce projet est une version modifiée de la SensorBox SmartEVSE pour ESPHome.
Il permet de remplacer le matériel de la sensorbox d’origine par :

- 3 sondes de courant (CT) connectées via un ADS1115 (entrées A1, A2, A3).
- Une entrée de référence 1 V (A0) utilisée pour l’autocalibration des CT.
- Un compteur Linky connecté en Modbus sur GPIO44.

Toutes les informations (CT et Linky) sont :
- Disponibles dans Home Assistant via les sensors ESPHome.
- Exposées en continu sur un bus Modbus esclave, pour être lues par un autre ESPHome master ou par le SmartEVSE.

Fonctionnalités principales
---------------------------
- Lecture des 3 sondes CT avec calibration (gain, offset).
- Autocalibration automatique grâce à la référence 1 V sur A0.
- Lecture des registres Modbus du compteur Linky (puissance, énergie).
- Calcul de la puissance totale (mono ou tri, avec facteur de puissance configurable).
- Préférence configurable : utiliser la puissance Linky ou les CT comme source principale.
- Exposition simultanée des données vers Home Assistant et Modbus esclave.

Exemple de configuration YAML
-----------------------------

esphome:
  name: smartevse-sensorbox-linky
  platform: ESP32
  board: esp32dev

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true

ads1115:
  - id: ads1115_ct
    address: 0x48

sensor:
  - platform: ads1115
    id: ct_a_in
    name: "Raw CT Phase A"
    multiplexer: A1_GND
    gain: 4.096
    update_interval: 1s

  - platform: ads1115
    id: ct_b_in
    name: "Raw CT Phase B"
    multiplexer: A2_GND
    gain: 4.096
    update_interval: 1s

  - platform: ads1115
    id: ct_c_in
    name: "Raw CT Phase C"
    multiplexer: A3_GND
    gain: 4.096
    update_interval: 1s

  - platform: ads1115
    id: ads_ref_in
    name: "ADS Reference 1V"
    multiplexer: A0_GND
    gain: 4.096
    update_interval: 1s

uart:
  id: uart_linky
  tx_pin: GPIO44
  rx_pin: GPIO43
  baud_rate: 9600
  stop_bits: 1

modbus:
  id: modbus_linky
  uart_id: uart_linky

modbus_controller:
  - id: linky_ctrl
    address: 1
    modbus_id: modbus_linky
    update_interval: 5s

sensor:
  - platform: modbus_controller
    modbus_controller_id: linky_ctrl
    id: linky_power_in
    name: "Raw Linky Power"
    address: 0x000C   # à confirmer selon table originale
    register_type: holding
    value_type: U_WORD
    unit_of_measurement: "W"

  - platform: modbus_controller
    modbus_controller_id: linky_ctrl
    id: linky_energy_in
    name: "Raw Linky Energy"
    address: 0x000E   # à confirmer selon table originale
    register_type: holding
    value_type: U_DWORD
    unit_of_measurement: "Wh"

smartevse_sensorbox:
  ct_phase_a: ct_a_in
  ct_phase_b: ct_b_in
  ct_phase_c: ct_c_in
  ads_ref: ads_ref_in
  linky_power: linky_power_in
  linky_energy: linky_energy_in

  ct_gain_a: 1.0
  ct_gain_b: 1.0
  ct_gain_c: 1.0
  ct_offset_a: 0.0
  ct_offset_b: 0.0
  ct_offset_c: 0.0

  ads_ref_voltage: 1.0
  autocalibration: true

  nominal_voltage: 230
  power_factor: 0.95
  three_phase: false
  prefer_linky_power: true
  update_interval: 1s

uart:
  id: uart_modbus_slave
  tx_pin: GPIO18
  rx_pin: GPIO19
  baud_rate: 9600

modbus_server:
  id: mb_slave
  uart_id: uart_modbus_slave
  holding_registers:
    - address: 0      # CT Phase A
      sensor_id: smartevse_sensorbox.ct_phase_a_out
    - address: 1      # CT Phase B
      sensor_id: smartevse_sensorbox.ct_phase_b_out
    - address: 2      # CT Phase C
      sensor_id: smartevse_sensorbox.ct_phase_c_out
    - address: 3      # Total Power
      sensor_id: smartevse_sensorbox.ct_total_power_out
    - address: 4      # Total Current
      sensor_id: smartevse_sensorbox.ct_total_current_out
    - address: 5      # Preference CT/Linky
      sensor_id: smartevse_sensorbox.prefer_ct_out
    - address: 6      # Linky Power
      sensor_id: smartevse_sensorbox.linky_power_out
    - address: 7      # Linky Energy
      sensor_id: smartevse_sensorbox.linky_energy_out

Notes
-----
- Les adresses Modbus ci-dessus doivent être confirmées et alignées avec la table officielle du projet original.
- La préférence CT/Linky est exposée en tant que sensor (prefer_ct_out) :
  - 1 = CT utilisé comme source principale.
  - 0 = Linky utilisé comme source principale.
- L’autocalibration ajuste automatiquement les gains des CT en fonction de la référence 1 V.
- Les valeurs sont rafraîchies toutes les secondes.

Licence
-------
Ce projet est basé sur ESPHome et adapté pour SmartEVSE.
Licence : MIT.
