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
| EAST      | 0  | U_DWORD |
| IRMS1     | 24  | U_WORD  |
| IRMS2     | 25  | U_WORD  |
| IRMS3     | 26  | U_WORD  |
| URMS1     | 27  | U_WORD  |
| URMS2     | 28  | U_WORD  |
| URMS3     | 29  | U_WORD  |
| SINSTS    | 30  | U_DWORD |

Les autres étiquettes (EASFxx, SMAXSNx, PREF, CCASN, NTARF, NJOURF, NGTF, LTARF, etc.) sont également mappées selon le tableau du projet linky_modbus.  
En plus, les valeurs des sondes CT et les paramètres SensorBox (rotation, wire_mode, wifi_mode) sont exposés.

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

La configuration complète est disponible dans `examples/init.yaml`.
Elle regroupe :
- UART TIC, UART SmartEVSE, UART Modbus
- Deux serveurs Modbus (`mb_smartevse`, `mb_esphome`)
- Capteurs TIC (EAST, EAIT, EASFxx, SINSTS, IRMS1‑3, URMS1‑3, SMAXSNx, PREF, PCOUP, NTARF, LTARF, NJOURF, NGTF, PJOURF+1, PPOINTE, etc.)
- Composant `smartevse_sensorbox` (CT + TIC)
- Composant `smartevse_modbus` (mapping Modbus)

---

### Exemple minimal (à placer dans votre configuration ESPHome) :

```yaml
smartevse_modbus:
  id: mb_esphome
  modbus_server_id: mb_server_esphome
  sensorbox_id: sensorbox
  profile: linky_modbus

Paramètre profile :
- smartevse_v2 : compatibilité 100% avec SmartEVSE (SensorBox‑V2) sur son bus dédié
- linky_modbus : expose toutes les étiquettes Teleinfo + sondes CT individuelles sur le bus esclave destiné aux autres maîtres ESPHome/tiers
```

---

## 🧩 Profils Modbus

- Profil smartevse_v2 :
  - Mappage strictement identique à la SensorBox‑V2
  - Aucune extension, aucune adresse modifiée
  - À utiliser sur le bus connecté au SmartEVSE (maître SmartEVSE)

- Profil linky_modbus :
  - Mappage complet des étiquettes Teleinfo et des CT (totaux + phases)
  - Destiné au bus esclave pour autres maîtres (ESPHome/PLC/SCADA)

---

## 🏠 Intégration Home Assistant

Les capteurs exposés par `smartevse_sensorbox` sont publiés vers Home Assistant via l’API ESPHome.
Le bus linky_modbus permet également à d’autres maîtres de lire ces registres via Modbus RTU.

Exemple de template simple :

sensor:
  - platform: template
    sensors:
      puissance_totale_ct:
        friendly_name: "Puissance totale CT"
        unit_of_measurement: "W"
        value_template: "{{ states('sensor.smartevse_sensorbox_ct_total_power') }}"

---

## 📊 Dashboard Lovelace

Un exemple de configuration Lovelace est disponible dans `examples/lovelace_dashboard.yaml`.
Il affiche :
- Vue Énergie & Puissance : entités + jauge + graphe historique
- Vue Courants : entités + graphes par phase
- Vue Tensions : entités + graphes par phase

---

## 📂 Exemples

Dans `examples/` :
- `init.yaml` : configuration ESPHome complète avec les deux profils Modbus (`smartevse_v2`, `linky_modbus`)
- `homeassistant_entities.yaml` : exemples d’entités HA basées sur les capteurs ESPHome
- `lovelace_dashboard.yaml` : dashboard Lovelace pour visualiser courants, tensions, puissances

---

## 📜 Licence

Projet basé sur ESPHome, adapté pour SmartEVSE. Licence : MIT.
