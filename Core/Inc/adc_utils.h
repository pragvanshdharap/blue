
#ifndef ADC_UTILS_H
#define ADC_UTILS_H

#include <stdint.h>

int32_t decode_24bit(uint8_t *data);
void decode_all_channels(uint8_t *rx_buf, int32_t *ch_values);

#endif
