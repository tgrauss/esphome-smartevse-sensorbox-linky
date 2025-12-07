#include "smartevse_sensorbox.h"
#include "esphome/core/log.h"
#include <math.h>

namespace esphome {
  namespace smartevse_sensorbox {

    static const char *const TAG = "smartevse_sensorbox";

    SmartEVSESensorBox::SmartEVSESensorBox() : PollingComponent(1000) {}

    void SmartEVSESensorBox::setup() {
      ESP_LOGI(TAG, "Setup: Vphase=%.1fV, PF=%.2f, three_phase=%s, prefer_linky_power=%s, autocalibration=%s (ref=%.3fV)",
               nominal_voltage_phase_, power_factor_,
               three_phase_ ? "true" : "false",
               prefer_linky_power_ ? "true" : "false",
               autocalibration_enabled_ ? "true" : "false",
               ads_ref_voltage_);
    }

    void SmartEVSESensorBox::update() {
      // 1) Autocalibration using ADS reference (e.g., ADS A0 = 1.0V)
      if (autocalibration_enabled_ && ads_ref_in_ && ads_ref_in_->has_state()) {
        const float measured = ads_ref_in_->state;
        if (!isnan(measured) && measured > 0.0f) {
          const float correction = ads_ref_voltage_ / measured;
          // Apply same correction to all CT gains
          ct_gain_a_ *= correction;
          ct_gain_b_ *= correction;
          ct_gain_c_ *= correction;
          ESP_LOGI(TAG, "Autocalibration: measured=%.3fV expected=%.3fV correction=%.5f", measured, ads_ref_voltage_, correction);
        } else {
          ESP_LOGW(TAG, "Autocalibration skipped: invalid ADS ref reading");
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
      } else {
        ct_total_current_out->publish_state(NAN);
      }

      // 3) Compute total power (from CTs or prefer Linky)
      float p_total_ct = NAN;
      if (!isnan(ia) && !isnan(ib) && !isnan(ic)) {
        if (three_phase_) {
          // Simple 3-phase approximation: P ≈ sqrt(3) * Vphase * Iavg * PF
          const float i_avg = (ia + ib + ic) / 3.0f;
          p_total_ct = sqrtf(3.0f) * nominal_voltage_phase_ * i_avg * power_factor_;
        } else {
          // Single-phase approximation: P ≈ (Ia+Ib+Ic) * Vphase * PF
          p_total_ct = (ia + ib + ic) * nominal_voltage_phase_ * power_factor_;
        }
      }

      float p_out = NAN;
      if (prefer_linky_power_ && has_linky_power_()) {
        p_out = linky_power_in_->state;
      } else {
        p_out = p_total_ct;
      }
      ct_total_power_out->publish_state(p_out);

      // 4) Linky passthrough (power, energy)
      if (linky_power_in_) {
        linky_power_out->publish_state(linky_power_in_->has_state() ? linky_power_in_->state : NAN);
      } else {
        linky_power_out->publish_state(NAN);
      }

      if (linky_energy_in_) {
        linky_energy_out->publish_state(linky_energy_in_->has_state() ? linky_energy_in_->state : NAN);
      } else {
        linky_energy_out->publish_state(NAN);
      }

      // 5) Publish source preference (1 = CT, 0 = Linky)
      prefer_ct_out->publish_state(prefer_linky_power_ ? 0.0f : 1.0f);
    }

  }  // namespace smartevse_sensorbox
}  // namespace esphome
