# SmartEVSE SensorBox Linky (ESP32-S3)

Ce projet est une adaptation de la SensorBox SmartEVSE pour ESPHome.  
Il permet de remplacer le matériel de la SensorBox d’origine par une solution basée sur ESP32‑S3, avec :

- 3 sondes de courant SCT013 (100A/50mA) connectées via un ADS1115.
- Le compteur Linky connecté en **TIC direct** sur GPIO44 (UART 7E1).
- Un bus RS485 Modbus vers SmartEVSE (SmartEVSE maître).
- Un bus RS485 Modbus supplémentaire pour exposer les mêmes registres à un autre maître ESPHome.
- Exposition des valeurs à Home Assistant via API ESPHome.

---

## 📦 Modules custom

Le projet s’appuie sur plusieurs composants custom ESPHome :

- **`smartevse_sensorbox`**  
  Agrège les mesures des sondes CT (ADS1115) et du compteur Linky (TIC).  
  Calibre les valeurs et publie les registres internes (courants, tensions, puissance, énergie, etc.).

- **`smartevse_modbus`**  
  Fait le mapping des registres internes de la `SensorBox` vers un serveur Modbus.  
  Permet d’exposer les registres SensorBox‑V2 attendus par SmartEVSE ou par un autre maître Modbus.

- **`modbus_server`** (issu du projet [arpiecodes/esphome-modbus-server](https://github.com/arpiecodes/esphome-modbus-server))  
  Composant custom qui implémente un serveur Modbus esclave dans ESPHome.  
  Utilisé par `smartevse_modbus` pour publier les registres sur un bus RS485.

---

## 🔌 Compatibilité SmartEVSE

Le composant expose **tous les registres Modbus de la SensorBox‑V2** :

- Adresse esclave : `0x0A`, vitesse `9600 bps`.
- Input Registers (FC=04) : version, DSMR/TIC info, tensions, courants CT, WiFi, heure/date, IP, MAC.
- Holding Registers (FC=06) :
  - `0x0800` (bits) : rotation champ (bit 0), configuration 3/4 fils (bit 1).
  - `0x0801` : mode WiFi (0 = désactivé, 1 = activé, 2 = portail).

---

## ⚙️ Configuration

La configuration complète (incluant les GPIO pour ESP32‑S3‑Zero et ESP32‑S3‑ETH/PoE, ainsi que l’intégration des modules custom) est disponible dans le fichier **`init.yaml`**.

Ce fichier regroupe :
- Les définitions des UART (TIC, SmartEVSE, Modbus esclave).  
- Les serveurs Modbus (`mb_smartevse`, `mb_esphome`).  
- Les capteurs TIC (`EAST`, `SINSTS`, `IRMS1‑3`, `URMS1‑3`).  
- Le composant `smartevse_sensorbox` (CT + TIC).  
- Le composant `smartevse_modbus` (mapping vers Modbus).  

---

## 📜 Licence

Ce projet est basé sur ESPHome et adapté pour SmartEVSE.  
Licence : MIT.
