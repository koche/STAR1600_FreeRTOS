#ifndef _ADC_TI863X_H_
#define _ADC_TI863X_H_
#include "spi.h"
#include "stm32l4xx_hal_gpio.h"

typedef enum
{
    TI_SGL_PIN_7    =   0x01,
    TI_SGL_PIN_6    =   0x02,
    TI_SGL_PIN_5    =   0x04,
    TI_SGL_PIN_4    =   0x08,
    TI_SGL_PIN_3    =   0x10,
    TI_SGL_PIN_2    =   0x20,
    TI_SGL_PIN_1    =   0x40,
    TI_SGL_PIN_0    =   0x80,
    /*  Alias  */
    TI_Signal_1     =  TI_SGL_PIN_0,
    TI_Signal_2     =  TI_SGL_PIN_1,
    TI_Signal_3     =  TI_SGL_PIN_2,
    TI_Signal_4     =  TI_SGL_PIN_3,
    TI_Signal_5     =  TI_SGL_PIN_4,
    TI_Signal_6     =  TI_SGL_PIN_5,
    TI_Signal_7     =  TI_SGL_PIN_6,
    TI_Signal_8     =  TI_SGL_PIN_7
}ADC_TI863x_PIN_TYPE_e;

typedef enum
{
    VOL_RANGE_READ    =   0,
    VOL_RANGE_P10N10,              //+-10v
    VOL_RANGE_P5N5,                //+-5v
    VOL_RANGE_P25N25,              //+-2.5v
    VOL_RANGE_RESERVED,
    VOL_RANGE_P10N0,               //0~10v
    VOL_RANGE_P5N0,                //0~5v
    VOL_RANGE_POWER_DOWN
}ADC_TI863x_VOL_RANGE_e;

typedef struct __ADC_TI863x_Callback
{
  HAL_StatusTypeDef (*HAL_SPI_Transmit)(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
  HAL_StatusTypeDef (*HAL_SPI_TransmitReceive)(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);
  void (*HAL_GPIO_Init)(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init);
  void (*HAL_GPIO_WritePin)(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
}ADC_TI863x_Callback;

int ADC_TI863x_Init(SPI_HandleTypeDef *spi);
void ADC_TI863x_DeInit(void);
void ADC_TI863x_Register_Callback(ADC_TI863x_Callback *callback);
int ADC_TI863x_Enable(void);
int ADC_TI863x_ConfigAuto(int pin, ADC_TI863x_VOL_RANGE_e range, char sel_temp);
int ADC_TI863x_ConfigManual(int pin, ADC_TI863x_VOL_RANGE_e range, char sel_temp);
int ADC_TI863x_ReadPin(ADC_TI863x_PIN_TYPE_e pin);
#endif //_ADC_TI863X_H_