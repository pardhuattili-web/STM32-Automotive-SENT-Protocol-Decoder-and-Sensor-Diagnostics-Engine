#ifndef SENT_DECODER_H
#define SENT_DECODER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SENT_DATA_NIBBLES 3u
#define SENT_FRAME_NIBBLES 5u

typedef struct {
    uint16_t nominal_tick_counts;
    uint8_t tolerance_percent;
    uint16_t sync_min_counts;
    uint16_t sync_max_counts;
    uint8_t crc_table[16];
    uint8_t crc_seed;
    bool crc_includes_status;
} sent_decoder_config_t;

typedef struct {
    uint8_t status;
    uint8_t data[SENT_DATA_NIBBLES];
    uint8_t crc;
    bool crc_ok;
    uint32_t sequence;
} sent_frame_t;

typedef struct {
    uint32_t valid_frames, sync_errors, pulse_errors, crc_errors, timeouts;
    bool signal_present;
    bool last_frame_valid;
} sent_diagnostics_t;

typedef enum {
    SENT_RESULT_OK = 0,
    SENT_RESULT_INVALID_ARGUMENT,
    SENT_RESULT_SYNC_ERROR,
    SENT_RESULT_PULSE_ERROR,
    SENT_RESULT_CRC_ERROR
} sent_result_t;

typedef struct {
    sent_decoder_config_t config;
    sent_diagnostics_t diagnostics;
    uint32_t next_sequence;
} sent_decoder_t;

sent_result_t sent_decoder_init(sent_decoder_t *decoder, const sent_decoder_config_t *config);
sent_result_t sent_decoder_process(sent_decoder_t *decoder, const uint16_t *capture,
                                   size_t capture_count, sent_frame_t *out_frame);
void sent_decoder_note_timeout(sent_decoder_t *decoder);
uint8_t sent_decoder_crc4(const sent_decoder_config_t *config,
                          const uint8_t *nibbles, size_t nibble_count);

#endif
