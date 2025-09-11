/* ads131m08.c
 * Multi-instance driver implementation for ADS131M08 using STM32 HAL
 */

#include "ads131m08.h"
#include <string.h>

/* Helper: SPI TX/RX blocking */
static HAL_StatusTypeDef spi_txrx(ADS131_Handle_t *h, uint8_t *tx, uint8_t *rx, uint16_t len)
{
    return HAL_SPI_TransmitReceive(h->hspi, tx, rx, len, HAL_MAX_DELAY);
}

/* Send a 16-bit command */
HAL_StatusTypeDef ADS131M08_SendCommand(ADS131_Handle_t *h, uint16_t cmd)
{
    uint8_t tx[2] = { (uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF) };
    uint8_t rx[2];

    ADS131_CS_LOW(h);
    HAL_StatusTypeDef ret = spi_txrx(h, tx, rx, 2);
    ADS131_CS_HIGH(h);

    HAL_Delay(1);
    return ret;
}

/* Write 16-bit register */
HAL_StatusTypeDef ADS131M08_WriteRegister(ADS131_Handle_t *h, uint8_t addr, uint16_t value)
{
    uint8_t tx[4];
    uint8_t rx[4];
    tx[0] = (uint8_t)(0x40u | (addr & 0x3F)); /* WREG + addr */
    tx[1] = 0x00u; /* count=0 → one reg */
    tx[2] = (uint8_t)(value >> 8);
    tx[3] = (uint8_t)(value & 0xFF);

    ADS131_CS_LOW(h);
    HAL_StatusTypeDef ret = spi_txrx(h, tx, rx, 4);
    ADS131_CS_HIGH(h);
    HAL_Delay(1);
    return ret;
}

/* Read 16-bit register */
HAL_StatusTypeDef ADS131M08_ReadRegister(ADS131_Handle_t *h, uint8_t addr, uint16_t *value)
{
    if (!value) return HAL_ERROR;
    uint8_t tx[2] = { (uint8_t)(0x20u | (addr & 0x3F)), 0x00u };
    uint8_t rx[2];

    ADS131_CS_LOW(h);
    if (HAL_SPI_Transmit(h->hspi, tx, 2, HAL_MAX_DELAY) != HAL_OK) {
        ADS131_CS_HIGH(h);
        return HAL_ERROR;
    }
    if (HAL_SPI_Receive(h->hspi, rx, 2, HAL_MAX_DELAY) != HAL_OK) {
        ADS131_CS_HIGH(h);
        return HAL_ERROR;
    }
    ADS131_CS_HIGH(h);

    *value = ((uint16_t)rx[0] << 8) | rx[1];
    HAL_Delay(1);
    return HAL_OK;
}

/* Initialize ADS131M08 with sane defaults */
HAL_StatusTypeDef ADS131M08_Init(ADS131_Handle_t *h)
{
    if (!h) return HAL_ERROR;

    ADS131_CS_HIGH(h);

    /* hardware reset */
    ADS131_RESET_LOW(h);
    HAL_Delay(10);
    ADS131_RESET_HIGH(h);
    HAL_Delay(10);

    /* software reset */
    if (ADS131M08_SendCommand(h, ADS131M08_CMD_RESET) != HAL_OK) return HAL_ERROR;
    HAL_Delay(5);

    /* unlock */
    if (ADS131M08_SendCommand(h, ADS131M08_CMD_UNLOCK) != HAL_OK) return HAL_ERROR;
    HAL_Delay(2);

    /* MODE: 24-bit word, OSR=1024, CHOP=1 */
    uint16_t mode_reg = (1u << 8) | (5u << 2) | (1u << 0);
    if (ADS131M08_WriteRegister(h, ADS131M08_REG_MODE, mode_reg) != HAL_OK) return HAL_ERROR;

    /* CLOCK: default external */
    if (ADS131M08_WriteRegister(h, ADS131M08_REG_CLOCK, 0x0000u) != HAL_OK) return HAL_ERROR;

    /* Gains = 1 */
    if (ADS131M08_WriteRegister(h, ADS131M08_REG_GAIN1, 0x0000u) != HAL_OK) return HAL_ERROR;
    if (ADS131M08_WriteRegister(h, ADS131M08_REG_GAIN2, 0x0000u) != HAL_OK) return HAL_ERROR;

    /* CFG: internal reference, CRC disabled for now */
    uint16_t cfg_reg = ADS131M08_CFG_REFSEL_MASK;
    if (ADS131M08_WriteRegister(h, ADS131M08_REG_CFG, cfg_reg) != HAL_OK) return HAL_ERROR;

    /* Enable all channels */
    for (uint8_t ch = 0; ch < ADS131M08_NUM_CHANNELS; ++ch) {
        uint16_t ch_cfg_val = ADS131M08_CHn_CFG_ENABLE_MASK;
        if (ADS131M08_WriteRegister(h, ADS131M08_REG_CHn_CFG(ch), ch_cfg_val) != HAL_OK)
            return HAL_ERROR;
    }

    /* lock */
    (void)ADS131M08_SendCommand(h, ADS131M08_CMD_LOCK);
    HAL_Delay(2);

    /* start conversions */
    if (ADS131M08_SendCommand(h, ADS131M08_CMD_START) != HAL_OK) return HAL_ERROR;
    HAL_Delay(2);

    return HAL_OK;
}

/* Compute CRC16 (poly=0x8005, init=0xFFFF, MSB-first) */
static uint16_t ads131_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x8005;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* Read one data frame (with optional CRC) */
HAL_StatusTypeDef ADS131M08_ReadDataFrame(ADS131_Handle_t *h, int32_t samples[ADS131M08_NUM_CHANNELS])
{
    if (!h || !samples) return HAL_ERROR;

    uint8_t tx[32] = {0};
    uint8_t rx[32] = {0};

    /* Default length = 27 (no CRC) */
    uint16_t frame_len = ADS131M08_FRAME_BYTES;

    /* Check if CRC enabled */
    uint16_t cfg_val;
    if (ADS131M08_ReadRegister(h, ADS131M08_REG_CFG, &cfg_val) == HAL_OK) {
        if (cfg_val & ADS131M08_CFG_CRCEN_MASK) {
            frame_len += 2;
        }
    }

    ADS131_CS_LOW(h);
    if (spi_txrx(h, tx, rx, frame_len) != HAL_OK) {
        ADS131_CS_HIGH(h);
        return HAL_ERROR;
    }
    ADS131_CS_HIGH(h);

    /* CRC check if enabled */
    if (frame_len > ADS131M08_FRAME_BYTES) {
        uint16_t received_crc = ((uint16_t)rx[frame_len - 2] << 8) | rx[frame_len - 1];
        uint16_t calc_crc = ads131_crc16(rx, frame_len - 2);
        if (received_crc != calc_crc) {
            return HAL_ERROR; /* CRC mismatch */
        }
    }

    /* Parse channels */
    for (int ch = 0; ch < ADS131M08_NUM_CHANNELS; ++ch) {
        uint32_t idx = 3u + (uint32_t)ch * 3u;
        int32_t raw24 = ((int32_t)rx[idx] << 16) | ((int32_t)rx[idx + 1] << 8) | rx[idx + 2];
        if (raw24 & 0x00800000) raw24 |= 0xFF000000;
        samples[ch] = raw24;
    }

    return HAL_OK;
}
