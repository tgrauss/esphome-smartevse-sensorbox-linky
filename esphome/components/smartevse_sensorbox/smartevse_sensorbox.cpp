#include "smartevse_sensorbox.h"
#include "esphome/core/log.h"
#include <math.h>
#include <ctime>

namespace esphome {
  namespace smartevse_sensorbox {

    static const char *const TAG = "smartevse_sensorbox";

    SmartEVSESensorBox::SmartEVSESensorBox() : PollingComponent(1000) {}

    void SmartEVSESensorBox::setup() {
      ESP_LOGI(TAG, "SmartEVSE SensorBox setup complete");
    }

    void SmartEVSESensorBox::update() {
      // 1) Autocalibration using ADS reference
      if (autocalibration_enabled_ && ads_ref_in_ && ads_ref_in_->has_state()) {
        const float measured = ads_ref_in_->state;
        if (!isnan(measured) && measured > 0.0f) {
          const float correction = ads_ref_voltage_ / measured;
          ct_gain_a_ *= correction;
          ct_gain_b_ *= correction;
          ct_gain_c_ *= correction;
          ESP_LOGI(TAG, "Autocalibration applied: correction=%.5f", correction);
        }
      }

      // 2) Read and calibrate CTs
      float ia = NAN, ib = NAN, ic = NAN;
      if (ct_a_in_ && ct_a_in_->has_state()) ia = calibrate_(ct_a_in_->state, ct_gain_a_, ct_offset_a_);
      if (ct_b_in_ && ct_b_in_->has_state()) ib = calibrate_(ct_b_in_->state, ct_gain_b_, ct_offset_b_);
      if (ct_c_in_ && ct_c_in_->has_state()) ic = calibrate_(ct_c_in_->state, ct_gain_c_, ct_offset_c_);

      ct_phase_a_out->publish_state(ia);
      ct_phase_b_out->publish_state(ib);
      ct_phase_c_out->publish_state(ic);

      float i_total = NAN;
      if (!isnan(ia) && !isnan(ib) && !isnan(ic)) {
        i_total = ia + ib + ic;
        ct_total_current_out->publish_state(i_total);
      }

      // 3) Compute total power
      float p_total_ct = NAN;
      if (!isnan(ia) && !isnan(ib) && !isnan(ic)) {
        if (three_phase_) {
          const float i_avg = (ia + ib + ic) / 3.0f;
          p_total_ct = sqrtf(3.0f) * nominal_voltage_phase_ * i_avg * power_factor_;
        } else {
          p_total_ct = (ia + ib + ic) * nominal_voltage_phase_ * power_factor_;
        }
      }

      float p_out = NAN;
      if (prefer_linky_power_ && linky_power_in_ && linky_power_in_->has_state()) {
        p_out = linky_power_in_->state;
      } else {
        p_out = p_total_ct;
      }
      ct_total_power_out->publish_state(p_out);

      // 4) Linky passthrough
      if (linky_power_in_) {
        linky_power_out->publish_state(linky_power_in_->has_state() ? linky_power_in_->state : NAN);
      }
      if (linky_energy_in_) {
        linky_energy_out->publish_state(linky_energy_in_->has_state() ? linky_energy_in_->state : NAN);
      }

      // 5) Preference flag
      prefer_ct_out->publish_state(prefer_linky_power_ ? 0.0f : 1.0f);

      // 6) Extra registers for Sensorbox-V2
      version_out->publish_state(0x0114); // Example version
      dsmr_info_out->publish_state(0x3283); // Example DSMR info

      voltage_l1_out->publish_state(nominal_voltage_phase_);
      voltage_l2_out->publish_state(nominal_voltage_phase_);
      voltage_l3_out->publish_state(nominal_voltage_phase_);

      p1_current_l1_out->publish_state(NAN); // Not connected
      p1_current_l2_out->publish_state(NAN);
      p1_current_l3_out->publish_state(NAN);

      wifi_status_out->publish_state(1); // Example: WiFi enabled
      wifi_mode_out->publish_state(1);   // Example: WiFi mode enabled

      // Rotation + wire_mode published in 0x0800
      int reg0800 = (wire_mode_ << 1) | (rotation_ & 0x01);
      rotation_out->publish_state(reg0800);

      // Time registers
      std::time_t t = std::time(nullptr);
      std::tm *tm = std::localtime(&t);
      if (tm) {
        int hm = (tm->tm_hour << 8) | tm->tm_min;
        int md = ((tm->tm_mon + 1) << 8) | tm->tm_mday;
        int yw = ((1900 + tm->tm_year) << 8) | tm->tm_wday;
        time_hm_out->publish_state(hm);
        time_md_out->publish_state(md);
        time_yw_out->publish_state(yw);
      }

      ip_out->publish_state(0);        // Placeholder
      mac_out->publish_state(0);       // Placeholder
      portal_pwd_out->publish_state(0);// Placeholder
    }

  }  // namespace smartevse_sensorbox
}  // namespace esphome
