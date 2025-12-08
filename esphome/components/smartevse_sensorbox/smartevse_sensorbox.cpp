#include "SmartEVSESensorBox.h"
#include "esphome/core/log.h"

namespace esphome {
  namespace smartevse_sensorbox {

    static const char *TAG = "smartevse.sensorbox";

    // Helper: write on Modbus server (16/32/64-bit)
    static inline void write_u16(modbus_server::ModbusServerComponent *srv, uint16_t addr, uint16_t val) {
      if (srv) srv->write_holding_u16(addr, val);
    }
    static inline void write_u32(modbus_server::ModbusServerComponent *srv, uint16_t addr, uint32_t val) {
      if (srv) srv->write_holding_u32(addr, val);  // addr..addr+1
    }
    static inline void write_u64(modbus_server::ModbusServerComponent *srv, uint16_t addr, uint64_t val) {
      if (srv) srv->write_holding_u64(addr, val);  // addr..addr+3
    }

    void SmartEVSESensorBox::setup() {
      bool ok = true;
      if (this->ads_ == nullptr) {
        ESP_LOGE(TAG, "ADS1115 not set (ads1115_id).");
        ok = false;
      }
      if (this->tic_ == nullptr) {
        ESP_LOGE(TAG, "Teleinfo not set (teleinfo_id).");
        ok = false;
      }
      if (this->mb_smartevse_ == nullptr) {
        ESP_LOGE(TAG, "Modbus SmartEVSE server not set (modbus_smartevse_id).");
        ok = false;
      }
      if (this->mb_esphome_ == nullptr) {
        ESP_LOGE(TAG, "Modbus ESPHome server not set (modbus_esphome_id).");
        ok = false;
      }
      if (!ok) {
        this->mark_failed();
        return;
      }
      ESP_LOGI(TAG, "Setup OK: fast=%ums slow=%ums", this->fast_interval_ms_, this->slow_interval_ms_);
    }

    void SmartEVSESensorBox::update() {
      const uint32_t now = millis();

      // Fast cadence (approx 1s)
      if (now - this->last_fast_ >= this->fast_interval_ms_) {
        this->last_fast_ = now;

        // Read instantaneous data
        this->read_ads_ct_fast_();
        this->read_teleinfo_fast_();

        // Publish fast blocks
        this->publish_smartevse_fast_();
        this->publish_esphome_fast_();
      }

      // Slow cadence (approx 2s)
      if (now - this->last_slow_ >= this->slow_interval_ms_) {
        this->last_slow_ = now;

        // Read energies and tariff/state
        this->read_teleinfo_slow_();

        // Publish slow blocks
        this->publish_smartevse_slow_();
        this->publish_esphome_slow_();
      }
    }

