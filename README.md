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

La configuration complète est disponible dans `examples/init.yaml`.
Elle regroupe :
- UART TIC, UART SmartEVSE, UART Modbus
- Deux serveurs Modbus (`mb_smartevse`, `mb_esphome`)
- Capteurs TIC (EAST, EAIT, EASFxx, SINSTS, IRMS1‑3, URMS1‑3, SMAXSNx, PREF, PCOUP, NTARF, LTARF, NJOURF, NGTF, PJOURF+1, PPOINTE, etc.)
- Composant `smartevse_sensorbox` (CT + TIC)
- Composant `smartevse_modbus` (mapping Modbus)

Exemple minimal (à placer dans votre configuration ESPHome) :

smartevse_modbus:
  id: mb_esphome
  modbus_server_id: mb_server_esphome
  sensorbox_id: sensorbox
  profile: linky_modbus

Paramètre profile :
- smartevse_v2 : compatibilité 100% avec SmartEVSE (SensorBox‑V2) sur son bus dédié
- linky_modbus : expose toutes les étiquettes Teleinfo + sondes CT individuelles sur le bus esclave destiné aux autres maîtres ESPHome/tiers

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

## 🗂️ Registres Modbus (profil linky_modbus)

Énergies (U_DWORD, Wh)
- EAST : 0x0000 — Énergie soutirée totale
- EAIT : 0x0002 — Énergie injectée totale
- EASF01 : 0x0004
- EASF02 : 0x0006
- EASF03 : 0x0008
- EASF04 : 0x000A
- EASF05 : 0x000C
- EASF06 : 0x000E
- EASF07 : 0x0010
- EASF08 : 0x0012
- EASF09 : 0x0014
- EASF10 : 0x0016

Courants et tensions RMS (U_WORD)
- IRMS1 : 0x0018
- IRMS2 : 0x0019
- IRMS3 : 0x001A
- URMS1 : 0x001B
- URMS2 : 0x001C
- URMS3 : 0x001D

Puissances instantanées et maxima (U_DWORD, VA)
- SINSTS  : 0x0030 — Puissance apparente totale
- SINSTS1 : 0x0032 — L1
- SINSTS2 : 0x0034 — L2
- SINSTS3 : 0x0036 — L3
- SMAXSN  : 0x0038 — Max jour total
- SMAXSN1 : 0x003A — Max jour L1
- SMAXSN2 : 0x003C — Max jour L2
- SMAXSN3 : 0x003E — Max jour L3

Contrat / coupure / tarification
- PCOUP  : 0x0046 (U_DWORD) — Puissance de coupure
- PREF   : 0x0048 (U_WORD)  — Puissance de référence
- CCASN  : 0x004A (U_WORD)  — Index asservi courant
- CCASN-1: 0x004B (U_WORD)  — Index asservi précédent
- NTARF  : 0x004C (U_WORD)  — N° de tarif en cours
- LTARF  : 0x004D (U_WORD/U_QWORD selon implémentation) — Libellé tarif
- NJOURF+1 : 0x004E (U_WORD) — Numéro de jour suivant
- NGTF   : 0x004F (U_WORD/U_QWORD) — Type de contrat / groupe tarifaire
- PJOURF+1 : 0x0050 (U_QWORD) — Profil tarifaire prévu demain
- PPOINTE  : 0x0052 (U_QWORD) — Indication jour de pointe demain

CT totaux (capteurs sondes de courant côté SensorBox)
- CT total courant : 0x0058 (U_WORD, A)
- CT total puissance : 0x005A (U_DWORD, W/VA)

CT individuels par phase
- CT courant phase A : 0x0060 (U_WORD, A)
- CT courant phase B : 0x0061 (U_WORD, A)
- CT courant phase C : 0x0062 (U_WORD, A)
- CT puissance phase A : 0x0064 (U_DWORD, W/VA) — si disponible
- CT puissance phase B : 0x0065 (U_DWORD, W/VA) — si disponible
- CT puissance phase C : 0x0066 (U_DWORD, W/VA) — si disponible

Paramètres SensorBox (Holding Registers)
- rotation : 0x005C (U_WORD) — rotation champ
- wifi_mode : 0x005D (U_WORD) — mode WiFi

Remarques :
- Les types U_WORD/U_DWORD/U_QWORD dépendent de votre implémentation du `ModbusServer` (endianness, largeur). Conservez la cohérence avec `linky_modbus`.
- Le profil smartevse_v2 ne doit pas être modifié (adresses/types comme l’original).
- Le profil linky_modbus ne chevauche pas les adresses réservées au profil smartevse_v2.

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
