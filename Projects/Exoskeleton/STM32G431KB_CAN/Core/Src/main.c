/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file           : main.c
* @brief          : Main program body
******************************************************************************
* @attention
*
* Copyright (c) 2023 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fdcan.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "serial.h"
#include "utility.h"
#include "stdbool.h"
#include "arm_math.h"
//#include "CO_app_STM32.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t PDO_id_cnt	= 0;
uint8_t node_id;
Prot_info_t prot_info;
char *uart_tx_msg;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void setup(void);
void loop_sync(void);
void loop_async(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t temp_uint = 1;
/* Timer interrupt function executes every 1 ms */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  //    if (htim == canopenNodeSTM32->timerHandle) {
  //        canopen_app_interrupt();
  //    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM17_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  setup();
  while (1)
  {
	loop_async();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */
void setup(void)
{
  __disable_irq();
  
  /* USART for Virtual com port */
  vcp.init(&vcp);
  
  HAL_TIM_Base_Start_IT(&htim17);
  if (HAL_UART_Receive_DMA(vcp.huart, (uint8_t *)vcp.rx_buffer, UART_RX_BUFF_SIZE) != HAL_OK)		//UART_RX_BUFF_SIZE = 256, TX = 128
  {
	Error_Handler();
  }
  
  if (HAL_FDCAN_Start(hcan.module) != HAL_OK)
  {
	Error_Handler();
  }
  
  if (HAL_FDCAN_ActivateNotification(hcan.module, hcan.activeITs, hcan.rxloc) != HAL_OK)
  {
	Error_Handler();
  }
  hcan.txmsg.header.IdType = FDCAN_STANDARD_ID;						// FDCAN_STANDARD_ID, FDCAN_EXTENDED_ID
  hcan.txmsg.header.TxFrameType = FDCAN_DATA_FRAME;				// FDCAN_DATA_FRAME, FDCAN_REMOTE_FRAME(FDCAN High Priority MEssage Storage)
  hcan.txmsg.header.DataLength = FDCAN_DLC_BYTES_8;					// FDCAN_DLC_BYTES_0~64: FDCAN Data Length Code
  hcan.txmsg.header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;			// Transmitting node is error active, FDCAN_ESI_PASSIVE: Transmitting node is error passive
  hcan.txmsg.header.BitRateSwitch = FDCAN_BRS_OFF;						// Tx frame w/ or w/o bit rate switching
  hcan.txmsg.header.FDFormat = FDCAN_CLASSIC_CAN;					// Tx frame classic or FD
  hcan.txmsg.header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;	// store or not store Tx events
  hcan.txmsg.header.MessageMarker = 0;										// message marker copied into Tx Event FIFO element 0 ~ 0xFF
  //  hcan.txmsg.header.Identifier;
  //	hcan.txmsg.header.IdType = FDCAN_STANDARD_ID;						// or FDCAN_EXTENDED_ID
  //	hcan.txmsg.header.TxFrameType = FDCAN_DATA_FRAME;				// or FDCAN_REMOTE_FRAME
  //	hcan.txmsg.header.DataLength;													// 8 bytes
  //	hcan.txmsg.header.ErrorStateIndicator = FDCAN_ESI_ACTIVE; 		// or FDCAN_ESI_PASSIVE,역할?
  //	hcan.txmsg.header.BitRateSwitch = FDCAN_BRS_OFF;					// FDCAN에서만 BRS 가능 https://www.datajob.com/en/definition/101/bit-rate-switch-(brs)
  //	hcan.txmsg.header.FDFormat = FDCAN_CLASSIC_CAN;					/* NO CAN-FD */
  //	hcan.txmsg.header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;	// or FDCAN_STORE_TX_EVENTS, 역할?
  //	hcan.txmsg.header.MessageMarker = 0;										// Specifies the message marker to be copied into Tx Event FIFO element\
																											for identification of Tx message status. This parameter must be a number between 0 and 0xFF 역할?
  __enable_irq();		// iar enable interrupt
  HAL_Delay(4);
  for(int i=1;i<3;i++){
	SET_SDO(i, sizeof(uint8_t), MOP, 0x00, 10);	
	SET_SDO(i, sizeof(uint8_t), CAN_bit_rate, 0x00, 0);	
	motor[i].id = i+1;
//	while(!__HAL_FDCAN_GET_FLAG(&hfdcan1, FDCAN_FLAG_RX_FIFO0_NEW_MESSAGE)){}
  }
  NMT_TRANS(PREOP);	
}

bool uart_send_flag = false;
bool can_PDO_flag = false;
bool QS_flag			= false;
bool NMT_FLAG		= false;
bool DS_FLAG		= false;
bool STATUS_FLAG	= false;
uint32_t freq_cnt	= 0;
double period		= 2000.0;
double amp			= 100.0;
double val_2			= 0;
int16_t val				= 0;
NMT_state_t NMT_state	= PREOP;
DS_state_t status			= DV;

void loop_sync(void)
{  
  static int cnt = 0;  
  freq_cnt++;			// 1ms timer
  if(freq_cnt <= period/2){
	val = (int16_t)(amp);
  }else if(freq_cnt <= period){
	val = (int16_t)(-amp);
  }else{
	freq_cnt = 0;
  }  
  //  val = (int16_t)(amp*arm_sin_f32(2.0*PI*(double)freq_cnt/period)); // 주기 0.5초 
  if(++cnt>=3){
	if(NMT_state == OP && can_PDO_flag){
	  SYNC_FRAME();
	  cnt = 0;
	}
  }
}
void loop_async(void)
{
  if(uart_send_flag){
	serial_print(&vcp, "AT+CONMAC=70B8F6977692\r");
	uart_send_flag = false;
  }
  vcp.run(&vcp);
  
  if(NMT_FLAG){
	NMT_TRANS(NMT_state);
	NMT_FLAG = false;
  }
  if(DS_FLAG){
	DS_TRANS(1, status);
	DS_TRANS(2, status);
	DS_FLAG = false;
  }
  if(QS_flag){
	DS_TRANS(1, QS);
	DS_TRANS(2, QS);
	QS_flag = false;
  }
  if(STATUS_FLAG){
	READ_STATUS(1);
	READ_STATUS(2);
	STATUS_FLAG = false;
  }  
  if(PDO_id_cnt >= 2){
	SET_PDO(&motor[0]);
	SET_PDO(&motor[1]);
	PDO_id_cnt = 0;
  }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_RX_FIFO0_MESSAGE_LOST)){
	while(1);
  } 
  if ((hfdcan->Instance == hcan.module->Instance) && ((RxFifo0ITs & hcan.activeITs) != 0))
  {
	if (HAL_FDCAN_GetRxMessage(hcan.module, hcan.rxloc, &hcan.rxmsg.header, hcan.rxmsg.data) != HAL_OK)
	{
	  
	  // hcan.rxloc = FDCAN_RX_FIFO0(0x00000040U)	  
	  // hcan.rxmsg.header.IdType				
	  // hcan.rxmsg.header.RxFrame				
	  // hcan.rxmsg.header.ErrorStateIndicator
	  // hcan.rxmsg.header.DataLength
	  // hcan.rxmsg.header.data	  
	  Error_Handler();
	}else{
	  node_id = (hcan.rxmsg.header.Identifier & 0x7F) - 1;
	  prot_info= (Prot_info_t)(hcan.rxmsg.header.Identifier & 0x780);
	  if(hcan.rxmsg.data[0] == 0x80){
		// ABORT SDO PROTOCOL
	  }
	  switch(prot_info){
	  case SDO:
		motor[node_id].parsing_SDO(&motor[node_id], node_id);
		break;
	  case PDO1:
		PDO_id_cnt ++;
		motor[node_id].parsing_PDO(&motor[node_id], node_id);		
		break;
		// case PDO2:
	  }
	}
	temp_uint = HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0);
	if(temp_uint >= 3){	
	  while(1);
	  //	  if (HAL_FDCAN_GetRxMessage(hcan.module, hcan.rxloc, &hcan.rxmsg.header, hcan.rxmsg.data) != HAL_OK)
	  //	{
	  //	}
	}
  }
}
void HAL_SYSTICK_Callback(void)
{
  loop_sync();
}
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){ //1khz timer
//  
//}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
