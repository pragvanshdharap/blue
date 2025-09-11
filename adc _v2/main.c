#include "main.h"
#include "ads131m08.h"
#include <stdio.h>
#include <stdarg.h>

/* HAL handles (CubeMX generates these) */
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
UART_HandleTypeDef huart1;

/* ADS handles */
ADS131_Handle_t ads1 = {
    .hspi = &hspi1,
    .CS_Port = GPIOA, .CS_Pin = GPIO_PIN_4,
    .RESET_Port = GPIOB, .RESET_Pin = GPIO_PIN_0,
    .DRDY_Port = GPIOB, .DRDY_Pin = GPIO_PIN_1,
    .crc_enable = 1   /* CRC enabled */
};

ADS131_Handle_t ads2 = {
    .hspi = &hspi2,
    .CS_Port = GPIOC, .CS_Pin = GPIO_PIN_6,
    .RESET_Port = GPIOC, .RESET_Pin = GPIO_PIN_7,
    .DRDY_Port = GPIOC, .DRDY_Pin = GPIO_PIN_8,
    .crc_enable = 0   /* CRC disabled */
};

int32_t adc1_samples[ADS131M08_NUM_CHANNELS];
int32_t adc2_samples[ADS131M08_NUM_CHANNELS];

/* UART printf */
static void debug_print(const char *fmt, ...)
{
    char buf[200];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n > 0) HAL_UART_Transmit(&huart1, (uint8_t*)buf, n, HAL_MAX_DELAY);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    MX_USART1_UART_Init();

    ADS131_CS_HIGH(&ads1); ADS131_RESET_HIGH(&ads1);
    ADS131_CS_HIGH(&ads2); ADS131_RESET_HIGH(&ads2);

    if (ADS131M08_Init(&ads1) == HAL_OK) debug_print("ADS1 init OK with CRC\r\n");
    else debug_print("ADS1 init FAIL\r\n");

    if (ADS131M08_Init(&ads2) == HAL_OK) debug_print("ADS2 init OK without CRC\r\n");
    else debug_print("ADS2 init FAIL\r\n");

    while (1)
    {
        if (HAL_GPIO_ReadPin(ads1.DRDY_Port, ads1.DRDY_Pin) == GPIO_PIN_RESET) {
            if (ADS131M08_ReadDataFrame(&ads1, adc1_samples) == HAL_OK) {
                debug_print("ADC1 CH0=%ld CH1=%ld\r\n", (long)adc1_samples[0], (long)adc1_samples[1]);
            }
        }
        if (HAL_GPIO_ReadPin(ads2.DRDY_Port, ads2.DRDY_Pin) == GPIO_PIN_RESET) {
            if (ADS131M08_ReadDataFrame(&ads2, adc2_samples) == HAL_OK) {
                debug_print("ADC2 CH0=%ld CH1=%ld\r\n", (long)adc2_samples[0], (long)adc2_samples[1]);
            }
        }
    }
}

/* CubeMX init stubs */
void MX_SPI1_Init(void) { /* configure SPI1 (mode 1/3, prescaler, etc.) */ }
void MX_SPI2_Init(void) { /* configure SPI2 same as SPI1 */ }
void MX_USART1_UART_Init(void) { /* UART1 115200 8N1 */ }
void MX_GPIO_Init(void) { /* CS/RESET outputs + DRDY inputs */ }
void SystemClock_Config(void) { /* system clock setup */ }
void Error_Handler(void) { while(1){} }
