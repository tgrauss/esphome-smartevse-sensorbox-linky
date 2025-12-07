#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/modbus_server/modbus_server.h"

namespace esphome {
    namespace smartevse_modbus {

        // Ce composant gère l’exposition des registres Modbus SensorBox-V2
        // en s’appuyant sur les capteurs fournis par SmartEVSESensorBox.

        class SmartEVSEModbusServer : public Component {
        public:
            SmartEVSEModbusServer() = default;

            // Lien vers le serveur Modbus ESPHome
            void set_modbus_server(modbus_server::ModbusServer *server) { server_ = server; }

            // Lien vers les capteurs de la SensorBox
            void set_version(sensor::Sensor *s) { version_ = s; }
            void set_dsmr_info(sensor::Sensor *s) { dsmr_info_ = s; }
            void set_voltage_l1(sensor::Sensor *s) { voltage_l1_ = s; }
            void set_voltage_l2(sensor::Sensor *s) { voltage_l2_ = s; }
            void set_voltage_l3(sensor::Sensor *s) { voltage_l3_ = s; }
            void set_p1_current_l1(sensor::Sensor *s) { p1_current_l1_ = s; }
            void set_p1_current_l2(sensor::Sensor *s) { p1_current_l2_ = s; }
            void set_p1_current_l3(sensor::Sensor *s) { p1_current_l3_ = s; }
            void set_ct_phase_a(sensor::Sensor *s) { ct_phase_a_ = s; }
            void set_ct_phase_b(sensor::Sensor *s) { ct_phase_b_ = s; }
            void set_ct_phase_c(sensor::Sensor *s) { ct_phase_c_ = s; }
            void set_ct_total_current(sensor::Sensor *s) { ct_total_current_ = s; }
            void set_ct_total_power(sensor::Sensor *s) { ct_total_power_ = s; }
            void set_linky_power(sensor::Sensor *s) { linky_power_ = s; }
            void set_linky_energy(sensor::Sensor *s) { linky_energy_ = s; }
            void set_wifi_status(sensor::Sensor *s) { wifi_status_ = s; }
            void set_time_hm(sensor::Sensor *s) { time_hm_ = s; }
            void set_time_md(sensor::Sensor *s) { time_md_ = s; }
            void set_time_yw(sensor::Sensor *s) { time_yw_ = s; }
            void set_ip(sensor::Sensor *s) { ip_ = s; }
            void set_mac(sensor::Sensor *s) { mac_ = s; }
            void set_portal_pwd(sensor::Sensor *s) { portal_pwd_ = s; }
            void set_rotation(sensor::Sensor *s) { rotation_ = s; }
            void set_wifi_mode(sensor::Sensor *s) { wifi_mode_ = s; }

            void setup() override;
            void loop() override;

        protected:
            modbus_server::ModbusServer *server_{nullptr};

            // Pointeurs vers les capteurs
            sensor::Sensor *version_{nullptr};
            sensor::Sensor *dsmr_info_{nullptr};
            sensor::Sensor *voltage_l1_{nullptr};
            sensor::Sensor *voltage_l2_{nullptr};
            sensor::Sensor *voltage_l3_{nullptr};
            sensor::Sensor *p1_current_l1_{nullptr};
            sensor::Sensor *p1_current_l2_{nullptr};
            sensor::Sensor *p1_current_l3_{nullptr};
            sensor::Sensor *ct_phase_a_{nullptr};
            sensor::Sensor *ct_phase_b_{nullptr};
            sensor::Sensor *ct_phase_c_{nullptr};
            sensor::Sensor *ct_total_current_{nullptr};
            sensor::Sensor *ct_total_power_{nullptr};
            sensor::Sensor *linky_power_{nullptr};
            sensor::Sensor *linky_energy_{nullptr};
            sensor::Sensor *wifi_status_{nullptr};
            sensor::Sensor *time_hm_{nullptr};
            sensor::Sensor *time_md_{nullptr};
            sensor::Sensor *time_yw_{nullptr};
            sensor::Sensor *ip_{nullptr};
            sensor::Sensor *mac_{nullptr};
            sensor::Sensor *portal_pwd_{nullptr};
            sensor::Sensor *rotation_{nullptr};
            sensor::Sensor *wifi_mode_{nullptr};
        };

    }  // namespace smartevse_modbus
}  // namespace esphome
