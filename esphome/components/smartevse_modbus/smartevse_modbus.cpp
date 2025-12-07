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
            this->server_->add_holding_register(this->sensorbox_->rotation_out,  0x0800);
            this->server_->add_holding_register(this->sensorbox_->wire_mode_out, 0x0800);
            this->server_->add_holding_register(this->sensorbox_->wifi_mode_out, 0x0801);
        }

        // ------------------------------------------------------------
        // Profil Linky Modbus : mapping complet Teleinfo + CT
        // ------------------------------------------------------------
        void SmartEVSEModbus::setup_linky_modbus_mapping_() {
            ESP_LOGI(TAG, "Setting up Linky Modbus mapping");

            // --- Énergies ---
            server_->add_input_register(sensorbox_->east_out,   0x0000); // EAST
            server_->add_input_register(sensorbox_->eait_out,   0x0002); // EAIT
            server_->add_input_register(sensorbox_->easf01_out, 0x0004);
            server_->add_input_register(sensorbox_->easf02_out, 0x0006);
            server_->add_input_register(sensorbox_->easf03_out, 0x0008);
            server_->add_input_register(sensorbox_->easf04_out, 0x000A);
            server_->add_input_register(sensorbox_->easf05_out, 0x000C);
            server_->add_input_register(sensorbox_->easf06_out, 0x000E);
            server_->add_input_register(sensorbox_->easf07_out, 0x0010);
            server_->add_input_register(sensorbox_->easf08_out, 0x0012);
            server_->add_input_register(sensorbox_->easf09_out, 0x0014);
            server_->add_input_register(sensorbox_->easf10_out, 0x0016);

            // --- Courants et tensions RMS ---
            server_->add_input_register(sensorbox_->irms1_out, 0x0018);
            server_->add_input_register(sensorbox_->irms2_out, 0x0019);
            server_->add_input_register(sensorbox_->irms3_out, 0x001A);
            server_->add_input_register(sensorbox_->urms1_out, 0x001B);
            server_->add_input_register(sensorbox_->urms2_out, 0x001C);
            server_->add_input_register(sensorbox_->urms3_out, 0x001D);

            // --- Puissances instantanées ---
            server_->add_input_register(sensorbox_->sinsts_out,  0x0030);
            server_->add_input_register(sensorbox_->sinsts1_out, 0x0032);
            server_->add_input_register(sensorbox_->sinsts2_out, 0x0034);
            server_->add_input_register(sensorbox_->sinsts3_out, 0x0036);

            // --- Puissances max ---
            server_->add_input_register(sensorbox_->smaxsn_out,  0x0038);
            server_->add_input_register(sensorbox_->smaxsn1_out, 0x003A);
            server_->add_input_register(sensorbox_->smaxsn2_out, 0x003C);
            server_->add_input_register(sensorbox_->smaxsn3_out, 0x003E);

            // --- Contrat / coupure ---
            server_->add_input_register(sensorbox_->pcoup_out, 0x0046);
            server_->add_input_register(sensorbox_->pref_out,  0x0048);

            // --- Asservissement / tarifs ---
            server_->add_input_register(sensorbox_->ccasn_out,    0x004A);
            server_->add_input_register(sensorbox_->ccasn_1_out,  0x004B);
            server_->add_input_register(sensorbox_->ntarf_out,    0x004C);
            server_->add_input_register(sensorbox_->ltarf_out,    0x004D);
            server_->add_input_register(sensorbox_->njourf1_out,  0x004E);
            server_->add_input_register(sensorbox_->ngtf_out,     0x004F);
            server_->add_input_register(sensorbox_->pjourf1_out,  0x0050);
            server_->add_input_register(sensorbox_->ppointe_out,  0x0052);

            // --- CT totaux ---
            server_->add_input_register(sensorbox_->ct_total_current_out, 0x0058);
            server_->add_input_register(sensorbox_->ct_total_power_out,   0x005A);

            // --- CT individuels par phase ---
            server_->add_input_register(sensorbox_->ct_phase_a_out,       0x0060);
            server_->add_input_register(sensorbox_->ct_phase_b_out,       0x0061);
            server_->add_input_register(sensorbox_->ct_phase_c_out,       0x0062);
            server_->add_input_register(sensorbox_->ct_power_phase_a_out, 0x0064);
            server_->add_input_register(sensorbox_->ct_power_phase_b_out, 0x0065);
            server_->add_input_register(sensorbox_->ct_power_phase_c_out, 0x0066);

            // --- Paramètres SensorBox ---
            server_->add_holding_register(sensorbox_->rotation_out,  0x005C);
            server_->add_holding_register(sensorbox_->wifi_mode_out, 0x005D);
        }

    }  // namespace smartevse_modbus
}  // namespace esphome
