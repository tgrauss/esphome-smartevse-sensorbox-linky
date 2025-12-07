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

            // Déclaration des registres Input (FC=04)
            server_->add_input_register(version_, 0x0000);
            server_->add_input_register(dsmr_info_, 0x0001);
            server_->add_input_register(voltage_l1_, 0x0002);
            server_->add_input_register(voltage_l2_, 0x0004);
            server_->add_input_register(voltage_l3_, 0x0006);
            server_->add_input_register(p1_current_l1_, 0x0008);
            server_->add_input_register(p1_current_l2_, 0x000A);
            server_->add_input_register(p1_current_l3_, 0x000C);
            server_->add_input_register(ct_phase_a_, 0x000E);
            server_->add_input_register(ct_phase_b_, 0x0010);
            server_->add_input_register(ct_phase_c_, 0x0012);
            server_->add_input_register(wifi_status_, 0x0014);
            server_->add_input_register(time_hm_, 0x0015);
            server_->add_input_register(time_md_, 0x0016);
            server_->add_input_register(time_yw_, 0x0017);
            server_->add_input_register(ip_, 0x0018);
            server_->add_input_register(mac_, 0x001A);
            server_->add_input_register(portal_pwd_, 0x001C);

            // Déclaration des registres Holding (FC=06)
            server_->add_holding_register(rotation_, 0x0800);
            server_->add_holding_register(wifi_mode_, 0x0801);

            ESP_LOGI(TAG, "Registers mapped to Modbus server");
        }

        void SmartEVSEModbusServer::loop() {
            // Rien de spécial ici : les valeurs sont publiées par les sensors
            // et le Modbus server les expose automatiquement.
            // On pourrait ajouter une logique de mise à jour si nécessaire.
        }

    }  // namespace smartevse_modbus
}  // namespace esphome
