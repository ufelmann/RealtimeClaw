import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import microphone, speaker
from esphome.const import CONF_ID, CONF_HOST, CONF_PORT

DEPENDENCIES = ["microphone"]
AUTO_LOAD = []

wyoming_tcp_client_ns = cg.esphome_ns.namespace("wyoming_tcp_client")
WyomingTcpClient = wyoming_tcp_client_ns.class_(
    "WyomingTcpClient", cg.Component
)

CONF_SPEAKER = "speaker"
CONF_MICROPHONE = "microphone"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WyomingTcpClient),
        cv.Required(CONF_HOST): cv.string,
        cv.Optional(CONF_PORT, default=10300): cv.port,
        cv.Required(CONF_MICROPHONE): microphone.microphone_source_schema(),
        cv.Required(CONF_SPEAKER): cv.use_id(speaker.Speaker),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_port(config[CONF_PORT]))

    mic_source = await microphone.microphone_source_to_code(
        config[CONF_MICROPHONE]
    )
    cg.add(var.set_microphone_source(mic_source))

    spk = await cg.get_variable(config[CONF_SPEAKER])
    cg.add(var.set_speaker(spk))
