#include <stdlib.h>
#include "main.h"

char     g_uartTxCpltFlag=0, g_uart2TxCpltFlag=0, g_uart3TxCpltFlag=0;
char     g_uartRxCpltFlag=0, g_uart2RxCpltFlag=0, g_uart3RxCpltFlag=0;

extern UART_HandleTypeDef huart2, huart3;

/****************************************************************************/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  
  if(huart == &huart3)
  {
    g_uart3TxCpltFlag = 1;
  }
  else if(huart == &huart2)
  {
    g_uart2TxCpltFlag = 1;
  }
  else
    g_uartTxCpltFlag = 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart == &huart3)
  {
    g_uart3RxCpltFlag = 1;
  }
  else if(huart == &huart2)
  {
    g_uart2RxCpltFlag = 1;
  }
  else
    g_uartRxCpltFlag = 1;
}

/****************************************************************************/
