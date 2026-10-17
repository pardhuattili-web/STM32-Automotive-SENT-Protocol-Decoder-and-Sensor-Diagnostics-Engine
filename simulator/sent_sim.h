#ifndef SENT_SIM_H
#define SENT_SIM_H
#include "sent_decoder.h"
int sent_sim_make_capture(const sent_decoder_config_t *config, uint8_t status,
                          const uint8_t data[SENT_DATA_NIBBLES],
                          uint16_t *capture, size_t capture_capacity);
#endif
