
#include "adc_utils.h"

int32_t decode_24bit(uint8_t *data) {
    int32_t val = ((int32_t)data[0] << 16) | ((int32_t)data[1] << 8) | data[2];
    if (val & 0x800000)
        val |= 0xFF000000;
    return val;
}

void decode_all_channels(uint8_t *rx_buf, int32_t *ch_values) {
    for (int i = 0; i < 8; i++) {
        ch_values[i] = decode_24bit(&rx_buf[i * 3]);
    }
}
