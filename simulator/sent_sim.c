#include "sent_sim.h"

int sent_sim_make_capture(const sent_decoder_config_t *config,
                          uint8_t status,
                          const uint8_t data[SENT_DATA_NIBBLES],
                          uint16_t *capture,
                          size_t capture_capacity)
{
    uint8_t nibbles[SENT_FRAME_NIBBLES];
    size_t i;
    size_t crc_count;
    uint32_t sync_counts;

    if (config == NULL || data == NULL || capture == NULL ||
        capture_capacity < (size_t)(1u + SENT_FRAME_NIBBLES) ||
        config->nominal_tick_counts == 0u || status > 0x0Fu ||
        data[0] > 0x0Fu || data[1] > 0x0Fu || data[2] > 0x0Fu) {
        return -1;
    }

    sync_counts = 56u * (uint32_t)config->nominal_tick_counts;
    if (sync_counts > UINT16_MAX) {
        return -1;
    }

    nibbles[0] = status;
    nibbles[1] = data[0];
    nibbles[2] = data[1];
    nibbles[3] = data[2];

    crc_count = config->crc_includes_status
                    ? (size_t)(SENT_DATA_NIBBLES + 1u)
                    : (size_t)SENT_DATA_NIBBLES;
    nibbles[4] = sent_decoder_crc4(
        config,
        config->crc_includes_status ? nibbles : &nibbles[1],
        crc_count);

    capture[0] = (uint16_t)sync_counts;
    for (i = 0u; i < SENT_FRAME_NIBBLES; ++i) {
        uint32_t interval = (12u + (uint32_t)nibbles[i]) *
                            (uint32_t)config->nominal_tick_counts;
        if (interval > UINT16_MAX) {
            return -1;
        }
        capture[i + 1u] = (uint16_t)interval;
    }

    return (int)(1u + SENT_FRAME_NIBBLES);
}
