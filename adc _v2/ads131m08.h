#ifndef __ADS131M08_H
#define __ADS131M08_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include <stdint.h>

/* ------------------------- Device Parameters -------------------------- */
#define ADS131M08_NUM_CHANNELS 8U
#define ADS131M08_FRAME_BYTES  (3 + ADS131M08_NUM_CHANNELS * 3) /* 27 bytes */

/* ------------------------- Command Opcodes ---------------------------- */
#define ADS131M08_CMD_NULL     0x0000u
#define ADS131M08_CMD_RESET    0x0011u
#define ADS131M08_CMD_STANDBY  0x0022u
#define ADS131M08_CMD_WAKEUP   0x0033u
#define ADS131M08_CMD_LOCK     0x0555u
#define ADS131M08_CMD_UNLOCK   0x0655u
#define ADS131M08_CMD_START    0x0088u
#define ADS131M08_CMD_STOP     0x00AAu

/* ------------------------- Register Map ------------------------------- */
#define ADS131M08_REG_ID          0x00u
#define ADS131M08_REG_STATUS      0x01u
#define ADS131M08_REG_MODE        0x02u
#define ADS131M08_REG_CLOCK       0x03u
#define ADS131M08_REG_GAIN1       0x04u
#define ADS131M08_REG_GAIN2       0x05u
#define ADS131M08_REG_CFG         0x06u
#define ADS131M08_REG_THRSHLD_MSB 0x07u
#define ADS131M08_REG_THRSHLD_LSB 0x08u

#define ADS131M08_REG_CHn_CFG(n)      (0x09u + (n)*5u)
#define ADS131M08_REG_CHn_OCAL_MSB(n) (0x0Au + (n)*5u)
#define ADS131M08_REG_CHn_OCAL_LSB(n) (0x0Bu + (n)*5u)
#define ADS131M08_REG_CHn_GCAL_MSB(n) (0x0Cu + (n)*5u)
#define ADS131M08_REG_CHn_GCAL_LSB(n) (0x0Du + (n)*5u)

#define ADS131M08_REG_CRC         0x3Eu

/* MODE Register Bits */
#define ADS131M08_MODE_WLENGTH_MASK   (3u << 8)
#define ADS131M08_MODE_OSR_MASK       (0x7u << 2)
#define ADS131M08_MODE_CHOP_MASK      (1u << 0)

/* CFG Register Bits */
#define ADS131M08_CFG_REFSEL_MASK     (1u << 8)
#define ADS131M08_CFG_CRCEN_MASK      (1u << 2)
#define ADS131M08_CFG_CRCFMT_MASK     (1u << 1)

/* CHn_CFG Register Bits */
#define ADS131M08_CHn_CFG_ENABLE_MASK (1u << 7)

/* Driver Handle */
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *CS_Port;
    uint16_t CS_Pin;
    GPIO_TypeDef *RESET_Port;
    uint16_t RESET_Pin;
    GPIO_TypeDef *DRDY_Port;
    uint16_t DRDY_Pin;
    uint8_t crc_enable;   /* 1 = CRC enabled, 0 = disabled */
} ADS131_Handle_t;

/* API Prototypes */
HAL_StatusTypeDef ADS131M08_SendCommand(ADS131_Handle_t *h, uint16_t cmd);
HAL_StatusTypeDef ADS131M08_WriteRegister(ADS131_Handle_t *h, uint8_t addr, uint16_t value);
HAL_StatusTypeDef ADS131M08_ReadRegister(ADS131_Handle_t *h, uint8_t addr, uint16_t *value);
HAL_StatusTypeDef ADS131M08_Init(ADS131_Handle_t *h);
HAL_StatusTypeDef ADS131M08_ReadDataFrame(ADS131_Handle_t *h, int32_t samples[ADS131M08_NUM_CHANNELS]);

/* Utility Macros */
static inline void ADS131_CS_LOW(ADS131_Handle_t *h)  { HAL_GPIO_WritePin(h->CS_Port, h->CS_Pin, GPIO_PIN_RESET); }
static inline void ADS131_CS_HIGH(ADS131_Handle_t *h) { HAL_GPIO_WritePin(h->CS_Port, h->CS_Pin, GPIO_PIN_SET); }
static inline void ADS131_RESET_LOW(ADS131_Handle_t *h)  { HAL_GPIO_WritePin(h->RESET_Port, h->RESET_Pin, GPIO_PIN_RESET); }
static inline void ADS131_RESET_HIGH(ADS131_Handle_t *h) { HAL_GPIO_WritePin(h->RESET_Port, h->RESET_Pin, GPIO_PIN_SET); }

#ifdef __cplusplus
}
#endif

#endif /* __ADS131M08_H */
