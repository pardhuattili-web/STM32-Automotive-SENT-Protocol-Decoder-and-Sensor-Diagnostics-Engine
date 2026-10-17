#include "sent_sim.h"

int sent_sim_make_capture(const sent_decoder_config_t *c, uint8_t status,
                          const uint8_t data[SENT_DATA_NIBBLES],
                          uint16_t *capture, size_t capacity)
{
    uint8_t n[5];
    size_t i, crc_count;
    if (c == NULL || data == NULL || capture == NULL || capacity < 6u ||
        status > 15u || data[0] > 15u || data[1] > 15u || data[2] > 15u) return -1;
    n[0] = status; n[1] = data[0]; n[2] = data[1]; n[3] = data[2];
    crc_count = c->crc_includes_status ? 4u : 3u;
    n[4] = sent_decoder_crc4(c, c->crc_includes_status ? n : &n[1], crc_count);
    capture[0] = (uint16_t)(56u * c->nominal_tick_counts);
    for (i = 0u; i < 5u; ++i) capture[i + 1u] = (uint16_t)((12u + n[i]) * c->nominal_tick_counts);
    return 6;
}
