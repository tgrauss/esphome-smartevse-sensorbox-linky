#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
    namespace smartevse_sensorbox {

        class SmartEVSESensorBox : public PollingComponent {
        public:
            SmartEVSESensorBox();

            // Inputs (ADS1115 channels)
            void set_ct_phase_a_input(sensor::Sensor *s) { ct_a_in_ = s; }
            void set_ct_phase_b_input(sensor::Sensor *s) { ct_b_in_ = s; }
            void set_ct_phase_c_input(sensor::Sensor *s) { ct_c_in_ = s; }
            void set_ads_ref_input(sensor::Sensor *s) { ads_ref_in_ = s; }

            // Inputs (Teleinfo sensors)
            void set_linky_power_input(sensor::Sensor *s) { linky_power_in_ = s; }   // SINSTS
            void set_linky_energy_input(sensor::Sensor *s) { linky_energy_in_ = s; } // EAST
            void set_linky_current_l1(sensor::Sensor *s) { linky_current_l1_in_ = s; } // IRMS1
            void set_linky_current_l2(sensor::Sensor *s) { linky_current_l2_in_ = s; } // IRMS2
            void set_linky_current_l3(sensor::Sensor *s) { linky_current_l3_in_ = s; } // IRMS3
            void set_linky_voltage_l1(sensor::Sensor *s) { linky_voltage_l1_in_ = s; } // URMS1
            void set_linky_voltage_l2(sensor::Sensor *s) { linky_voltage_l2_in_ = s; } // URMS2
            void set_linky_voltage_l3(sensor::Sensor *s) { linky_voltage_l3_in_ = s; } // URMS3

            // Calibration
            void set_ct_gain_a(float g) { ct_gain_a_ = g; }
            void set_ct_gain_b(float g) { ct_gain_b_ = g; }
            void set_ct_gain_c(float g) { ct_gain_c_ = g; }
            void set_ct_offset_a(float o) { ct_offset_a_ = o; }
            void set_ct_offset_b(float o) { ct_offset_b_ = o; }
            void set_ct_offset_c(float o) { ct_offset_c_ = o; }

            void set_ads_ref_voltage(float v) { ads_ref_voltage_ = v; }
            void enable_autocalibration(bool e) { autocalibration_enabled_ = e; }

            void set_nominal_voltage_phase(float v) { nominal_voltage_phase_ = v; }
            void set_power_factor(float pf) { power_factor_ = pf; }
            void set_three_phase(bool t) { three_phase_ = t; }
            void set_prefer_linky_power(bool p) { prefer_linky_power_ = p; }

            // Paramètres configurables via Holding Registers
            void set_rotation(int r) { rotation_ = r; }
            void set_wire_mode(int w) { wire_mode_ = w; }
            void set_wifi_mode(int m) { wifi_mode_ = m; } // 0=disabled, 1=enabled, 2=portal

            // Outputs (Sensorbox-V2 registers)
            sensor::Sensor *ct_phase_a_out = new sensor::Sensor();       // 0x000E
            sensor::Sensor *ct_phase_b_out = new sensor::Sensor();       // 0x0010
            sensor::Sensor *ct_phase_c_out = new sensor::Sensor();       // 0x0012
            sensor::Sensor *ct_total_current_out = new sensor::Sensor();
            sensor::Sensor *ct_total_power_out = new sensor::Sensor();
            sensor::Sensor *linky_power_out = new sensor::Sensor();
            sensor::Sensor *linky_energy_out = new sensor::Sensor();
            sensor::Sensor *prefer_ct_out = new sensor::Sensor();

            // Extra registers for Sensorbox-V2 compatibility
            sensor::Sensor *version_out = new sensor::Sensor();          // 0x0000
            sensor::Sensor *dsmr_info_out = new sensor::Sensor();        // 0x0001
            sensor::Sensor *voltage_l1_out = new sensor::Sensor();       // 0x0002
            sensor::Sensor *voltage_l2_out = new sensor::Sensor();       // 0x0004
            sensor::Sensor *voltage_l3_out = new sensor::Sensor();       // 0x0006
            sensor::Sensor *p1_current_l1_out = new sensor::Sensor();    // 0x0008
            sensor::Sensor *p1_current_l2_out = new sensor::Sensor();    // 0x000A
            sensor::Sensor *p1_current_l3_out = new sensor::Sensor();    // 0x000C
            sensor::Sensor *wifi_status_out = new sensor::Sensor();      // 0x0014
            sensor::Sensor *time_hm_out = new sensor::Sensor();          // 0x0015
            sensor::Sensor *time_md_out = new sensor::Sensor();          // 0x0016
            sensor::Sensor *time_yw_out = new sensor::Sensor();          // 0x0017
            sensor::Sensor *ip_out = new sensor::Sensor();               // 0x0018
            sensor::Sensor *mac_out = new sensor::Sensor();              // 0x001A
            sensor::Sensor *portal_pwd_out = new sensor::Sensor();       // 0x001C
            sensor::Sensor *rotation_out = new sensor::Sensor();         // 0x0800
            sensor::Sensor *wifi_mode_out = new sensor::Sensor();        // 0x0801

            void setup() override;
            void update() override;

        protected:
            // Inputs CT
            sensor::Sensor *ct_a_in_{nullptr};
            sensor::Sensor *ct_b_in_{nullptr};
            sensor::Sensor *ct_c_in_{nullptr};
            sensor::Sensor *ads_ref_in_{nullptr};

            // Inputs TIC
            sensor::Sensor *linky_power_in_{nullptr};
            sensor::Sensor *linky_energy_in_{nullptr};
            sensor::Sensor *linky_current_l1_in_{nullptr};
            sensor::Sensor *linky_current_l2_in_{nullptr};
            sensor::Sensor *linky_current_l3_in_{nullptr};
            sensor::Sensor *linky_voltage_l1_in_{nullptr};
            sensor::Sensor *linky_voltage_l2_in_{nullptr};
            sensor::Sensor *linky_voltage_l3_in_{nullptr};

            // Calibration
            float ct_gain_a_{1.0f};
            float ct_gain_b_{1.0f};
            float ct_gain_c_{1.0f};
            float ct_offset_a_{0.0f};
            float ct_offset_b_{0.0f};
            float ct_offset_c_{0.0f};

            float ads_ref_voltage_{1.0f};
            bool autocalibration_enabled_{true};

            float nominal_voltage_phase_{230.0f};
            float power_factor_{0.95f};
            bool three_phase_{false};
            bool prefer_linky_power_{true};

            // Paramètres configurables
            int rotation_{0};   // 0 = droite, 1 = gauche
            int wire_mode_{1};  // 0 = 4 fils, 1 = 3 fils
            int wifi_mode_{1};  // 0 = désactivé, 1 = activé, 2 = portail

            inline float calibrate_(float raw, float gain, float offset) const { return raw * gain + offset; }
        };

    }  // namespace smartevse_sensorbox
}  // namespace esphome
