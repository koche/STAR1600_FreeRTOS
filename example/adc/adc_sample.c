#include <cmsis_os.h>
#include <stdlib.h>
#include "main.h"
#include "libadc.h"

ADC_TI863x_PIN_TYPE_e ADC_INPUT_1 = TI_SGL_PIN_0;
ADC_TI863x_PIN_TYPE_e ADC_INPUT_2 = TI_SGL_PIN_1;

static ADC_TI863x_Callback AdcCallback;
extern SPI_HandleTypeDef hspi3;

int ADCInit();
int ADCDeinit();

int ADCInit()
{
  int ret=0;
  
  //Enable related GPIO clock
  if(__HAL_RCC_GPIOA_IS_CLK_DISABLED())
    __HAL_RCC_GPIOA_CLK_ENABLE();
  if(__HAL_RCC_GPIOC_IS_CLK_DISABLED())
    __HAL_RCC_GPIOC_CLK_ENABLE();
  
  /* assign hspi3 as ADC interface */
  ret = ADC_TI863x_Init(&hspi3);
  AdcCallback.HAL_SPI_Transmit = HAL_SPI_Transmit;
  AdcCallback.HAL_SPI_TransmitReceive = HAL_SPI_TransmitReceive;
  AdcCallback.HAL_GPIO_Init = HAL_GPIO_Init;
  AdcCallback.HAL_GPIO_WritePin = HAL_GPIO_WritePin;
  ADC_TI863x_Register_Callback(&AdcCallback);
  ret |= ADC_TI863x_Enable(); //Enable ADC CS/AL_PD pin
  
	/* Enable ADC pin 1, pin  with voltage range = 0 ~ 5V*/
  ret |= ADC_TI863x_ConfigAuto(ADC_INPUT_1 | ADC_INPUT_2, VOL_RANGE_P5N0, 0);
  osDelay(10);
  return ret;
}

int ADCDeinit()
{
   return 0;
}

/*
 @param
   pin - Which ADC pin want to read

return
   ADC value
*/
short ADCRead(ADC_TI863x_PIN_TYPE_e pin)
{
  short val = 0x0;
  val = ADC_TI863x_ReadPin(pin);
  return val;
}

void AdcTestTask(void const * argument)
{
  short  val_1, val_2;

  ADCInit();
  while(1)
  {
    val_1  = ADCRead(ADC_INPUT_1);
    val_2  = ADCRead(ADC_INPUT_2);
    printf("val_1 = %d mV, val_2= %d mV\n", val_1*5000/4096, val_2*5000/4096);
    osDelay(1000);
  }
  
  ADCDeinit();
}

/*************************************************************************************************/
osThreadId AdcTestTaskHandle;
int adc_sample()
{
   osThreadDef(AdcTestTask_Name, AdcTestTask, osPriorityNormal, 0, 512);
   AdcTestTaskHandle = osThreadCreate(osThread(AdcTestTask_Name), NULL);
   if(AdcTestTaskHandle == NULL)
     printf("AdcTestTaskHandle error\n");
   
   return 0;
}
