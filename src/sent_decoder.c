#include "sent_decoder.h"
#include <string.h>

static bool config_is_valid(const sent_decoder_config_t *c)
{
    size_t i;
    if (c == NULL || c->nominal_tick_counts == 0u ||
        c->tolerance_percent > 50u || c->sync_min_counts == 0u ||
        c->sync_min_counts >= c->sync_max_counts || c->crc_seed > 15u) return false;
    for (i = 0u; i < 16u; ++i) if (c->crc_table[i] > 15u) return false;
    return true;
}

sent_result_t sent_decoder_init(sent_decoder_t *d, const sent_decoder_config_t *c)
{
    if (d == NULL || !config_is_valid(c)) return SENT_RESULT_INVALID_ARGUMENT;
    memset(d, 0, sizeof(*d));
    d->config = *c;
    d->diagnostics.signal_present = true;
    return SENT_RESULT_OK;
}

uint8_t sent_decoder_crc4(const sent_decoder_config_t *c, const uint8_t *n, size_t count)
{
    size_t i;
    uint8_t crc;
    if (c == NULL || (n == NULL && count != 0u)) return 0u;
    crc = (uint8_t)(c->crc_seed & 15u);
    for (i = 0u; i < count; ++i) {
        if (n[i] > 15u) return 0u;
        crc = c->crc_table[(crc ^ n[i]) & 15u];
    }
    return (uint8_t)(crc & 15u);
}

static bool to_nibble(const sent_decoder_config_t *c, uint16_t interval, uint8_t *out)
{
    uint32_t nominal = c->nominal_tick_counts;
    uint32_t tolerance = (nominal * c->tolerance_percent) / 100u;
    uint32_t units, residual, target;
    if (out == NULL || interval < 12u * nominal || interval > 27u * nominal) return false;
    units = interval / nominal;
    residual = interval % nominal;
    if (residual * 2u >= nominal) ++units;
    if (units < 12u || units > 27u) return false;
    target = units * nominal;
    if ((interval > target ? interval - target : target - interval) > tolerance) return false;
    *out = (uint8_t)(units - 12u);
    return true;
}

sent_result_t sent_decoder_process(sent_decoder_t *d, const uint16_t *capture,
                                   size_t count, sent_frame_t *out)
{
    uint8_t n[5], crc;
    size_t i, crc_count;
    if (d == NULL || capture == NULL || out == NULL || count != 6u) {
        if (d != NULL) d->diagnostics.last_frame_valid = false;
        return SENT_RESULT_INVALID_ARGUMENT;
    }
    memset(out, 0, sizeof(*out));
    if (capture[0] < d->config.sync_min_counts || capture[0] > d->config.sync_max_counts) {
        ++d->diagnostics.sync_errors;
        d->diagnostics.last_frame_valid = false;
        return SENT_RESULT_SYNC_ERROR;
    }
    for (i = 0u; i < 5u; ++i) {
        if (!to_nibble(&d->config, capture[i + 1u], &n[i])) {
            ++d->diagnostics.pulse_errors;
            d->diagnostics.last_frame_valid = false;
            return SENT_RESULT_PULSE_ERROR;
        }
    }
    out->status = n[0];
    out->data[0] = n[1]; out->data[1] = n[2]; out->data[2] = n[3];
    out->crc = n[4];
    crc_count = d->config.crc_includes_status ? 4u : 3u;
    crc = sent_decoder_crc4(&d->config,
        d->config.crc_includes_status ? n : &n[1], crc_count);
    out->crc_ok = crc == out->crc;
    out->sequence = d->next_sequence++;
    d->diagnostics.signal_present = true;
    if (!out->crc_ok) {
        ++d->diagnostics.crc_errors;
        d->diagnostics.last_frame_valid = false;
        return SENT_RESULT_CRC_ERROR;
    }
    ++d->diagnostics.valid_frames;
    d->diagnostics.last_frame_valid = true;
    return SENT_RESULT_OK;
}

void sent_decoder_note_timeout(sent_decoder_t *d)
{
    if (d == NULL) return;
    ++d->diagnostics.timeouts;
    d->diagnostics.signal_present = false;
    d->diagnostics.last_frame_valid = false;
}
