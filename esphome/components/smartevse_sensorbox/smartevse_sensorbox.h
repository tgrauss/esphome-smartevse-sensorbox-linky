#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
    namespace smartevse_sensorbox {

        // Ce composant agrège les mesures CT (ADS1115) et TIC (Teleinfo Linky),
        // les calibre et les publie vers des capteurs internes.

        class SmartEVSESensorBox : public PollingComponent {
        public:
            SmartEVSESensorBox() : PollingComponent(1000) {}

            // Entrées CT et ADS
            sensor::Sensor *ct_a_in_{nullptr};
            sensor::Sensor *ct_b_in_{nullptr};
            sensor::Sensor *ct_c_in_{nullptr};
            sensor::Sensor *ads_ref_in_{nullptr};

            // Entrées TIC Linky
            sensor::Sensor *linky_power_in_{nullptr};
            sensor::Sensor *linky_energy_in_{nullptr};
            sensor::Sensor *linky_current_l1_in_{nullptr};
            sensor::Sensor *linky_current_l2_in_{nullptr};
            sensor::Sensor *linky_current_l3_in_{nullptr};
            sensor::Sensor *linky_voltage_l1_in_{nullptr};
            sensor::Sensor *linky_voltage_l2_in_{nullptr};
            sensor::Sensor *linky_voltage_l3_in_{nullptr};

            // Sorties CT
            sensor::Sensor *ct_phase_a_out{new sensor::Sensor()};
            sensor::Sensor *ct_phase_b_out{new sensor::Sensor()};
            sensor::Sensor *ct_phase_c_out{new sensor::Sensor()};
            sensor::Sensor *ct_total_current_out{new sensor::Sensor()};
            sensor::Sensor *ct_total_power_out{new sensor::Sensor()};

            // Sorties TIC
            sensor::Sensor *linky_power_out{new sensor::Sensor()};
            sensor::Sensor *linky_energy_out{new sensor::Sensor()};
            sensor::Sensor *p1_current_l1_out{new sensor::Sensor()};
            sensor::Sensor *p1_current_l2_out{new sensor::Sensor()};
            sensor::Sensor *p1_current_l3_out{new sensor::Sensor()};
            sensor::Sensor *voltage_l1_out{new sensor::Sensor()};
            sensor::Sensor *voltage_l2_out{new sensor::Sensor()};
            sensor::Sensor *voltage_l3_out{new sensor::Sensor()};

            // Registres supplémentaires SensorBox
            sensor::Sensor *version_out{new sensor::Sensor()};
            sensor::Sensor *dsmr_info_out{new sensor::Sensor()};
            sensor::Sensor *wifi_status_out{new sensor::Sensor()};
            sensor::Sensor *time_hm_out{new sensor::Sensor()};
            sensor::Sensor *time_md_out{new sensor::Sensor()};
            sensor::Sensor *time_yw_out{new sensor::Sensor()};
            sensor::Sensor *ip_out{new sensor::Sensor()};
            sensor::Sensor *mac_out{new sensor::Sensor()};
            sensor::Sensor *portal_pwd_out{new sensor::Sensor()};
            sensor::Sensor *rotation_out{new sensor::Sensor()};
            sensor::Sensor *wifi_mode_out{new sensor::Sensor()};

            // Paramètres internes
            float ct_gain_a_{1.0f};
            float ct_gain_b_{1.0f};
            float ct_gain_c_{1.0f};
            float ct_offset_a_{0.0f};
            float ct_offset_b_{0.0f};
            float ct_offset_c_{0.0f};
            float ads_ref_voltage_{2.048f};
            bool autocalibration_enabled_{true};
            bool three_phase_{true};
            float nominal_voltage_phase_{230.0f};
            float power_factor_{0.95f};
            bool prefer_linky_power_{true};
            int rotation_{0};
            int wire_mode_{1};
            int wifi_mode_{1};

            void setup() override;
            void update() override;

        protected:
            float calibrate_(float raw, float gain, float offset) {
                return raw * gain + offset;
            }
        };

    }  // namespace smartevse_sensorbox
}  // namespace esphome
