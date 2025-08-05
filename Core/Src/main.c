
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "adc_utils.h"

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
UART_HandleTypeDef huart1;

#define CS1_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define CS1_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define CS2_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define CS2_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)

uint8_t rx_buf[24];
int32_t ch_values[16];

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);

void ADS131_Read(SPI_HandleTypeDef *hspi, uint8_t *rx) {
    uint8_t tx[24] = {0};
    HAL_SPI_TransmitReceive(hspi, tx, rx, 24, HAL_MAX_DELAY);
}

void print_uart(const char* msg) {
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void read_adc_board(SPI_HandleTypeDef *hspi, int32_t *values, const char *label, uint8_t cs_pin) {
    ADS131_Read(hspi, rx_buf);
    decode_all_channels(rx_buf, values);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    MX_USART1_UART_Init();

    CS1_HIGH();
    CS2_HIGH();

    char msg[256];

    while (1) {
        uint32_t timestamp = HAL_GetTick();

        CS1_LOW();
        HAL_Delay(1);
        read_adc_board(&hspi1, &ch_values[0], "ADC1", GPIO_PIN_4);
        CS1_HIGH();

        CS2_LOW();
        HAL_Delay(1);
        read_adc_board(&hspi2, &ch_values[8], "ADC2", GPIO_PIN_12);
        CS2_HIGH();

        int len = snprintf(msg, sizeof(msg), "%lu", timestamp);
        for (int i = 0; i < 16; i++) {
            len += snprintf(msg + len, sizeof(msg) - len, ",%ld", ch_values[i]);
        }
        snprintf(msg + len, sizeof(msg) - len, "\r\n");
        print_uart(msg);

        HAL_Delay(200);
    }
}