    // ADS constants: PGA 2.048V LSB calculation depends on ADS lib; here we rely on ads_->get_value() as volts
    // Single-shot per channel: read A0 then A1..A3
    void SmartEVSESensorBox::read_ads_ct_fast_() {
      // Measure Vref at A0 (in volts)
      this->vref_meas_ = this->ads_->get_value(ads1115::ADS1115_CHANNEL_0);
      const float vref = this->vref_meas_ > 0.05f ? this->vref_meas_ : this->ads_ref_voltage_;
      const float vref_scale = (vref > 0.05f) ? (this->ads_ref_voltage_ / vref) : 1.0f;

      // Read CT channels (volts). Inputs are biased around Vref; compute differential to Vref.
      const float v_a = this->ads_->get_value(ads1115::ADS1115_CHANNEL_1);
      const float v_b = this->ads_->get_value(ads1115::ADS1115_CHANNEL_2);
      const float v_c = this->ads_->get_value(ads1115::ADS1115_CHANNEL_3);

      const float dv_a = (v_a - vref);
      const float dv_b = (v_b - vref);
      const float dv_c = (v_c - vref);

      // Convert differential voltages to current using front-end:
      // CT 100A/50mA with burden 22Ω and 1k/1k input network biases. For simplicity,
      // map dv (volts) to amps via per-channel gain/offset + vref_scale, letting calibration gains encode exact conversion.
      float i_a = (dv_a + this->ct_offset_a_) * this->ct_gain_a_ * vref_scale;
      float i_b = (dv_b + this->ct_offset_b_) * this->ct_gain_b_ * vref_scale;
      float i_c = (dv_c + this->ct_offset_c_) * this->ct_gain_c_ * vref_scale;

      // Clamp to non-negative for apparent power calculation
      i_a = i_a < 0.f ? 0.f : i_a;
      i_b = i_b < 0.f ? 0.f : i_b;
      i_c = i_c < 0.f ? 0.f : i_c;

      this->ct_a_curr_a_ = i_a;
      this->ct_b_curr_a_ = i_b;
      this->ct_c_curr_a_ = i_c;

      const float i_total = i_a + i_b + i_c;
      const float s_total = i_total * this->nominal_voltage_phase_ * this->power_factor_;
      this->ct_total_va_ = s_total > 0.f ? static_cast<uint32_t>(s_total) : 0U;

      ESP_LOGD(TAG, "ADS: vref=%.3fV, CT A=%.3fA B=%.3fA C=%.3fA, S=%uVA", vref, i_a, i_b, i_c, this->ct_total_va_);
    }

    void SmartEVSESensorBox::read_teleinfo_fast_() {
      if (!this->tic_) return;

      // Instantaneous values
      this->sinsts_va_ = this->tic_->get_u32("SINSTS");     // VA
      this->sinst1_va_ = this->tic_->get_u32("SINST1");     // VA
      this->sinst2_va_ = this->tic_->get_u32("SINST2");     // VA
      this->sinst3_va_ = this->tic_->get_u32("SINST3");     // VA

      this->irms1_a_ = this->tic_->get_u16("IRMS1");        // A
      this->irms2_a_ = this->tic_->get_u16("IRMS2");        // A
      this->irms3_a_ = this->tic_->get_u16("IRMS3");        // A

      this->urms1_v_ = this->tic_->get_u16("URMS1");        // V
      this->urms2_v_ = this->tic_->get_u16("URMS2");        // V
      this->urms3_v_ = this->tic_->get_u16("URMS3");        // V
    }

    void SmartEVSESensorBox::read_teleinfo_slow_() {
      if (!this->tic_) return;

      // Energies and max power, tariffs and states
      this->east_wh_    = this->tic_->get_u32("EAST");     // Wh
      this->eait_wh_    = this->tic_->get_u32("EAIT");     // Wh
      this->easf01_wh_  = this->tic_->get_u32("EASF01");   // Wh
      this->easf02_wh_  = this->tic_->get_u32("EASF02");   // Wh
      this->easf03_wh_  = this->tic_->get_u32("EASF03");   // Wh
      this->easf04_wh_  = this->tic_->get_u32("EASF04");   // Wh
      this->easf05_wh_  = this->tic_->get_u32("EASF05");   // Wh
      this->easf06_wh_  = this->tic_->get_u32("EASF06");   // Wh

      this->smaxsn_va_  = this->tic_->get_u32("SMAXSN");   // VA
      this->smaxsn1_va_ = this->tic_->get_u32("SMAXSN1");  // VA
      this->smaxsn2_va_ = this->tic_->get_u32("SMAXSN2");  // VA
      this->smaxsn3_va_ = this->tic_->get_u32("SMAXSN3");  // VA
      this->pcoup_va_   = this->tic_->get_u32("PCOUP");    // VA
      this->pref_kva_   = this->tic_->get_u16("PREF");     // kVA
      this->ccasn_w_    = this->tic_->get_u32("CCASN");    // W

      this->ntarf_      = this->tic_->get_u16("NTARF");
      this->njourf_     = this->tic_->get_u16("NJOURF");
      this->njourf_p1_  = this->tic_->get_u16("NJOURF+1");
      this->ngtf_       = this->tic_->get_u64("NGTF");
      this->ltarf_      = this->tic_->get_u64("LTARF");
      this->pjourf_p1_  = this->tic_->get_u64("PJOURF+1");
      this->ppointe_    = this->tic_->get_u64("PPOINTE");
    }

