#include "smartevse_modbus.h"
#include "esphome/core/log.h"

namespace esphome {
    namespace smartevse_modbus {

        static const char *const TAG = "smartevse_modbus";

        void SmartEVSEModbusServer::setup() {
            ESP_LOGI(TAG, "SmartEVSE Modbus Server setup");

            if (!server_) {
                ESP_LOGE(TAG, "No Modbus server linked!");
                return;
            }

            // Choix du profil
            if (profile_ == Profile::SMART_EVSE_V2) {
                setup_smartevse_v2_mapping_();
            } else {
                setup_linky_modbus_mapping_();
            }
        }

        void SmartEVSEModbusServer::loop() {
            // Les valeurs sont publiées par les sensors et exposées automatiquement
        }

        // -------------------------------
        // Mapping SensorBox-V2 (SmartEVSE)
        // -------------------------------
        void SmartEVSEModbusServer::setup_smartevse_v2_mapping_() {
            ESP_LOGI(TAG, "Mapping SmartEVSE V2 registers");

            // Input Registers (0x0000–0x001C)
            server_->add_input_register(version_,     0x0000);
            server_->add_input_register(dsmr_info_,   0x0001);
            server_->add_input_register(voltage_l1_,  0x0002);
            server_->add_input_register(voltage_l2_,  0x0004);
            server_->add_input_register(voltage_l3_,  0x0006);
            server_->add_input_register(p1_current_l1_, 0x0008);
            server_->add_input_register(p1_current_l2_, 0x000A);
            server_->add_input_register(p1_current_l3_, 0x000C);
            server_->add_input_register(ct_phase_a_,  0x000E);
            server_->add_input_register(ct_phase_b_,  0x0010);
            server_->add_input_register(ct_phase_c_,  0x0012);
            server_->add_input_register(wifi_status_, 0x0014);
            server_->add_input_register(time_hm_,     0x0015);
            server_->add_input_register(time_md_,     0x0016);
            server_->add_input_register(time_yw_,     0x0017);
            server_->add_input_register(ip_,          0x0018);
            server_->add_input_register(mac_,         0x001A);
            server_->add_input_register(portal_pwd_,  0x001C);

            // Holding Registers (0x0800–0x0801)
            server_->add_holding_register(rotation_,  0x0800);
            server_->add_holding_register(wifi_mode_, 0x0801);
        }

        // -------------------------------
        // Mapping Linky Modbus (profil esclave ESPHome)
        // -------------------------------
        void SmartEVSEModbusServer::setup_linky_modbus_mapping_() {
            ESP_LOGI(TAG, "Mapping Linky Modbus registers");

            // Exemple de mapping conforme à ton projet linky_modbus
            // Energie soutirée (EAST) U_DWORD à 0x0000
            server_->add_input_register(linky_energy_, 0x0000);

            // Puissance apparente totale (SINSTS) U_DWORD à 0x0030
            server_->add_input_register(linky_power_, 0x0030);

            // Courants IRMS1–3 U_WORD à 0x0018–0x001A
            server_->add_input_register(p1_current_l1_, 0x0018);
            server_->add_input_register(p1_current_l2_, 0x0019);
            server_->add_input_register(p1_current_l3_, 0x001A);

            // Tensions URMS1–3 U_WORD à 0x001B–0x001D
            server_->add_input_register(voltage_l1_, 0x001B);
            server_->add_input_register(voltage_l2_, 0x001C);
            server_->add_input_register(voltage_l3_, 0x001D);

            // CT sensors (choisir une zone libre, ex. 0x0050+)
            server_->add_input_register(ct_total_current_, 0x0050);
            server_->add_input_register(ct_total_power_,   0x0052);
            server_->add_input_register(ct_phase_a_,       0x0054);
            server_->add_input_register(ct_phase_b_,       0x0055);
            server_->add_input_register(ct_phase_c_,       0x0056);

            // Paramètres SensorBox (Holding Registers)
            server_->add_holding_register(rotation_,  0x0058);
            server_->add_holding_register(wifi_mode_, 0x0059);
            // wire_mode peut être combiné ou ajouté séparément
        }

    }  // namespace smartevse_modbus
}  // namespace esphome
