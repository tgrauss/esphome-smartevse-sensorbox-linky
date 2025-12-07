import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Namespaces
smartevse_modbus_ns = cg.global_ns.namespace("esphome").namespace("smartevse_modbus")
SmartEVSEModbus = smartevse_modbus_ns.class_("SmartEVSEModbus", cg.Component)

modbus_server_ns = cg.global_ns.namespace("esphome").namespace("modbus_server")
ModbusServer = modbus_server_ns.class_("ModbusServer", cg.Component)

sensorbox_ns = cg.global_ns.namespace("esphome").namespace("smartevse_sensorbox")
SmartEVSESensorBox = sensorbox_ns.class_("SmartEVSESensorBox", cg.PollingComponent)

# Config keys
CONF_MODBUS_SERVER_ID = "modbus_server_id"
CONF_SENSORBOX_ID = "sensorbox_id"
CONF_PROFILE = "profile"

# Profils disponibles
PROFILE_ENUM = cv.one_of("smartevse_v2", "linky_modbus", lower=True)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SmartEVSEModbus),
    cv.Required(CONF_MODBUS_SERVER_ID): cv.use_id(ModbusServer),
    cv.Required(CONF_SENSORBOX_ID): cv.use_id(SmartEVSESensorBox),
    cv.Optional(CONF_PROFILE, default="smartevse_v2"): PROFILE_ENUM,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Bind Modbus server
    server = await cg.get_variable(config[CONF_MODBUS_SERVER_ID])
    cg.add(var.set_modbus_server(server))

    # Bind SensorBox
    sensorbox = await cg.get_variable(config[CONF_SENSORBOX_ID])
    cg.add(var.set_sensorbox(sensorbox))

    # Relier les sorties principales de la SensorBox
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
    cg.add(var.set_ct_power_phase_a(sensorbox.ct_power_phase_a_out))
    cg.add(var.set_ct_power_phase_b(sensorbox.ct_power_phase_b_out))
    cg.add(var.set_ct_power_phase_c(sensorbox.ct_power_phase_c_out))
    cg.add(var.set_linky_power(sensorbox.linky_power_out))
    cg.add(var.set_linky_energy(sensorbox.linky_energy_out))

    # Sorties Teleinfo (pour profil linky_modbus)
    cg.add(var.set_eait(sensorbox.eait_out))
    cg.add(var.set_easf01(sensorbox.easf01_out))
    cg.add(var.set_easf02(sensorbox.easf02_out))
    cg.add(var.set_easf03(sensorbox.easf03_out))
    cg.add(var.set_easf04(sensorbox.easf04_out))
    cg.add(var.set_easf05(sensorbox.easf05_out))
    cg.add(var.set_easf06(sensorbox.easf06_out))
    cg.add(var.set_easf07(sensorbox.easf07_out))
    cg.add(var.set_easf08(sensorbox.easf08_out))
    cg.add(var.set_easf09(sensorbox.easf09_out))
    cg.add(var.set_easf10(sensorbox.easf10_out))
    cg.add(var.set_irms1(sensorbox.irms1_out))
    cg.add(var.set_irms2(sensorbox.irms2_out))
    cg.add(var.set_irms3(sensorbox.irms3_out))
    cg.add(var.set_urms1(sensorbox.urms1_out))
    cg.add(var.set_urms2(sensorbox.urms2_out))
    cg.add(var.set_urms3(sensorbox.urms3_out))
    cg.add(var.set_sinsts(sensorbox.sinsts_out))
    cg.add(var.set_sinsts1(sensorbox.sinsts1_out))
    cg.add(var.set_sinsts2(sensorbox.sinsts2_out))
    cg.add(var.set_sinsts3(sensorbox.sinsts3_out))
    cg.add(var.set_smaxsn(sensorbox.smaxsn_out))
    cg.add(var.set_smaxsn1(sensorbox.smaxsn1_out))
    cg.add(var.set_smaxsn2(sensorbox.smaxsn2_out))
    cg.add(var.set_smaxsn3(sensorbox.smaxsn3_out))
    cg.add(var.set_pcoup(sensorbox.pcoup_out))
    cg.add(var.set_pref(sensorbox.pref_out))
    cg.add(var.set_ccasn(sensorbox.ccasn_out))
    cg.add(var.set_ccasn_1(sensorbox.ccasn_1_out))
    cg.add(var.set_ntarf(sensorbox.ntarf_out))
    cg.add(var.set_ltarf(sensorbox.ltarf_out))
    cg.add(var.set_njourf1(sensorbox.njourf_out))
    cg.add(var.set_njourf1(sensorbox.njourf1_out))
    cg.add(var.set_ngtf(sensorbox.ngtf_out))
    cg.add(var.set_pjourf1(sensorbox.pjourf1_out))
    cg.add(var.set_ppointe(sensorbox.ppointe_out))

    # Sorties diverses
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
