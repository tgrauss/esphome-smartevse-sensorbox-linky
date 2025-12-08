import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Dependencies
from esphome.components import ads1115
from esphome.components import teleinfo
from esphome.components import uart
from esphome.components import modbus_server

# Namespaces/classes (match C++)
smartevse_ns = cg.global_ns.namespace("esphome").namespace("smartevse_sensorbox")
SmartEVSESensorBox = smartevse_ns.class_("SmartEVSESensorBox", cg.PollingComponent)

# Config keys
CONF_ADS1115_ID = "ads1115_id"
CONF_TELEINFO_ID = "teleinfo_id"
CONF_MODBUS_SMARTEVSE_ID = "modbus_smartevse_id"
CONF_MODBUS_ESPHOME_ID = "modbus_esphome_id"

CONF_THREE_PHASE = "three_phase"
CONF_PREFER_LINKY_POWER = "prefer_linky_power"
CONF_ROTATION = "rotation"      # bit0
CONF_WIRE_MODE = "wire_mode"    # bit1 (3/4 wire)
CONF_WIFI_MODE = "wifi_mode"    # 0 off, 1 on, 2 portal

# ADS calibration and scaling
CONF_CT_GAIN_A = "ct_gain_a"
CONF_CT_GAIN_B = "ct_gain_b"
CONF_CT_GAIN_C = "ct_gain_c"
CONF_CT_OFFSET_A = "ct_offset_a"
CONF_CT_OFFSET_B = "ct_offset_b"
CONF_CT_OFFSET_C = "ct_offset_c"
CONF_ADS_REF_VOLTAGE = "ads_ref_voltage"
CONF_NOMINAL_VOLTAGE = "nominal_voltage"
CONF_POWER_FACTOR = "power_factor"

# Polling intervals
CONF_FAST_INTERVAL_MS = "fast_interval_ms"  # match SensorBox-2 fast block cadence
CONF_SLOW_INTERVAL_MS = "slow_interval_ms"  # match SensorBox-2 slow block cadence

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SmartEVSESensorBox),

    # Required component dependencies
    cv.Required(CONF_ADS1115_ID): cv.use_id(ads1115.ADS1115Component),
    cv.Required(CONF_TELEINFO_ID): cv.use_id(teleinfo.TeleinfoComponent),
    cv.Required(CONF_MODBUS_SMARTEVSE_ID): cv.use_id(modbus_server.ModbusServerComponent),
    cv.Required(CONF_MODBUS_ESPHOME_ID): cv.use_id(modbus_server.ModbusServerComponent),

    # Operation mode and preferences
    cv.Optional(CONF_THREE_PHASE, default=False): cv.boolean,
    cv.Optional(CONF_PREFER_LINKY_POWER, default=True): cv.boolean,

    # Config bitfield and wifi mode
    cv.Optional(CONF_ROTATION, default=0): cv.int_range(min=0, max=1),
    cv.Optional(CONF_WIRE_MODE, default=1): cv.int_range(min=0, max=1),
    cv.Optional(CONF_WIFI_MODE, default=1): cv.int_range(min=0, max=2),

    # ADS calibration/scaling
    cv.Optional(CONF_CT_GAIN_A, default=1.0): cv.float_,
    cv.Optional(CONF_CT_GAIN_B, default=1.0): cv.float_,
    cv.Optional(CONF_CT_GAIN_C, default=1.0): cv.float_,
    cv.Optional(CONF_CT_OFFSET_A, default=0.0): cv.float_,
    cv.Optional(CONF_CT_OFFSET_B, default=0.0): cv.float_,
    cv.Optional(CONF_CT_OFFSET_C, default=0.0): cv.float_,
    cv.Optional(CONF_ADS_REF_VOLTAGE, default=1.0): cv.float_,  # A0 nominal
    cv.Optional(CONF_NOMINAL_VOLTAGE, default=230.0): cv.float_range(min=100.0, max=270.0),
    cv.Optional(CONF_POWER_FACTOR, default=0.95): cv.float_range(min=0.0, max=1.0),

    # Cadence: follow SensorBox-2 timing (fast vs slow blocks)
    cv.Optional(CONF_FAST_INTERVAL_MS, default=1000): cv.int_range(min=100, max=10000),
    cv.Optional(CONF_SLOW_INTERVAL_MS, default=2000): cv.int_range(min=500, max=60000),
}).extend(cv.polling_component_schema("1s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Dependencies
    ads = await cg.get_variable(config[CONF_ADS1115_ID])
    cg.add(var.set_ads1115(ads))

    tic = await cg.get_variable(config[CONF_TELEINFO_ID])
    cg.add(var.set_teleinfo(tic))

    mb_sve = await cg.get_variable(config[CONF_MODBUS_SMARTEVSE_ID])
    cg.add(var.set_modbus_smartevse(mb_sve))

    mb_esp = await cg.get_variable(config[CONF_MODBUS_ESPHOME_ID])
    cg.add(var.set_modbus_esphome(mb_esp))

    # Operation/config
    cg.add(var.set_three_phase(config[CONF_THREE_PHASE]))
    cg.add(var.set_prefer_linky_power(config[CONF_PREFER_LINKY_POWER]))

    cg.add(var.set_rotation(config[CONF_ROTATION]))
    cg.add(var.set_wire_mode(config[CONF_WIRE_MODE]))
    cg.add(var.set_wifi_mode(config[CONF_WIFI_MODE]))

    # ADS calibration/scaling
    cg.add(var.set_ct_gain_a(config[CONF_CT_GAIN_A]))
    cg.add(var.set_ct_gain_b(config[CONF_CT_GAIN_B]))
    cg.add(var.set_ct_gain_c(config[CONF_CT_GAIN_C]))
    cg.add(var.set_ct_offset_a(config[CONF_CT_OFFSET_A]))
    cg.add(var.set_ct_offset_b(config[CONF_CT_OFFSET_B]))
    cg.add(var.set_ct_offset_c(config[CONF_CT_OFFSET_C]))
    cg.add(var.set_ads_ref_voltage(config[CONF_ADS_REF_VOLTAGE]))
    cg.add(var.set_nominal_voltage_phase(config[CONF_NOMINAL_VOLTAGE]))
    cg.add(var.set_power_factor(config[CONF_POWER_FACTOR]))

    # Cadence (fast/slow blocks)
    cg.add(var.set_fast_interval_ms(config[CONF_FAST_INTERVAL_MS]))
    cg.add(var.set_slow_interval_ms(config[CONF_SLOW_INTERVAL_MS]))
