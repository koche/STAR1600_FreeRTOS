#ifndef _modbus_rtu_H_
#define _modbus_rtu_H_

#include <cmsis_os.h>

//#define MODBUS_RTU_RX_TIMEOUT    1000
#define DATA_MAX_LEN             512

typedef struct __Modbus_Callback
{
  HAL_StatusTypeDef (*HAL_UART_Transmit)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
  HAL_StatusTypeDef (*HAL_UART_Receive)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
  HAL_StatusTypeDef (*HAL_UART_Transmit_IT)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
  HAL_StatusTypeDef (*HAL_UART_Receive_IT)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
  void (*HAL_GPIO_WritePin)(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
  osStatus (*osDelay)(uint32_t millisec);
}Modbus_Callback;

extern int    MODBUS_RTU_RX_TIMEOUT;
extern char g_uart3RxCpltFlag;

int MODBUS_Init(UART_HandleTypeDef *uart);
void MODBUS_Register_Callback(Modbus_Callback *callback);

int MODBUS_ReadRegisterFC03(unsigned char addr, unsigned short start_reg, unsigned short nb, unsigned char *res);
int MODBUS_ReadRegisterFC04(unsigned char addr, unsigned short start_reg, unsigned short nb, unsigned char *res);
int MODBUS_WriteRegisterFC06(unsigned char addr, unsigned short reg, unsigned char *writeData);
int MODBUS_WriteRegisterFC10(unsigned char addr, unsigned short start_reg, unsigned short nb, unsigned char nbyte, unsigned char *writeData);

#endif //_modbus_rtu_H_