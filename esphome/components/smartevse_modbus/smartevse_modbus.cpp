#include "smartevse_modbus.h"
#include "esphome/core/log.h"

namespace esphome {
    namespace smartevse_modbus {

        static const char *const TAG = "smartevse_modbus";

        void SmartEVSEModbus::setup() {
            if (this->server_ == nullptr || this->sensorbox_ == nullptr) {
                ESP_LOGE(TAG, "Modbus server or SensorBox not set");
                return;
            }

            switch (this->profile_) {
                case PROFILE_SMARTEVSE_V2:
                    this->setup_smartevse_v2_mapping_();
                    break;
                case PROFILE_LINKY_MODBUS:
                    this->setup_linky_modbus_mapping_();
                    break;
                default:
                    ESP_LOGE(TAG, "Unknown Modbus profile");
                    break;
            }
        }

        void SmartEVSEModbus::dump_config() {
            ESP_LOGCONFIG(TAG, "SmartEVSE Modbus:");
            ESP_LOGCONFIG(TAG, "  Profile: %s", this->profile_ == PROFILE_SMARTEVSE_V2 ? "smartevse_v2" : "linky_modbus");
        }

        // ------------------------------------------------------------
        // Profil SmartEVSE V2 : compatibilité 100% avec SensorBox-V2
        // ------------------------------------------------------------
        void SmartEVSEModbus::setup_smartevse_v2_mapping_() {
            ESP_LOGI(TAG, "Setting up SmartEVSE V2 Modbus mapping");

            // Input Registers (FC=04)
            this->server_->add_input_register(this->sensorbox_->version_out,     0x0000);
            this->server_->add_input_register(this->sensorbox_->dsmr_info_out,   0x0001);
            this->server_->add_input_register(this->sensorbox_->voltage_l1_out,  0x0002);
            this->server_->add_input_register(this->sensorbox_->voltage_l2_out,  0x0003);
            this->server_->add_input_register(this->sensorbox_->voltage_l3_out,  0x0004);
            this->server_->add_input_register(this->sensorbox_->ct_phase_a_out,  0x0005);
            this->server_->add_input_register(this->sensorbox_->ct_phase_b_out,  0x0006);
            this->server_->add_input_register(this->sensorbox_->ct_phase_c_out,  0x0007);
            this->server_->add_input_register(this->sensorbox_->ct_total_current_out, 0x0008);
            this->server_->add_input_register(this->sensorbox_->ct_total_power_out,   0x0009);
            this->server_->add_input_register(this->sensorbox_->linky_power_out,     0x000A);
            this->server_->add_input_register(this->sensorbox_->linky_energy_out,    0x000B);
            this->server_->add_input_register(this->sensorbox_->prefer_ct_out,       0x000C);

            // Holding Registers (FC=06)
            // Créer un capteur virtuel qui combine rotation + wire_mode
            auto *config_reg = new Sensor();
            config_reg->set_state(
                (this->sensorbox_->rotation_out->state ? 0x01 : 0x00) |
                (this->sensorbox_->wire_mode_out->state ? 0x02 : 0x00)
            );

            // Publier ce capteur sur l’adresse 0x0800
            this->server_->add_holding_register(config_reg, 0x0800);
            this->server_->add_holding_register(this->sensorbox_->wifi_mode_out, 0x0801);
        }

        // ------------------------------------------------------------
        // Profil Linky Modbus : mapping complet Teleinfo + CT
        // ------------------------------------------------------------
        void SmartEVSEModbus::setup_linky_modbus_mapping_() {
            ESP_LOGI(TAG, "Setting up Linky Modbus mapping");

            // --- Énergies ---
            server_->add_input_register(sensorbox_->east_out,   00); // EAST
            server_->add_input_register(sensorbox_->eait_out,   02); // EAIT
            server_->add_input_register(sensorbox_->easf01_out, 04);
            server_->add_input_register(sensorbox_->easf02_out, 06);
            server_->add_input_register(sensorbox_->easf01_out, 08);
            server_->add_input_register(sensorbox_->easf02_out, 10);
            server_->add_input_register(sensorbox_->easf01_out, 12);
            server_->add_input_register(sensorbox_->easf02_out, 14);
            server_->add_input_register(sensorbox_->easf03_out, 16);
            server_->add_input_register(sensorbox_->easf04_out, 18);
            server_->add_input_register(sensorbox_->easf05_out, 20);
            server_->add_input_register(sensorbox_->easf06_out, 22);

            // --- Courants et tensions RMS ---
            server_->add_input_register(sensorbox_->irms1_out, 24);
            server_->add_input_register(sensorbox_->irms2_out, 25);
            server_->add_input_register(sensorbox_->irms3_out, 26);
            server_->add_input_register(sensorbox_->urms1_out, 27);
            server_->add_input_register(sensorbox_->urms2_out, 28);
            server_->add_input_register(sensorbox_->urms3_out, 29);

            // --- Puissances instantanées ---
            server_->add_input_register(sensorbox_->sinsts_out,  30);
            server_->add_input_register(sensorbox_->sinsts1_out, 32);
            server_->add_input_register(sensorbox_->sinsts2_out, 34);
            server_->add_input_register(sensorbox_->sinsts3_out, 36);

            // --- Puissances max ---
            server_->add_input_register(sensorbox_->smaxsn_out,  38);
            server_->add_input_register(sensorbox_->smaxsn1_out, 40);
            server_->add_input_register(sensorbox_->smaxsn2_out, 42);
            server_->add_input_register(sensorbox_->smaxsn3_out, 44);

            // --- Contrat / coupure ---
            server_->add_input_register(sensorbox_->pcoup_out, 46);
            server_->add_input_register(sensorbox_->pref_out,  48);

            // --- Asservissement / tarifs ---
            server_->add_input_register(sensorbox_->ccasn_out,    49);
            //server_->add_input_register(sensorbox_->ccasn_1_out,  0x004B);
            server_->add_input_register(sensorbox_->ntarf_out,    51);
            server_->add_input_register(sensorbox_->ltarf_out,    58);
            server_->add_input_register(sensorbox_->njourf_out,   52);
            server_->add_input_register(sensorbox_->njourf1_out,  53);
            server_->add_input_register(sensorbox_->ngtf_out,     54);
            server_->add_input_register(sensorbox_->pjourf1_out,  62);
            server_->add_input_register(sensorbox_->ppointe_out,  66);

            // --- CT totaux ---
            server_->add_input_register(sensorbox_->ct_total_current_out, 70);
            server_->add_input_register(sensorbox_->ct_total_power_out,   72);

            // --- CT individuels par phase ---
            server_->add_input_register(sensorbox_->ct_phase_a_out,       80);
            server_->add_input_register(sensorbox_->ct_phase_b_out,       81);
            server_->add_input_register(sensorbox_->ct_phase_c_out,       82);
            server_->add_input_register(sensorbox_->ct_power_phase_a_out, 84);
            server_->add_input_register(sensorbox_->ct_power_phase_b_out, 85);
            server_->add_input_register(sensorbox_->ct_power_phase_c_out, 86);

            // --- Paramètres SensorBox ---
            server_->add_holding_register(sensorbox_->rotation_out,  90);
            server_->add_holding_register(sensorbox_->wifi_mode_out, 91);
        }

    }  // namespace smartevse_modbus
}  // namespace esphome
