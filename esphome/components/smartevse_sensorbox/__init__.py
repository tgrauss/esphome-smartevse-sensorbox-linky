import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID

smartevse_ns = cg.global_ns.namespace("esphome").namespace("smartevse_sensorbox")
SmartEVSESensorBox = smartevse_ns.class_("SmartEVSESensorBox", cg.PollingComponent)

# Configuration keys
CONF_CT_PHASE_A = "ct_phase_a"
CONF_CT_PHASE_B = "ct_phase_b"
CONF_CT_PHASE_C = "ct_phase_c"
CONF_ADS_REF = "ads_ref"

# TIC inputs
CONF_LINKY_POWER = "linky_power"     # SINSTS
CONF_LINKY_ENERGY = "linky_energy"   # EAST
CONF_LINKY_CURRENT_L1 = "linky_current_l1"  # IRMS1
CONF_LINKY_CURRENT_L2 = "linky_current_l2"  # IRMS2
CONF_LINKY_CURRENT_L3 = "linky_current_l3"  # IRMS3
CONF_LINKY_VOLTAGE_L1 = "linky_voltage_l1"  # URMS1
CONF_LINKY_VOLTAGE_L2 = "linky_voltage_l2"  # URMS2
CONF_LINKY_VOLTAGE_L3 = "linky_voltage_l3"  # URMS3

# Calibration
CONF_CT_GAIN_A = "ct_gain_a"
CONF_CT_GAIN_B = "ct_gain_b"
CONF_CT_GAIN_C = "ct_gain_c"
CONF_CT_OFFSET_A = "ct_offset_a"
CONF_CT_OFFSET_B = "ct_offset_b"
CONF_CT_OFFSET_C = "ct_offset_c"

CONF_NOMINAL_VOLTAGE = "nominal_voltage"
CONF_POWER_FACTOR = "power_factor"
CONF_THREE_PHASE = "three_phase"
CONF_PREFER_LINKY_POWER = "prefer_linky_power"
CONF_AUTOCALIBRATION = "autocalibration"
CONF_ADS_REF_VOLTAGE = "ads_ref_voltage"

