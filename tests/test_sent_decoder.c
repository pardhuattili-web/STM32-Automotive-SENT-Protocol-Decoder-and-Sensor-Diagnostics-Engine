#include "sent_decoder.h"
#include "sent_sim.h"
#include <assert.h>
#include <stdio.h>

static sent_decoder_config_t config_make(void)
{
    sent_decoder_config_t c = {
        .nominal_tick_counts=10u, .tolerance_percent=20u,
        .sync_min_counts=450u, .sync_max_counts=650u,
        .crc_table={0u,13u,7u,10u,14u,3u,9u,4u,1u,12u,6u,11u,15u,2u,8u,5u},
        .crc_seed=0u, .crc_includes_status=true
    };
    return c;
}
static void capture_make(sent_decoder_config_t *c, uint16_t *cap)
{
    const uint8_t data[3]={3u,10u,5u};
    assert(sent_sim_make_capture(c, 2u, data, cap, 6u)==6);
}
static void test_valid(void)
{
    sent_decoder_config_t c=config_make(); sent_decoder_t d; sent_frame_t f; uint16_t cap[6];
    capture_make(&c,cap); assert(sent_decoder_init(&d,&c)==SENT_RESULT_OK);
    assert(sent_decoder_process(&d,cap,6u,&f)==SENT_RESULT_OK);
    assert(f.status==2u && f.data[0]==3u && f.data[1]==10u && f.data[2]==5u);
    assert(f.crc_ok && f.sequence==0u && d.diagnostics.valid_frames==1u);
}
static void test_crc(void)
{
    sent_decoder_config_t c=config_make(); sent_decoder_t d; sent_frame_t f; uint16_t cap[6];
    capture_make(&c,cap); cap[5]=(uint16_t)(cap[5]==120u?130u:120u);
    assert(sent_decoder_init(&d,&c)==SENT_RESULT_OK);
    assert(sent_decoder_process(&d,cap,6u,&f)==SENT_RESULT_CRC_ERROR);
    assert(d.diagnostics.crc_errors==1u);
}
static void test_sync_and_pulse(void)
{
    sent_decoder_config_t c=config_make(); sent_decoder_t d; sent_frame_t f; uint16_t cap[6];
    capture_make(&c,cap); cap[0]=100u; assert(sent_decoder_init(&d,&c)==SENT_RESULT_OK);
    assert(sent_decoder_process(&d,cap,6u,&f)==SENT_RESULT_SYNC_ERROR);
    assert(d.diagnostics.sync_errors==1u);
    capture_make(&c,cap); cap[2]=999u;
    assert(sent_decoder_process(&d,cap,6u,&f)==SENT_RESULT_PULSE_ERROR);
    assert(d.diagnostics.pulse_errors==1u);
}
static void test_arguments_config_timeout(void)
{
    sent_decoder_config_t c=config_make(); sent_decoder_t d; sent_frame_t f;
    assert(sent_decoder_init(&d,&c)==SENT_RESULT_OK);
    assert(sent_decoder_process(&d,NULL,6u,&f)==SENT_RESULT_INVALID_ARGUMENT);
    sent_decoder_note_timeout(&d);
    assert(d.diagnostics.timeouts==1u && !d.diagnostics.signal_present);
    c.nominal_tick_counts=0u;
    assert(sent_decoder_init(&d,&c)==SENT_RESULT_INVALID_ARGUMENT);
}
int main(void)
{
    test_valid(); test_crc(); test_sync_and_pulse(); test_arguments_config_timeout();
    puts("All SENT decoder tests passed."); return 0;
}
