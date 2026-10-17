#include "sent_decoder.h"
#include "sent_sim.h"
#include <stdio.h>
static sent_decoder_config_t demo_config(void)
{
    sent_decoder_config_t c = {
        .nominal_tick_counts=10u, .tolerance_percent=20u,
        .sync_min_counts=450u, .sync_max_counts=650u,
        .crc_table={0u,13u,7u,10u,14u,3u,9u,4u,1u,12u,6u,11u,15u,2u,8u,5u},
        .crc_seed=0u, .crc_includes_status=true
    };
    return c;
}
int main(void)
{
    sent_decoder_config_t c=demo_config(); sent_decoder_t d; sent_frame_t f;
    uint16_t cap[6]; const uint8_t data[3]={3u,10u,5u}; int count;
    if (sent_decoder_init(&d,&c)!=SENT_RESULT_OK) return 1;
    count=sent_sim_make_capture(&c,2u,data,cap,6u);
    if (count<0) return 1;
    sent_result_t r=sent_decoder_process(&d,cap,(size_t)count,&f);
    printf("SENT host demo (synthetic data)\nresult=%d status=0x%X data=[0x%X,0x%X,0x%X] crc=0x%X crc_ok=%s\n",
           (int)r,f.status,f.data[0],f.data[1],f.data[2],f.crc,f.crc_ok?"true":"false");
    printf("valid=%lu sync_errors=%lu pulse_errors=%lu crc_errors=%lu\n",
           (unsigned long)d.diagnostics.valid_frames,(unsigned long)d.diagnostics.sync_errors,
           (unsigned long)d.diagnostics.pulse_errors,(unsigned long)d.diagnostics.crc_errors);
    return r==SENT_RESULT_OK?0:2;
}
