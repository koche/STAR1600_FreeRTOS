/*
UART2 : RX
        TX
*/

#include "string.h"

#include "cmsis_os.h"
#include "stm32L4_eval.h"

#define  UART_TX_RX_WS211x_DEBUG        1
#define _printf(format, args...) printf(format, ## args)

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

osThreadId WS211x_TX_TaskHandle;
osThreadId WS211x_RX_TaskHandle;

void UART_WS211x_Fun();
void UART_WS211x_TX_TASK(void const * argument);
void UART_WS211x_RX_TASK(void const * argument);

char g_WS211x_TX_Enable = 1;

/*
eable : GPIO_PIN_SET/GPIO_PIN_RESET
*/
void WS211x_POWER_ENABLE(GPIO_PinState enable)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  //PC0
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Pin = SIP_PW_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SIP_PW_EN_GPIO_Port, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(SIP_PW_EN_GPIO_Port, SIP_PW_EN_Pin, enable);
  
  
  //Pull nRESET to low
  HAL_GPIO_WritePin(SIP_RESET_GPIO_Port, SIP_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(SIP_RESET_GPIO_Port, SIP_RESET_Pin, GPIO_PIN_SET);

}

void WS211x_USART2_GPIO_Init(void)
{
  //UART2 : PD5 TX, PD6 RX
  GPIO_InitTypeDef GPIO_InitStruct;
  //__HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_USART2_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);  
    
  HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void WS211x_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void UART_WS211x_Init()
{
  // Initial UART GPIO Pin (option)
  //WS211x_USART2_GPIO_Init();
  
  //Enable UART2 with baud rate = 115200
  //WS211x_USART2_UART_Init();

  //Enable power in sip connector pin and reset
  WS211x_POWER_ENABLE(GPIO_PIN_SET);
}


void UART_WS211x_Fun()
{
  // Initial UART GPIO Pin (option)
  WS211x_USART2_GPIO_Init();
  
  //Enable UART2 with baud rate = 115200
  WS211x_USART2_UART_Init();

  //Enable power in sip connector pin and reset
  WS211x_POWER_ENABLE(GPIO_PIN_SET);

#if (UART_TX_RX_WS211x_DEBUG)  
  osThreadDef(UART_WS211x_RX_TASKName, UART_WS211x_RX_TASK, osPriorityNormal, 0, 256);//stack size is in word(4bytes)
  WS211x_RX_TaskHandle = osThreadCreate(osThread(UART_WS211x_RX_TASKName), NULL);
  if(WS211x_RX_TaskHandle == NULL)
    _printf("WS211x_RX_TaskHandle error\n");  
#endif

  osThreadDef(UART_WS211x_TX_TASKName, UART_WS211x_TX_TASK, osPriorityNormal, 0, 512);//stack size is in word(4bytes)
  WS211x_TX_TaskHandle = osThreadCreate(osThread(UART_WS211x_TX_TASKName), NULL);
  if(WS211x_TX_TaskHandle == NULL)
    _printf("WS211x_TX_TaskHandle error\n"); 
}

HAL_StatusTypeDef UART_WS211x_DataSend(uint8_t* data, int data_size) {
  HAL_StatusTypeDef result = HAL_OK;
  int i = 0, j = 0;
  char sendData[64];
  int sendDataSize = 0;
  char payload[64];
  int payloadSize = 0;

  if(data != NULL) {
    memcpy(sendData, "AT+SENDSIGFOX {", 15);
    sendDataSize += 15;
	
    memcpy(payload, data, data_size);
    payloadSize += data_size;

    memcpy(sendData + sendDataSize, payload, payloadSize);
    sendDataSize += payloadSize;
    memcpy(sendData + sendDataSize, "}\r\n", 3);
    sendDataSize += 3;

    //if(sendDataSize < (18+24))
    _printf("sendDataSize = %d \n",sendDataSize);
    result = HAL_UART_Transmit(&huart2, sendData, sendDataSize, 1000); 
    //HEX_Dump(sendData, sendDataSize);
  } else {
    _printf("data is NULL \n");
    result = HAL_ERROR;
  }

  return result;
}

HAL_StatusTypeDef UART_WS211x_Reboot(void) {
  HAL_StatusTypeDef result = HAL_OK;
  char sendData[16];

  sprintf(sendData, "%s", "AT+REBOOT\r\n");
  result = HAL_UART_Transmit(&huart2, sendData, strlen(sendData), 1000); 
  //_printf("sendData = %s\n", sendData);

  return result;
}

void UART_WS211x_TX_TASK(void const * argument)
{
  HAL_StatusTypeDef result = HAL_OK;
  int i=0, j;
  char readByte;
  unsigned int  CHECK_TIME=1000 * 10;
  int cmd_num = 3;
  char cmd[3][15]={"494E4E4F3939","1234","5678"}; //HEX code only and max. 12byte (eg. INNO99 = 49 4E 4E 4F 39 39

  /* Initial delay 2 sec for dht */
  osDelay(2000);
  UART_WS211x_Reboot();
  osDelay(1000);
  
  i=0;
  while(1)
  {
    result = UART_WS211x_DataSend(cmd[i], strlen(cmd[i]));
    osDelay(CHECK_TIME);

    i++;
    i = i%cmd_num;
  }

  osThreadYield();
}

void UART_WS211x_RX_TASK(void const * argument)
{
  int  UART2_RX_TIMEOUT = 500; //ms
  int  i, ret, readLen, maxLen;
  char data[512], readByte;
  uint32_t recv_size = 0;
  uint32_t count = 0;
  maxLen = 512; //maximun length
  memset(data, 0, maxLen);

  while(1)
  {
    data[0] = '\0';
    readLen = 0;
    
    i=0;
    while(1)
    {
      g_uart2RxCpltFlag=0;
      if(HAL_UART_Receive_IT(&huart2, data, maxLen) == HAL_OK)
      {
        i = 0 ;
        while(g_uart2RxCpltFlag==0) 
        {
          osDelay(1);
          if(i++ > UART2_RX_TIMEOUT)
          { //Wait for a timeout
            readLen = (huart2.RxXferSize) - (huart2.RxXferCount);
            huart2.RxState = HAL_UART_STATE_READY;
            data[readLen]=0;
            
            break;
          }
        }
        g_uart2RxCpltFlag=0;
      }
      else
      {
        _printf("UART2_RX_IT() FAIL. Data not complete ???\n");
      }
      
      if(readLen > 0)
      {
        //_printf_uart("%s", data);
        if(strstr(data,"OK") != NULL) {
          //setLedState(LED_COMM_TX);
          _printf("Send OK\n");
        } else if(strstr(data,"RadioTxDelayDone") != NULL) {
          g_WS211x_TX_Enable = 1;
          _printf("RadioTxDelayDone\n");
        } else {
          //setLedState(LED_SYS_ACTIVE);
          //String_Dump(data, readLen);
        }
      }
    }
  }

  osThreadYield();
}

