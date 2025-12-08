#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ads1115/ads1115.h"
#include "esphome/components/teleinfo/teleinfo.h"

// Forward-declare your custom Modbus server class from esphome/components/modbus_server
namespace esphome {
    namespace modbus_server {
        class ModbusServerComponent;
    }  // namespace modbus_server
}  // namespace esphome

namespace esphome {
    namespace smartevse_sensorbox {

        class SmartEVSESensorBox : public PollingComponent {
        public:
            // ===== Lifecycle =====
            void setup() override;
            void update() override;  // fast block cadence; slow cadence handled internally
            void dump_config() override;

            // ===== Dependency setters =====
            void set_ads1115(ads1115::ADS1115Component *ads) { this->ads_ = ads; }
            void set_teleinfo(teleinfo::TeleinfoComponent *tic) { this->tic_ = tic; }
            void set_modbus_smartevse(modbus_server::ModbusServerComponent *srv) { this->mb_smartevse_ = srv; }
            void set_modbus_esphome(modbus_server::ModbusServerComponent *srv) { this->mb_esphome_ = srv; }

            // ===== Configuration setters =====
            void set_three_phase(bool v) { this->three_phase_ = v; }
            void set_prefer_linky_power(bool v) { this->prefer_linky_power_ = v; }

            void set_rotation(uint8_t v) { this->rotation_ = v & 0x01; }
            void set_wire_mode(uint8_t v) { this->wire_mode_ = v & 0x01; }
            void set_wifi_mode(uint8_t v) { this->wifi_mode_ = v; }

            void set_ct_gain_a(float v) { this->ct_gain_a_ = v; }
            void set_ct_gain_b(float v) { this->ct_gain_b_ = v; }
            void set_ct_gain_c(float v) { this->ct_gain_c_ = v; }
            void set_ct_offset_a(float v) { this->ct_offset_a_ = v; }
            void set_ct_offset_b(float v) { this->ct_offset_b_ = v; }
            void set_ct_offset_c(float v) { this->ct_offset_c_ = v; }

            void set_ads_ref_voltage(float v) { this->ads_ref_voltage_ = v; }         // nominal 1.0 V
            void set_nominal_voltage_phase(float v) { this->nominal_voltage_phase_ = v; }  // default 230 V
            void set_power_factor(float v) { this->power_factor_ = v; }               // default 0.95

            void set_fast_interval_ms(uint32_t v) { this->fast_interval_ms_ = v; }
            void set_slow_interval_ms(uint32_t v) { this->slow_interval_ms_ = v; }

        private:
            // ===== Dependencies =====
            ads1115::ADS1115Component *ads_{nullptr};
            teleinfo::TeleinfoComponent *tic_{nullptr};
            modbus_server::ModbusServerComponent *mb_smartevse_{nullptr};  // SmartEVSE SensorBox-2 compatible bus
            modbus_server::ModbusServerComponent *mb_esphome_{nullptr};    // Linky Modbus decimal map bus

            // ===== Configuration =====
            bool three_phase_{false};
            bool prefer_linky_power_{true};
            uint8_t rotation_{0};
            uint8_t wire_mode_{1};
            uint8_t wifi_mode_{1};

            float ct_gain_a_{1.0f};
            float ct_gain_b_{1.0f};
            float ct_gain_c_{1.0f};
            float ct_offset_a_{0.0f};
            float ct_offset_b_{0.0f};
            float ct_offset_c_{0.0f};

            float ads_ref_voltage_{1.0f};
            float nominal_voltage_phase_{230.0f};
            float power_factor_{0.95f};

            // ===== Cadence (SensorBox-2 compatible) =====
            uint32_t fast_interval_ms_{1000};  // CT/TIC instantaneous values
            uint32_t slow_interval_ms_{2000};  // energies, max power, text labels
            uint32_t last_fast_{0};
            uint32_t last_slow_{0};

            // ===== Internal state (CT) =====
            float vref_meas_{1.0f};     // measured A0 volts
            float ct_a_curr_a_{0.0f};   // phase currents in amps
            float ct_b_curr_a_{0.0f};
            float ct_c_curr_a_{0.0f};
            uint32_t ct_total_va_{0};   // total apparent power

            // ===== Internal state (Linky TIC, standard mode) =====
            // Energies (Wh), powers (VA), currents (A), voltages (V), tariffs/state
            uint32_t east_wh_{0};
            uint32_t eait_wh_{0};
            uint32_t easf01_wh_{0};
            uint32_t easf02_wh_{0};
            uint32_t easf03_wh_{0};
            uint32_t easf04_wh_{0};
            uint32_t easf05_wh_{0};
            uint32_t easf06_wh_{0};

            uint16_t irms1_a_{0};
            uint16_t irms2_a_{0};
            uint16_t irms3_a_{0};
            uint16_t urms1_v_{0};
            uint16_t urms2_v_{0};
            uint16_t urms3_v_{0};

            uint32_t sinsts_va_{0};
            uint32_t sinst1_va_{0};
            uint32_t sinst2_va_{0};
            uint32_t sinst3_va_{0};
            uint32_t smaxsn_va_{0};
            uint32_t smaxsn1_va_{0};
            uint32_t smaxsn2_va_{0};
            uint32_t smaxsn3_va_{0};
            uint32_t pcoup_va_{0};
            uint16_t pref_kva_{0};
            uint32_t ccasn_w_{0};

            uint16_t ntarf_{0};
            uint16_t njourf_{0};
            uint16_t njourf_p1_{0};
            uint64_t ngtf_{0};
            uint64_t ltarf_{0};
            uint64_t pjourf_p1_{0};
            uint64_t ppointe_{0};

            // ===== Helpers =====
            void read_ads_ct_fast_();        // Read A0..A3 in single-shot, PGA ±2.048V, SPS=860, compute currents and total VA
            void read_teleinfo_fast_();      // Read instantaneous TIC values (SINSTS, IRMSx, URMSx, SINSTx)
            void read_teleinfo_slow_();      // Read energies and tariff/state labels (EAST, EAIT, EASFxx, SMAXSNx, PREF, etc.)
            void publish_smartevse_fast_();  // Publish fast block to SmartEVSE SensorBox-2 bus
            void publish_smartevse_slow_();  // Publish slow block to SmartEVSE SensorBox-2 bus
            void publish_esphome_fast_();    // Publish fast block to Linky Modbus bus (decimal)
            void publish_esphome_slow_();    // Publish slow block to Linky Modbus bus (decimal)
        };

    }  // namespace smartevse_sensorbox
}  // namespace esphome
