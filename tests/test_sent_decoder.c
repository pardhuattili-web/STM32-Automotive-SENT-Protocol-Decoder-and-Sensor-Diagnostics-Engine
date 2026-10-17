#include "sent_decoder.h"
#include "sent_sim.h"

#include <assert.h>
#include <stdio.h>

static sent_decoder_config_t make_config(void)
{
    sent_decoder_config_t config = {
        .nominal_tick_counts = 10u,
        .tolerance_percent = 20u,
        .sync_min_counts = 450u,
        .sync_max_counts = 650u,
        .crc_table = {0u, 13u, 7u, 10u, 14u, 3u, 9u, 4u,
                      1u, 12u, 6u, 11u, 15u, 2u, 8u, 5u},
        .crc_seed = 0u,
        .crc_includes_status = true
    };
    return config;
}

static void make_valid_capture(const sent_decoder_config_t *config,
                               uint16_t capture[6])
{
    const uint8_t data[SENT_DATA_NIBBLES] = {3u, 10u, 5u};
    assert(sent_sim_make_capture(config, 2u, data, capture, 6u) == 6);
}

static void test_valid_frame(void)
{
    sent_decoder_config_t config = make_config();
    sent_decoder_t decoder;
    sent_frame_t frame;
    uint16_t capture[6];

    make_valid_capture(&config, capture);
    assert(sent_decoder_init(&decoder, &config) == SENT_RESULT_OK);
    assert(sent_decoder_process(&decoder, capture, 6u, &frame) == SENT_RESULT_OK);
    assert(frame.status == 2u);
    assert(frame.data[0] == 3u && frame.data[1] == 10u && frame.data[2] == 5u);
    assert(frame.crc_ok && frame.sequence == 0u);
    assert(decoder.diagnostics.valid_frames == 1u);
    assert(decoder.diagnostics.last_frame_valid);
}

static void test_crc_error(void)
{
    sent_decoder_config_t config = make_config();
    sent_decoder_t decoder;
    sent_frame_t frame;
    uint16_t capture[6];

    make_valid_capture(&config, capture);
    capture[5] = (uint16_t)(capture[5] == 120u ? 130u : 120u);
    assert(sent_decoder_init(&decoder, &config) == SENT_RESULT_OK);
    assert(sent_decoder_process(&decoder, capture, 6u, &frame) == SENT_RESULT_CRC_ERROR);
    assert(decoder.diagnostics.crc_errors == 1u);
    assert(!decoder.diagnostics.last_frame_valid);
}

static void test_sync_error(void)
{
    sent_decoder_config_t config = make_config();
    sent_decoder_t decoder;
    sent_frame_t frame;
    uint16_t capture[6];

    make_valid_capture(&config, capture);
    capture[0] = 100u;
    assert(sent_decoder_init(&decoder, &config) == SENT_RESULT_OK);
    assert(sent_decoder_process(&decoder, capture, 6u, &frame) == SENT_RESULT_SYNC_ERROR);
    assert(decoder.diagnostics.sync_errors == 1u);
}

static void test_pulse_error(void)
{
    sent_decoder_config_t config = make_config();
    sent_decoder_t decoder;
    sent_frame_t frame;
    uint16_t capture[6];

    make_valid_capture(&config, capture);
    capture[2] = 999u;
    assert(sent_decoder_init(&decoder, &config) == SENT_RESULT_OK);
    assert(sent_decoder_process(&decoder, capture, 6u, &frame) == SENT_RESULT_PULSE_ERROR);
    assert(decoder.diagnostics.pulse_errors == 1u);
}

static void test_arguments_timeout_and_invalid_config(void)
{
    sent_decoder_config_t config = make_config();
    sent_decoder_t decoder;
    sent_frame_t frame;

    assert(sent_decoder_init(&decoder, &config) == SENT_RESULT_OK);
    assert(sent_decoder_process(&decoder, NULL, 6u, &frame) == SENT_RESULT_INVALID_ARGUMENT);
    sent_decoder_note_timeout(&decoder);
    assert(decoder.diagnostics.timeouts == 1u);
    assert(!decoder.diagnostics.signal_present);
    assert(!decoder.diagnostics.last_frame_valid);

    config.nominal_tick_counts = 0u;
    assert(sent_decoder_init(&decoder, &config) == SENT_RESULT_INVALID_ARGUMENT);
}

static void test_capture_generator_rejects_overflow(void)
{
    sent_decoder_config_t config = make_config();
    uint16_t capture[6];
    uint8_t data[SENT_DATA_NIBBLES] = {1u, 2u, 3u};

    config.nominal_tick_counts = 1200u;
    assert(sent_sim_make_capture(&config, 0u, data, capture, 6u) == -1);
}

int main(void)
{
    test_valid_frame();
    test_crc_error();
    test_sync_error();
    test_pulse_error();
    test_arguments_timeout_and_invalid_config();
    test_capture_generator_rejects_overflow();
    puts("All SENT decoder tests passed.");
    return 0;
}