    // Publish fast block to SmartEVSE SensorBox-2 bus (addresses as per SensorBox-2 map)
    void SmartEVSESensorBox::publish_smartevse_fast_() {
      if (!this->mb_smartevse_) return;

      // Example mapping; you must ensure exact addresses/types per SensorBox-2 reference
      // Currents (A) and voltages (V)
      write_u16(this->mb_smartevse_, 0x0100, static_cast<uint16_t>(this->ct_a_curr_a_));  // CT1
      write_u16(this->mb_smartevse_, 0x0101, static_cast<uint16_t>(this->ct_b_curr_a_));  // CT2
      write_u16(this->mb_smartevse_, 0x0102, static_cast<uint16_t>(this->ct_c_curr_a_));  // CT3

      write_u16(this->mb_smartevse_, 0x0110, this->urms1_v_);
      write_u16(this->mb_smartevse_, 0x0111, this->urms2_v_);
      write_u16(this->mb_smartevse_, 0x0112, this->urms3_v_);

      // Total apparent power (VA)
      write_u32(this->mb_smartevse_, 0x0120, this->prefer_linky_power_ && this->sinsts_va_ ? this->sinsts_va_ : this->ct_total_va_);
    }

    // Publish slow block to SmartEVSE SensorBox-2 bus (config and slower-changing metrics)
    void SmartEVSESensorBox::publish_smartevse_slow_() {
      if (!this->mb_smartevse_) return;

      // Config bitfield (0x0800): bit0 rotation, bit1 wire_mode (3/4-wire)
      const uint16_t cfg = (static_cast<uint16_t>(this->rotation_) & 0x01)
      | ((static_cast<uint16_t>(this->wire_mode_) & 0x01) << 1);
      write_u16(this->mb_smartevse_, 0x0800, cfg);
      write_u16(this->mb_smartevse_, 0x0801, static_cast<uint16_t>(this->wifi_mode_));
    }

    // Publish fast block to ESPHome Linky Modbus bus (decimal addresses)
    void SmartEVSESensorBox::publish_esphome_fast_() {
      if (!this->mb_esphome_) return;

      // Currents and voltages from TIC
      write_u16(this->mb_esphome_, 24, this->irms1_a_);
      write_u16(this->mb_esphome_, 25, this->irms2_a_);
      write_u16(this->mb_esphome_, 26, this->irms3_a_);
      write_u16(this->mb_esphome_, 27, this->urms1_v_);
      write_u16(this->mb_esphome_, 28, this->urms2_v_);
      write_u16(this->mb_esphome_, 29, this->urms3_v_);

      // Instantaneous apparent powers
      write_u32(this->mb_esphome_, 30, this->sinsts_va_);
      write_u32(this->mb_esphome_, 32, this->sinst1_va_);
      write_u32(this->mb_esphome_, 34, this->sinst2_va_);
      write_u32(this->mb_esphome_, 36, this->sinst3_va_);

      // CT extras: 2000..2003
      write_u16(this->mb_esphome_, 2000, static_cast<uint16_t>(this->ct_a_curr_a_));
      write_u16(this->mb_esphome_, 2001, static_cast<uint16_t>(this->ct_b_curr_a_));
      write_u16(this->mb_esphome_, 2002, static_cast<uint16_t>(this->ct_c_curr_a_));
      write_u32(this->mb_esphome_, 2003, this->ct_total_va_);
    }

