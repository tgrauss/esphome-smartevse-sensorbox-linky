#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
    namespace smartevse_sensorbox {

        class SmartEVSESensorBox : public PollingComponent {
        public:
            SmartEVSESensorBox();

            // Inputs (ADS1115 channels and Linky)
            void set_ct_phase_a_input(sensor::Sensor *s) { ct_a_in_ = s; }
            void set_ct_phase_b_input(sensor::Sensor *s) { ct_b_in_ = s; }
            void set_ct_phase_c_input(sensor::Sensor *s) { ct_c_in_ = s; }
            void set_ads_ref_input(sensor::Sensor *s) { ads_ref_in_ = s; }

            void set_linky_power_input(sensor::Sensor *s) { linky_power_in_ = s; }   // W
            void set_linky_energy_input(sensor::Sensor *s) { linky_energy_in_ = s; } // Wh

            // Calibration (per-phase, applied to raw CT readings)
            void set_ct_gain_a(float g) { ct_gain_a_ = g; }
            void set_ct_gain_b(float g) { ct_gain_b_ = g; }
            void set_ct_gain_c(float g) { ct_gain_c_ = g; }
            void set_ct_offset_a(float o) { ct_offset_a_ = o; }
            void set_ct_offset_b(float o) { ct_offset_b_ = o; }
            void set_ct_offset_c(float o) { ct_offset_c_ = o; }

            // ADS reference for autocalibration (e.g., ADS A0 wired to 1.0 V)
            void set_ads_ref_voltage(float v) { ads_ref_voltage_ = v; }
            void enable_autocalibration(bool e) { autocalibration_enabled_ = e; }

            // Power computation config
            void set_nominal_voltage_phase(float v) { nominal_voltage_phase_ = v; } // per-phase nominal voltage
            void set_power_factor(float pf) { power_factor_ = pf; }
            void set_three_phase(bool t) { three_phase_ = t; } // tri-phasé approximation
            void set_prefer_linky_power(bool p) { prefer_linky_power_ = p; }
            bool get_prefer_linky_power() const { return prefer_linky_power_; }

            // Outputs (publish to HA, can be mapped to Modbus server)
            sensor::Sensor *ct_phase_a_out = new sensor::Sensor();       // A
            sensor::Sensor *ct_phase_b_out = new sensor::Sensor();       // A
            sensor::Sensor *ct_phase_c_out = new sensor::Sensor();       // A
            sensor::Sensor *ct_total_current_out = new sensor::Sensor(); // A
            sensor::Sensor *ct_total_power_out = new sensor::Sensor();   // W (approx or Linky)
            sensor::Sensor *linky_power_out = new sensor::Sensor();      // W
            sensor::Sensor *linky_energy_out = new sensor::Sensor();     // Wh
            sensor::Sensor *prefer_ct_out = new sensor::Sensor();        // 1 = CT, 0 = Linky

            void setup() override;
            void update() override;

        protected:
            // Inputs
            sensor::Sensor *ct_a_in_{nullptr};
            sensor::Sensor *ct_b_in_{nullptr};
            sensor::Sensor *ct_c_in_{nullptr};
            sensor::Sensor *ads_ref_in_{nullptr};
            sensor::Sensor *linky_power_in_{nullptr};
            sensor::Sensor *linky_energy_in_{nullptr};

            // Calibration
            float ct_gain_a_{1.0f};
            float ct_gain_b_{1.0f};
            float ct_gain_c_{1.0f};
            float ct_offset_a_{0.0f};
            float ct_offset_b_{0.0f};
            float ct_offset_c_{0.0f};

            // Autocalibration reference
            float ads_ref_voltage_{1.0f};
            bool autocalibration_enabled_{true};

            // Power computation
            float nominal_voltage_phase_{230.0f};
            float power_factor_{0.95f};
            bool three_phase_{false};
            bool prefer_linky_power_{true};

            // Helpers
            inline float calibrate_(float raw, float gain, float offset) const { return raw * gain + offset; }
            inline bool has_ct_inputs_() const { return ct_a_in_ && ct_b_in_ && ct_c_in_; }
            inline bool has_linky_power_() const { return linky_power_in_ && linky_power_in_->has_state(); }
        };

    }  // namespace smartevse_sensorbox
}  // namespace esphome
