# SmartEVSE SensorBox Linky (ESP32-S3)

Ce projet est une adaptation de la **SensorBox SmartEVSE** pour ESPHome.  
Il permet de remplacer le matériel d’origine par une solution basée sur ESP32‑S3, avec :

- 3 sondes de courant SCT013 (100A/50mA) connectées via un ADS1115.
- Le compteur Linky connecté en **TIC direct** (UART 7E1).
- Deux bus RS485 Modbus :
  - **Bus SmartEVSE** : compatibilité avec le protocole SensorBox‑V2.
  - **Bus ESPHome** : mapping complet des étiquettes Teleinfo selon [linky_modbus](https://github.com/tgrauss/linky_modbus).
- Exposition des valeurs à Home Assistant via API ESPHome.

---

## 📦 Modules custom

Le projet s’appuie sur plusieurs composants custom ESPHome :

- **`smartevse_sensorbox`**  
  Agrège les mesures des sondes CT (ADS1115) et du compteur Linky (TIC).  
  Calibre les valeurs et publie les registres internes (courants, tensions, puissance, énergie, etc.).  
  Configurable via YAML (gains, offsets, tension nominale, facteur de puissance, etc.).

- **`smartevse_modbus`**  
  Fait le mapping des registres internes de la `SensorBox` vers un serveur Modbus.  
  Deux profils disponibles :
  - `smartevse_v2` : compatibilité avec SmartEVSE (SensorBox‑V2).
  - `linky_modbus` : compatibilité avec le mapping complet des étiquettes Teleinfo (projet linky_modbus), plus les CT et paramètres SensorBox.

- **`modbus_server`**  
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

## 📊 Compatibilité Linky Modbus

Le deuxième bus RS485 expose toutes les étiquettes Teleinfo selon le mapping [linky_modbus](https://github.com/tgrauss/linky_modbus).  
Exemples d’adresses :

| Étiquette | Adresse | Type   |
|-----------|---------|--------|
| EAST      | 0x0000  | U_DWORD |
| IRMS1     | 0x0018  | U_WORD  |
| IRMS2     | 0x0019  | U_WORD  |
| IRMS3     | 0x001A  | U_WORD  |
| URMS1     | 0x001B  | U_WORD  |
| URMS2     | 0x001C  | U_WORD  |
| URMS3     | 0x001D  | U_WORD  |
| SINSTS    | 0x0030  | U_DWORD |

Les autres étiquettes (EASFxx, SMAXSNx, PREF, CCASN, NTARF, NJOURF, NGTF, LTARF, etc.) sont également mappées selon le tableau du projet linky_modbus.  
En plus, les valeurs des sondes CT et les paramètres SensorBox (rotation, wire_mode, wifi_mode) sont exposés dans une zone libre (ex. 0x0050+).

---

## 🏗️ Architecture

```text
                ┌───────────────────────┐
                │   SmartEVSESensorBox  │
                │  (ESP32-S3 + ADS1115) │
                │   CT + TIC Linky      │
                └──────────┬────────────┘
                           │
                           │ valeurs internes
                           ▼
                ┌───────────────────────┐
                │   smartevse_sensorbox │
                │   (composant custom)  │
                └──────────┬────────────┘
                           │
          ┌────────────────┴────────────────┐
          │                                 │
          ▼                                 ▼
┌───────────────────────┐         ┌───────────────────────┐
│   smartevse_modbus    │         │   smartevse_modbus    │
│   profil smartevse_v2 │         │  profil linky_modbus  │
└──────────┬────────────┘         └──────────┬────────────┘
           │                                 │
           ▼                                 ▼
┌───────────────────────┐         ┌───────────────────────┐
│   modbus_server       │         │   modbus_server       │
│   (mb_smartevse)      │         │   (mb_esphome)        │
└──────────┬────────────┘         └──────────┬────────────┘
           │                                 │
           ▼                                 ▼
   RS485 Bus vers SmartEVSE          RS485 Bus vers autre
   (maître SmartEVSE)                maître ESPHome (linky_modbus)
```

---

## ⚙️ Configuration YAML

La configuration complète est disponible dans **`examples/init.yaml`**.  
Elle regroupe :
- Les définitions des UART (TIC, SmartEVSE, Modbus esclave).  
- Les serveurs Modbus (`mb_smartevse`, `mb_esphome`).  
- Les capteurs TIC (`EAST`, `SINSTS`, `IRMS1‑3`, `URMS1‑3`).  
- Le composant `smartevse_sensorbox` (CT + TIC).  
- Le composant `smartevse_modbus` (mapping vers Modbus, profil `smartevse_v2` ou `linky_modbus`).  

---

## 🏠 Intégration Home Assistant

Tous les capteurs créés par `smartevse_sensorbox` sont automatiquement publiés vers Home Assistant via l’API ESPHome.  
Cela inclut :
- Les étiquettes Teleinfo (puissance, énergie, courants, tensions, etc.).  
- Les valeurs CT (courants, puissance totale).  
- Les registres supplémentaires (rotation, wifi_mode, etc.).  

Un exemple de configuration Home Assistant est disponible dans **`examples/homeassistant_entities.yaml`**.  
Il montre comment les entités exposées par ESPHome apparaissent et peuvent être utilisées dans HA.

---

## 📊 Dashboard Lovelace

Un exemple de configuration Lovelace est disponible dans **`examples/lovelace_dashboard.yaml`**.  
Il permet de visualiser les courants, tensions et puissances sous forme de cartes et de graphes dans Home Assistant.  

---

## 📂 Exemples

Des fichiers d’exemple sont fournis dans le dossier `examples/` :

- **`init.yaml`** : configuration ESPHome complète avec les deux profils Modbus (`smartevse_v2` et `linky_modbus`).  
- **`homeassistant_entities.yaml`** : exemple de configuration Home Assistant montrant comment les capteurs exposés par ESPHome apparaissent et peuvent être utilisés dans HA.  
- **`lovelace_dashboard.yaml`** : exemple de configuration Lovelace (Home Assistant Dashboard) pour visualiser les courants, tensions et puissances sous forme de cartes et de graphes.  

---

## 📜 Licence

Ce projet est basé sur ESPHome et adapté pour SmartEVSE.  
Licence : MIT.