    // Publish slow block to ESPHome Linky Modbus bus (decimal addresses)
    void SmartEVSESensorBox::publish_esphome_slow_() {
      if (!this->mb_esphome_) return;

      // Energies
      write_u32(this->mb_esphome_, 0,  this->east_wh_);
      write_u32(this->mb_esphome_, 2,  this->eait_wh_);
      write_u32(this->mb_esphome_, 4,  this->easf01_wh_);
      write_u32(this->mb_esphome_, 6,  this->easf02_wh_);
      write_u32(this->mb_esphome_, 8,  this->easf01_wh_);  // EJP Normal (uses EASF01 per your table)
      write_u32(this->mb_esphome_, 10, this->easf02_wh_);  // EJP Pointe (uses EASF02)
      write_u32(this->mb_esphome_, 12, this->easf01_wh_);  // BLEU HC
      write_u32(this->mb_esphome_, 14, this->easf02_wh_);  // BLEU HP
      write_u32(this->mb_esphome_, 16, this->easf03_wh_);  // BLANC HC
      write_u32(this->mb_esphome_, 18, this->easf04_wh_);  // BLANC HP
      write_u32(this->mb_esphome_, 20, this->easf05_wh_);  // ROUGE HC
      write_u32(this->mb_esphome_, 22, this->easf06_wh_);  // ROUGE HP

      // Max powers and params
      write_u32(this->mb_esphome_, 38, this->smaxsn_va_);
      write_u32(this->mb_esphome_, 40, this->smaxsn1_va_);
      write_u32(this->mb_esphome_, 42, this->smaxsn2_va_);
      write_u32(this->mb_esphome_, 44, this->smaxsn3_va_);
      write_u32(this->mb_esphome_, 46, this->pcoup_va_);
      write_u16(this->mb_esphome_, 48, this->pref_kva_);
      write_u32(this->mb_esphome_, 49, this->ccasn_w_);

      // Tariff/state
      write_u16(this->mb_esphome_, 51, this->ntarf_);
      write_u16(this->mb_esphome_, 52, this->njourf_);
      write_u16(this->mb_esphome_, 53, this->njourf_p1_);
      write_u64(this->mb_esphome_, 54, this->ngtf_);
      write_u64(this->mb_esphome_, 58, this->ltarf_);
      write_u64(this->mb_esphome_, 62, this->pjourf_p1_);
      write_u64(this->mb_esphome_, 66, this->ppointe_);

      // Config extras
      const uint16_t cfg = (static_cast<uint16_t>(this->rotation_) & 0x01)
      | ((static_cast<uint16_t>(this->wire_mode_) & 0x01) << 1);
      write_u16(this->mb_esphome_, 2048, cfg);
      write_u16(this->mb_esphome_, 2049, static_cast<uint16_t>(this->wifi_mode_));
    }

    void SmartEVSESensorBox::dump_config() {
      ESP_LOGCONFIG(TAG, "SmartEVSE SensorBox");
      ESP_LOGCONFIG(TAG, "  ADS1115: %s", (this->ads_ ? "OK" : "MISSING"));
      ESP_LOGCONFIG(TAG, "  Teleinfo: %s", (this->tic_ ? "OK" : "MISSING"));
      ESP_LOGCONFIG(TAG, "  Modbus SmartEVSE: %s", (this->mb_smartevse_ ? "OK" : "MISSING"));
      ESP_LOGCONFIG(TAG, "  Modbus ESPHome: %s", (this->mb_esphome_ ? "OK" : "MISSING"));
      ESP_LOGCONFIG(TAG, "  Fast interval: %u ms  Slow interval: %u ms", this->fast_interval_ms_, this->slow_interval_ms_);
      ESP_LOGCONFIG(TAG, "  Nominal V: %.1f  PF: %.2f", this->nominal_voltage_phase_, this->power_factor_);
    }

  }  // namespace smartevse_sensorbox
}  // namespace esphome