# Configurable registers
CONF_ROTATION = "rotation"
CONF_WIRE_MODE = "wire_mode"
CONF_WIFI_MODE = "wifi_mode"  # 0=disabled, 1=enabled, 2=portal

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SmartEVSESensorBox),

    cv.Required(CONF_CT_PHASE_A): cv.use_id(sensor.Sensor),
    cv.Required(CONF_CT_PHASE_B): cv.use_id(sensor.Sensor),
    cv.Required(CONF_CT_PHASE_C): cv.use_id(sensor.Sensor),
    cv.Required(CONF_ADS_REF): cv.use_id(sensor.Sensor),

    # TIC sensors
    cv.Optional(CONF_LINKY_POWER): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_LINKY_ENERGY): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_LINKY_CURRENT_L1): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_LINKY_CURRENT_L2): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_LINKY_CURRENT_L3): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_LINKY_VOLTAGE_L1): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_LINKY_VOLTAGE_L2): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_LINKY_VOLTAGE_L3): cv.use_id(sensor.Sensor),

    # Calibration
    cv.Optional(CONF_CT_GAIN_A, default=1.0): cv.float_,
    cv.Optional(CONF_CT_GAIN_B, default=1.0): cv.float_,
    cv.Optional(CONF_CT_GAIN_C, default=1.0): cv.float_,
    cv.Optional(CONF_CT_OFFSET_A, default=0.0): cv.float_,
    cv.Optional(CONF_CT_OFFSET_B, default=0.0): cv.float_,
    cv.Optional(CONF_CT_OFFSET_C, default=0.0): cv.float_,

    cv.Optional(CONF_ADS_REF_VOLTAGE, default=1.0): cv.float_,
    cv.Optional(CONF_AUTOCALIBRATION, default=True): cv.boolean,

    cv.Optional(CONF_NOMINAL_VOLTAGE, default=230.0): cv.float_range(min=100.0, max=270.0),
    cv.Optional(CONF_POWER_FACTOR, default=0.95): cv.float_range(min=0.0, max=1.0),
    cv.Optional(CONF_THREE_PHASE, default=False): cv.boolean,
    cv.Optional(CONF_PREFER_LINKY_POWER, default=True): cv.boolean,

    # Configurable registers
    cv.Optional(CONF_ROTATION, default=0): cv.int_range(min=0, max=1),
    cv.Optional(CONF_WIRE_MODE, default=1): cv.int_range(min=0, max=1),
    cv.Optional(CONF_WIFI_MODE, default=1): cv.int_range(min=0, max=2),
}).extend(cv.polling_component_schema("1s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Inputs CT
    ct_a = await cg.get_variable(config[CONF_CT_PHASE_A])
    ct_b = await cg.get_variable(config[CONF_CT_PHASE_B])
    ct_c = await cg.get_variable(config[CONF_CT_PHASE_C])
    ads_ref = await cg.get_variable(config[CONF_ADS_REF])

    cg.add(var.set_ct_phase_a_input(ct_a))
    cg.add(var.set_ct_phase_b_input(ct_b))
    cg.add(var.set_ct_phase_c_input(ct_c))
    cg.add(var.set_ads_ref_input(ads_ref))

    # TIC inputs
    if CONF_LINKY_POWER in config:
        lp = await cg.get_variable(config[CONF_LINKY_POWER])
        cg.add(var.set_linky_power_input(lp))
    if CONF_LINKY_ENERGY in config:
        le = await cg.get_variable(config[CONF_LINKY_ENERGY])
        cg.add(var.set_linky_energy_input(le))
    if CONF_LINKY_CURRENT_L1 in config:
        lc1 = await cg.get_variable(config[CONF_LINKY_CURRENT_L1])
        cg.add(var.set_linky_current_l1(lc1))
    if CONF_LINKY_CURRENT_L2 in config:
        lc2 = await cg.get_variable(config[CONF_LINKY_CURRENT_L2])
        cg.add(var.set_linky_current_l2(lc2))
    if CONF_LINKY_CURRENT_L3 in config:
        lc3 = await cg.get_variable(config[CONF_LINKY_CURRENT_L3])
        cg.add(var.set_linky_current_l3(lc3))
    if CONF_LINKY_VOLTAGE_L1 in config:
        lv1 = await cg.get_variable(config[CONF_LINKY_VOLTAGE_L1])
        cg.add(var.set_linky_voltage_l1(lv1))
    if CONF_LINKY_VOLTAGE_L2 in config:
        lv2 = await cg.get_variable(config[CONF_LINKY_VOLTAGE_L2])
        cg.add(var.set_linky_voltage_l2(lv2))
    if CONF_LINKY_VOLTAGE_L3 in config:
        lv3 = await cg.get_variable(config[CONF_LINKY_VOLTAGE_L3])
        cg.add(var.set_linky_voltage_l3(lv3))

    # Calibration
    cg.add(var.set_ct_gain_a(config[CONF_CT_GAIN_A]))
    cg.add(var.set_ct_gain_b(config[CONF_CT_GAIN_B]))
    cg.add(var.set_ct_gain_c(config[CONF_CT_GAIN_C]))
    cg.add(var.set_ct_offset_a(config[CONF_CT_OFFSET_A]))
    cg.add(var.set_ct_offset_b(config[CONF_CT_OFFSET_B]))
    cg.add(var.set_ct_offset_c(config[CONF_CT_OFFSET_C]))

    cg.add(var.set_ads_ref_voltage(config[CONF_ADS_REF_VOLTAGE]))
    cg.add(var.enable_autocalibration(config[CONF_AUTOCALIBRATION]))

    # Power computation config
    cg.add(var.set_nominal_voltage_phase(config[CONF_NOMINAL_VOLTAGE]))
    cg.add(var.set_power_factor(config[CONF_POWER_FACTOR]))
    cg.add(var.set_three_phase(config[CONF_THREE_PHASE]))
    cg.add(var.set_prefer_linky_power(config[CONF_PREFER_LINKY_POWER]))

    # Configurable registers
    cg.add(var.set_rotation(config[CONF_ROTATION]))
    cg.add(var.set_wire_mode(config[CONF_WIRE_MODE]))
    cg.add(var.set_wifi_mode(config[CONF_WIFI_MODE]))

    # Outputs (sensors auto-créés)
    await sensor.new_sensor(var.ct_phase_a_out)
    await sensor.new_sensor(var.ct_phase_b_out)
    await sensor.new_sensor(var.ct_phase_c_out)
    await sensor.new_sensor(var.ct_total_current_out)
    await sensor.new_sensor(var.ct_total_power_out)
    await sensor.new_sensor(var.linky_power_out)
    await sensor.new_sensor(var.linky_energy_out)
    await sensor.new_sensor(var.prefer_ct_out)

    # Extra registers for Sensorbox-V2 compatibility
    await sensor.new_sensor(var.version_out)
    await sensor.new_sensor(var.dsmr_info_out)
    await sensor.new_sensor(var.voltage_l1_out)
    await sensor.new_sensor(var.voltage_l2_out)
    await sensor.new_sensor(var.voltage_l3_out)
    await sensor.new_sensor(var.p1_current_l1_out)
    await sensor
