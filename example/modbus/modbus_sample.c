#include <cmsis_os.h>
#include <stdlib.h>
#include "main.h"
#include "modbus_rtu.h"

#define   ModbusDevDelay(t)          osDelay(t)
static Modbus_Callback modbusCallback;

extern UART_HandleTypeDef huart3;

int ModbusDevInit()
{
  int ret=0;
  
  /* assign uart3 as RS485 interface */
  MODBUS_Init(&huart3);
  modbusCallback.HAL_UART_Transmit = HAL_UART_Transmit;
  modbusCallback.HAL_UART_Receive = HAL_UART_Receive;
  modbusCallback.HAL_UART_Transmit_IT = HAL_UART_Transmit_IT;
  modbusCallback.HAL_UART_Receive_IT = HAL_UART_Receive_IT;
  modbusCallback.HAL_GPIO_WritePin = HAL_GPIO_WritePin;
  modbusCallback.osDelay = osDelay;
  MODBUS_Register_Callback(&modbusCallback);

  return ret;
}

int ModbusDevDeinit()
{
   return 0;
}

/*
 @param
   addr - modbus slave addrsss
   start_reg
   nb   - register word length

return
   0  - success
   -1 - fail
*/
int ModbusDevReadOneReg(unsigned char  addr, unsigned short start_reg, unsigned short nb)
{
  int ret, val;
  unsigned char  res[32];

  ret = MODBUS_ReadRegisterFC03(addr, start_reg, nb, res);
  if(ret == 0)
  {
    if(res[0] == 4)
    { //4 byte
      val = res[4] | (res[3]<<8) | (res[2]<<16) | (res[1]<<24);
    }
    else if(res[0] == 2)
    { //2 byte
      val = res[2] | (res[1]<<8);
    }
    else
    {
      printf("Not Recognized bytes format !!!");
      val = -1;
    }
  }
  else
    val = -1;

  return val;
}

/*
 @param
   addr      - modbus slave addrsss
   reg       - register address to be written
   writeData - 2bytes short data
*/
int ModbusDevWriteOneWord(unsigned char  addr, unsigned short reg, unsigned short writeData)
{
  int ret;
  unsigned char data[2];

  data[0] = writeData >> 8;
  data[1] = writeData & 0x00FF;
  ret = MODBUS_WriteRegisterFC06(addr, reg, data);

  return ret;
}


void ModbusDevTestTask(void const * argument)
{
  int             addr;
  unsigned short  vol, cur, watt, wh;
  
  addr = 0x01;
  
  ModbusDevInit();
  while(1)
  {
    vol  = ModbusDevReadOneReg(addr, 0x9C, 2);  
    cur  = ModbusDevReadOneReg(addr, 0xA4, 2);
    watt = ModbusDevReadOneReg(addr, 0xAC, 2);
    wh   = ModbusDevReadOneReg(addr, 0xC6, 2);

    printf("voltage = %.1f V    current= %d mA\nwatt = %d Watt        wh=%d WH\n", vol/10.0, cur, watt, wh);

    ModbusDevDelay(10000);
  }
  
  ModbusDevDeinit();
}

/*************************************************************************************************/
osThreadId ModbusDevTestTaskHandle;
int modbus_sample()
{
  
   osThreadDef(ModbusDevTestTask_Name, ModbusDevTestTask, osPriorityNormal, 0, 512);
   ModbusDevTestTaskHandle = osThreadCreate(osThread(ModbusDevTestTask_Name), NULL);
   if(ModbusDevTestTaskHandle == NULL)
     printf("ModbusDevTestTaskHandle error\n");
   
   return 0;
}
