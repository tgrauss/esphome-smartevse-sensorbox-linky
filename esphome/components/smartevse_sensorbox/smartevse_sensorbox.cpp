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

      // 3) Compute total power (CT-based)
      float p_total_ct = NAN;
      if (!isnan(ia) && !isnan(ib) && !isnan(ic)) {
        if (three_phase_) {
          const float i_avg = (ia + ib + ic) / 3.0f;
          p_total_ct = sqrtf(3.0f) * nominal_voltage_phase_ * i_avg * power_factor_;
        } else {
          p_total_ct = (ia + ib + ic) * nominal_voltage_phase_ * power_factor_;
        }
      }

      // 4) Linky TIC passthrough
      if (linky_power_in_ && linky_power_in_->has_state()) {
        linky_power_out->publish_state(linky_power_in_->state);
      }
      if (linky_energy_in_ && linky_energy_in_->has_state()) {
        linky_energy_out->publish_state(linky_energy_in_->state);
      }

      // 5) Choix de la puissance totale
      float p_out = NAN;
      if (prefer_linky_power_ && linky_power_in_ && linky_power_in_->has_state()) {
        p_out = linky_power_in_->state;
      } else {
        p_out = p_total_ct;
      }
      ct_total_power_out->publish_state(p_out);

      // 6) Publier courants et tensions TIC
      if (linky_current_l1_in_ && linky_current_l1_in_->has_state())
        p1_current_l1_out->publish_state(linky_current_l1_in_->state);
      if (linky_current_l2_in_ && linky_current_l2_in_->has_state())
        p1_current_l2_out->publish_state(linky_current_l2_in_->state);
      if (linky_current_l3_in_ && linky_current_l3_in_->has_state())
        p1_current_l3_out->publish_state(linky_current_l3_in_->state);

      if (linky_voltage_l1_in_ && linky_voltage_l1_in_->has_state())
        voltage_l1_out->publish_state(linky_voltage_l1_in_->state);
      if (linky_voltage_l2_in_ && linky_voltage_l2_in_->has_state())
        voltage_l2_out->publish_state(linky_voltage_l2_in_->state);
      if (linky_voltage_l3_in_ && linky_voltage_l3_in_->has_state())
        voltage_l3_out->publish_state(linky_voltage_l3_in_->state);

      // 7) Preference flag
      prefer_ct_out->publish_state(prefer_linky_power_ ? 0.0f : 1.0f);

      // 8) Extra registers
      version_out->publish_state(0x0114); // Exemple version
      dsmr_info_out->publish_state(0x3283); // Exemple DSMR info

      // WiFi status based on wifi_mode parameter
      if (wifi_mode_ == 0) {
        wifi_status_out->publish_state(0); // WiFi désactivé
        ip_out->publish_state(0);
        mac_out->publish_state(0);
        portal_pwd_out->publish_state(0);
      } else if (wifi_mode_ == 1) {
        wifi_status_out->publish_state(1); // WiFi actif
        ip_out->publish_state(0xC0A80101); // Exemple IP 192.168.1.1
        mac_out->publish_state(0x112233445566); // Exemple MAC
        portal_pwd_out->publish_state(1234); // Exemple mot de passe portail
      } else if (wifi_mode_ == 2) {
        wifi_status_out->publish_state(2); // Mode portail
        ip_out->publish_state(0xC0A801FE); // Exemple IP portail
        mac_out->publish_state(0xAABBCCDDEEFF); // Exemple MAC portail
        portal_pwd_out->publish_state(5678); // Exemple mot de passe portail
      }

      // Rotation + wire_mode published in 0x0800
      int reg0800 = (wire_mode_ << 1) | (rotation_ & 0x01);
      rotation_out->publish_state(reg0800);

      // WiFi mode register (0x0801)
      wifi_mode_out->publish_state(wifi_mode_);

      // 9) Time registers
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
    }

  }  // namespace smartevse_sensorbox
}  // namespace esphome
