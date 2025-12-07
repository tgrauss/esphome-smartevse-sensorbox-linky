import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Namespaces
smartevse_modbus_ns = cg.global_ns.namespace("esphome").namespace("smartevse_modbus")
SmartEVSEModbusServer = smartevse_modbus_ns.class_("SmartEVSEModbusServer", cg.Component)

modbus_server_ns = cg.global_ns.namespace("esphome").namespace("modbus_server")
ModbusServer = modbus_server_ns.class_("ModbusServer", cg.Component)

sensorbox_ns = cg.global_ns.namespace("esphome").namespace("smartevse_sensorbox")
SmartEVSESensorBox = sensorbox_ns.class_("SmartEVSESensorBox", cg.PollingComponent)

# Config keys
CONF_MODBUS_SERVER_ID = "modbus_server_id"
CONF_SENSORBOX_ID = "sensorbox_id"
CONF_PROFILE = "profile"

PROFILE_ENUM = cv.one_of("smartevse_v2", "linky_modbus", lower=True)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SmartEVSEModbusServer),
    cv.Required(CONF_MODBUS_SERVER_ID): cv.use_id(ModbusServer),
    cv.Required(CONF_SENSORBOX_ID): cv.use_id(SmartEVSESensorBox),
    cv.Optional(CONF_PROFILE, default="smartevse_v2"): PROFILE_ENUM,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Bind Modbus server
    server = await cg.get_variable(config[CONF_MODBUS_SERVER_ID])
    cg.add(var.set_modbus_server(server))

    # Bind SensorBox
    sensorbox = await cg.get_variable(config[CONF_SENSORBOX_ID])

    # Link all sensors from SensorBox
    cg.add(var.set_version(sensorbox.version_out))
    cg.add(var.set_dsmr_info(sensorbox.dsmr_info_out))
    cg.add(var.set_voltage_l1(sensorbox.voltage_l1_out))
    cg.add(var.set_voltage_l2(sensorbox.voltage_l2_out))
    cg.add(var.set_voltage_l3(sensorbox.voltage_l3_out))
    cg.add(var.set_p1_current_l1(sensorbox.p1_current_l1_out))
    cg.add(var.set_p1_current_l2(sensorbox.p1_current_l2_out))
    cg.add(var.set_p1_current_l3(sensorbox.p1_current_l3_out))
    cg.add(var.set_ct_phase_a(sensorbox.ct_phase_a_out))
    cg.add(var.set_ct_phase_b(sensorbox.ct_phase_b_out))
    cg.add(var.set_ct_phase_c(sensorbox.ct_phase_c_out))
    cg.add(var.set_ct_total_current(sensorbox.ct_total_current_out))
    cg.add(var.set_ct_total_power(sensorbox.ct_total_power_out))
    cg.add(var.set_linky_power(sensorbox.linky_power_out))
    cg.add(var.set_linky_energy(sensorbox.linky_energy_out))
    cg.add(var.set_wifi_status(sensorbox.wifi_status_out))
    cg.add(var.set_time_hm(sensorbox.time_hm_out))
    cg.add(var.set_time_md(sensorbox.time_md_out))
    cg.add(var.set_time_yw(sensorbox.time_yw_out))
    cg.add(var.set_ip(sensorbox.ip_out))
    cg.add(var.set_mac(sensorbox.mac_out))
    cg.add(var.set_portal_pwd(sensorbox.portal_pwd_out))
    cg.add(var.set_rotation(sensorbox.rotation_out))
    cg.add(var.set_wifi_mode(sensorbox.wifi_mode_out))

    # Set profile
    profile = config.get(CONF_PROFILE, "smartevse_v2")
    cg.add(var.set_profile(profile))
