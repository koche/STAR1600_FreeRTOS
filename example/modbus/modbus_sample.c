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
  unsigned short  reg;
  
  addr = 0x01;
  
  ModbusDevInit();
  while(1)
  {

    reg = 0xAC;
    printf("reg %04X = %d\n",reg, ModbusDevReadOneReg(addr, reg, 2));

    reg = 0xC6;
    printf("reg %04X = %d\n",reg, ModbusDevReadOneReg(addr, reg, 2));
    
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
}
