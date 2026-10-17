#include "sent_decoder.h"

#include <string.h>

static bool config_is_valid(const sent_decoder_config_t *config)
{
    size_t i;

    if (config == NULL || config->nominal_tick_counts == 0u ||
        config->tolerance_percent > 50u || config->sync_min_counts == 0u ||
        config->sync_min_counts >= config->sync_max_counts ||
        config->crc_seed > 0x0Fu) {
        return false;
    }

    for (i = 0u; i < 16u; ++i) {
        if (config->crc_table[i] > 0x0Fu) {
            return false;
        }
    }
    return true;
}

sent_result_t sent_decoder_init(sent_decoder_t *decoder,
                                const sent_decoder_config_t *config)
{
    if (decoder == NULL || !config_is_valid(config)) {
        return SENT_RESULT_INVALID_ARGUMENT;
    }

    memset(decoder, 0, sizeof(*decoder));
    decoder->config = *config;
    decoder->diagnostics.signal_present = true;
    return SENT_RESULT_OK;
}

uint8_t sent_decoder_crc4(const sent_decoder_config_t *config,
                          const uint8_t *nibbles,
                          size_t nibble_count)
{
    size_t i;
    uint8_t crc;

    if (config == NULL || (nibbles == NULL && nibble_count != 0u)) {
        return 0u;
    }

    crc = (uint8_t)(config->crc_seed & 0x0Fu);
    for (i = 0u; i < nibble_count; ++i) {
        if (nibbles[i] > 0x0Fu) {
            return 0u;
        }
        crc = config->crc_table[(crc ^ nibbles[i]) & 0x0Fu];
    }
    return (uint8_t)(crc & 0x0Fu);
}

static bool interval_to_nibble(const sent_decoder_config_t *config,
                               uint16_t interval,
                               uint8_t *nibble)
{
    const uint32_t nominal = config->nominal_tick_counts;
    const uint32_t tolerance =
        (nominal * (uint32_t)config->tolerance_percent) / 100u;
    uint32_t units;
    uint32_t residual;
    uint32_t target;
    uint32_t difference;

    if (nibble == NULL || interval < 12u * nominal ||
        interval > 27u * nominal) {
        return false;
    }

    units = interval / nominal;
    residual = interval % nominal;
    if (residual * 2u >= nominal) {
        ++units;
    }
    if (units < 12u || units > 27u) {
        return false;
    }

    target = units * nominal;
    difference = (interval > target) ? (interval - target) : (target - interval);
    if (difference > tolerance) {
        return false;
    }

    *nibble = (uint8_t)(units - 12u);
    return true;
}

sent_result_t sent_decoder_process(sent_decoder_t *decoder,
                                   const uint16_t *capture,
                                   size_t capture_count,
                                   sent_frame_t *out_frame)
{
    uint8_t nibbles[SENT_FRAME_NIBBLES];
    uint8_t computed_crc;
    size_t i;
    size_t crc_count;

    if (decoder == NULL || capture == NULL || out_frame == NULL ||
        capture_count != (size_t)(1u + SENT_FRAME_NIBBLES)) {
        if (decoder != NULL) {
            decoder->diagnostics.last_frame_valid = false;
        }
        return SENT_RESULT_INVALID_ARGUMENT;
    }

    memset(out_frame, 0, sizeof(*out_frame));
    if (capture[0] < decoder->config.sync_min_counts ||
        capture[0] > decoder->config.sync_max_counts) {
        ++decoder->diagnostics.sync_errors;
        decoder->diagnostics.last_frame_valid = false;
        return SENT_RESULT_SYNC_ERROR;
    }

    for (i = 0u; i < SENT_FRAME_NIBBLES; ++i) {
        if (!interval_to_nibble(&decoder->config, capture[i + 1u],
                                &nibbles[i])) {
            ++decoder->diagnostics.pulse_errors;
            decoder->diagnostics.last_frame_valid = false;
            return SENT_RESULT_PULSE_ERROR;
        }
    }

    out_frame->status = nibbles[0];
    out_frame->data[0] = nibbles[1];
    out_frame->data[1] = nibbles[2];
    out_frame->data[2] = nibbles[3];
    out_frame->crc = nibbles[4];

    crc_count = decoder->config.crc_includes_status
                    ? (size_t)(SENT_DATA_NIBBLES + 1u)
                    : (size_t)SENT_DATA_NIBBLES;
    computed_crc = sent_decoder_crc4(
        &decoder->config,
        decoder->config.crc_includes_status ? nibbles : &nibbles[1],
        crc_count);
    out_frame->crc_ok = (computed_crc == out_frame->crc);
    out_frame->sequence = decoder->next_sequence++;
    decoder->diagnostics.signal_present = true;

    if (!out_frame->crc_ok) {
        ++decoder->diagnostics.crc_errors;
        decoder->diagnostics.last_frame_valid = false;
        return SENT_RESULT_CRC_ERROR;
    }

    ++decoder->diagnostics.valid_frames;
    decoder->diagnostics.last_frame_valid = true;
    return SENT_RESULT_OK;
}

void sent_decoder_note_timeout(sent_decoder_t *decoder)
{
    if (decoder == NULL) {
        return;
    }

    ++decoder->diagnostics.timeouts;
    decoder->diagnostics.signal_present = false;
    decoder->diagnostics.last_frame_valid = false;
}
