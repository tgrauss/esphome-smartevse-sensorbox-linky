# SmartEVSE SensorBox Linky (ESP32‑S3)

Ce projet est une adaptation de la **SensorBox SmartEVSE** pour ESPHome.  
Il permet de remplacer le matériel d’origine par une solution basée sur ESP32‑S3, avec :

- 3 sondes de courant SCT013 (100A/50mA) connectées via un ADS1115.  
- Le compteur Linky connecté en **TIC direct** (UART gpio44).  
- Deux bus RS485 Modbus :  
  - **Bus SmartEVSE** : compatibilité avec le protocole SensorBox‑V2.  
  - **Bus ESPHome** : mapping complet des étiquettes Teleinfo selon la norme définie dans [linky_modbus](https://github.com/tgrauss/linky_modbus), plus des registres supplémentaires pour exposer les infos SmartEVSE.  

---

## Modules custom

- **smartevse_sensorbox**  
  Agrège les mesures des sondes CT (ADS1115) et du compteur Linky (TIC).  
  Calibre les valeurs et publie les registres internes (courants, tensions, puissance, énergie, etc.).

- **smartevse_modbus**  
  Fait le mapping des registres internes de la `SensorBox` vers un serveur Modbus.  
  Deux profils disponibles :  
  - `smartevse_v2` : compatibilité avec SmartEVSE (SensorBox‑V2).  
  - `linky_modbus` : compatibilité avec le mapping complet des étiquettes Teleinfo (projet linky_modbus), plus les CT et paramètres SensorBox.

- **modbus_server**  
  Composant custom qui implémente un serveur Modbus esclave dans ESPHome.  
  Utilisé par `smartevse_modbus` pour publier les registres sur un bus RS485.

---

## Compatibilité SmartEVSE (profil `smartevse_v2`)

- Adresse esclave : 10, vitesse 9600 bps.  
- Input Registers (FC=04) : version, DSMR/TIC info, tensions, courants CT, WiFi, heure/date, IP, MAC.  
- Holding Registers (FC=06) :  
  - 0x0800 : registre de configuration (bitfield)  
    - bit 0 = rotation champ  
    - bit 1 = configuration 3/4 fils  
  - 0x0801 : mode WiFi (0 = désactivé, 1 = activé, 2 = portail).  

👉 Les adresses sont indiquées en **hexadécimal** pour correspondre à la documentation SmartEVSE.

---

## Compatibilité Linky Modbus (profil `linky_modbus`)

Le deuxième bus RS485 expose toutes les étiquettes Teleinfo du compteur Linky.  
👉 Les adresses sont indiquées en **décimal** pour correspondre à la norme que j’ai définie dans le projet [linky_modbus](https://github.com/tgrauss/linky_modbus).

⚠️ Attention : le compteur Linky ne fournit pas directement de registres Modbus.  
Ce mapping est une convention propre à mes projets, qui permet d’exposer les étiquettes TIC sous forme de registres Modbus.

---

## Registres supplémentaires (après les adresses Linky)

- 80 : Courant CT1 (U_WORD, A)  
- 81 : Courant CT2 (U_WORD, A)  
- 82 : Courant CT3 (U_WORD, A)  
- 84 : Puissance CT1 (U_WORD, W)  
- 85 : Puissance CT2 (U_WORD, W)  
- 86 : Puissance CT3 (U_WORD, W)  
- 70 : Courant total CT
- 72 : Puissance totale mesurée par CT (U_DWORD, VA)  
- 90 : Registre de configuration (bitfield)  
  - bit 0 = rotation champ  
  - bit 1 = configuration 3/4 fils  
- 91 : Mode WiFi (U_WORD)  
  - 0 = désactivé  
  - 1 = activé  
  - 2 = portail  

Ces adresses sont en **décimal**, pour rester cohérentes avec le mapping `linky_modbus`.

---

## Usage du serveur Modbus

Le serveur Modbus intégré sert à deux choses :
- Assurer la compatibilité avec SmartEVSE (profil `smartevse_v2`).  
- Permettre à un **autre ESPHome** ou microcontrôleur de se connecter en RS485 et de lire les registres exposés (profil `linky_modbus`).  

Ainsi, un ESPHome peut agir comme “maître” Modbus et interroger un autre ESPHome configuré comme “esclave” Modbus, ce qui facilite l’interconnexion entre plusieurs projets.

---

## Notes importantes

- Les adresses sont en **hex pour SmartEVSE** et en **décimal pour Linky/ESPHome**.  
- Le registre 90 est un **bitfield** (rotation + wire_mode).  
- Les champs texte (NGTF, LTARF, PJOURF+1, PPOINTE) sont des chaînes ASCII limitées à **8 caractères max** (4 registres Modbus chacun).  
- Le profil `linky_modbus` expose toutes les étiquettes Teleinfo + CT + registres SmartEVSE additionnels.  
- Le profil `smartevse_v2` reste strictement identique à la SensorBox‑V2 originale.

---

👉 Ce README est volontairement simplifié. Pour le détail complet des registres TIC Linky, se référer directement au [README du projet linky_modbus](https://github.com/tgrauss/linky_modbus).
