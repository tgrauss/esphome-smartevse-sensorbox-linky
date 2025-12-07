SmartEVSE SensorBox Linky (ESP32-S3)
====================================

Ce projet est une version modifiée de la SensorBox SmartEVSE pour ESPHome.
Il permet de remplacer le matériel de la sensorbox d’origine par :

- 3 sondes de courant SCT013 (100A/50mA) connectées via un ADS1115 :
  - A0 : référence 1 V
  - A1 : Phase 1
  - A2 : Phase 2
  - A3 : Phase 3
- Le compteur Linky connecté en **TIC direct** sur GPIO44 (RX de l’ESP32‑S3) en **7E1** (9600 bauds, parité EVEN, 7 bits, 1 stop), via le composant ESPHome `teleinfo`.
- Un bus Modbus SmartEVSE (où SmartEVSE est maître).
- Un bus Modbus esclave supplémentaire pour communiquer avec d’autres ESPHome.
- Exposition des valeurs à Home Assistant via API ESPHome.

Compatibilité SmartEVSE
-----------------------
Le composant expose **tous les registres Modbus de la Sensorbox‑V2** :
- Adresse esclave : 0x0A, vitesse 9600 bps.
- Input Registers (FC=04) : version, DSMR/TIC info, tensions, courants CT, WiFi, heure/date, IP, MAC.
- Holding Registers (FC=06) :
  - 0x0800 (bits) : rotation champ (bit 0), configuration 3/4 fils (bit 1).
  - 0x0801 : mode WiFi (0 = désactivé, 1 = activé, 2 = portail).

Configuration
-------------
La configuration complète (incluant les GPIO pour ESP32‑S3‑Zero et ESP32‑S3‑ETH/PoE, ainsi que l’intégration du composant `teleinfo`) est disponible dans le fichier **`init.yaml`**.  
Ce fichier regroupe :
- Les définitions des UART (TIC, SmartEVSE, Modbus esclave).  
- Les serveurs Modbus (`mb_smartevse`, `mb_esphome`).  
- Les capteurs TIC (`EAST`, `SINSTS`, `IRMS1‑3`, `URMS1‑3`).  
- Le composant custom `smartevse_sensorbox` qui agrège CT + TIC et expose les registres Modbus SensorBox‑V2.

Licence
-------
Ce projet est basé sur ESPHome et adapté pour SmartEVSE.  
Licence : MIT.
