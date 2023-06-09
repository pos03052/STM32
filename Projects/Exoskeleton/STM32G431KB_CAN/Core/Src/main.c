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
int16_t val				= 0;
Prot_info_t prot_info;
char *uart_tx_msg;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void setup(void);
void loop_sync(void);
void loop_async(void);
int16_t func_sin(uint32_t freq);
void I2C_DMA_COMM();

// dynamic params
double l1 = 437.6;
//double l2 = 245;
//double l2 = 228;
double l2 = 278;
//double cg_upperarm	= 341.48;
double cg_upperarm	= 333.655;	// aluminium shaft holder
//double cg_forearm	= 182.856;
//double cg_forearm	= 81.57;
double cg_forearm	= 178.35;	// 2kg disk jig
double weight = 20;
//double weight_upperarm	= 477.93 * 9.80665  / 1000;	// N
double weight_upperarm	= 508.83 * 9.80665  / 1000;	// aluminium shaft holder
//double weight_forearm	= 182.857 * 9.80665 / 1000; // N
//double weight_forearm	= 43.348 * 9.80665 / 1000; // N
double weight_forearm	= 98.88 * 9.80665 / 1000; // 2kg disk jig

double rated_torque 	= 167.001;
double gear_ratio[4] = {-96.875, 96.875, 48.82, -48.82};

double period				= 2000.0;
double amp					= 50.0;
//double gear_efficiency[4] 		= {1.1, 1.1, 1.05, 1.05};
double gear_efficiency[4] 		= {1.0, 1.0, 1.0, 1.0};
double trq_offset[4] = {1.0, 1.0, 1.0, 1.0};

bool UART_FLAG		 		= false;
bool QS_flag				= false;
bool NMT_FLAG				= false;
bool DS_FLAG				= false;
bool STATUS_FLAG			= true;
bool SET_ANGLE_ZERO_FLAG 	= false;
bool ANGLE_FLAG				= false;
bool CLEAR_ERROR_FLAG		= false;
bool GET_SDO_FLAG			= false;
bool SET_SDO_FLAG			= false;
bool I2C_FLAG				= false;
bool I2C_TRQ_FLAG			= false;
bool ERROR_FLAG				= false;

GPIO_PinState pin_state;
uint32_t freq_cnt			= 0;
uint32_t tick 				= 0;	// async timer tick
uint8_t timer				= 0;	// async period

uint8_t id_temp 			= 0;
uint8_t length_temp 		= 0;
Obj_dict_t object 			= def;
uint16_t object_sub 		= 0x00;
int32_t data_temp 			= 0;

NMT_state_t	NMT_state		= PRE;
DS_state_t	DS_state		= DV;

uint8_t txfifoemptycheck 	= 0;
uint8_t id_sum			 	= 0;
uint8_t id_cnt				= 0;

float error_res[2]		= {0, };

////////////////////////////////////////
//uint8_t data1[6] = {0, };
////uint8_t data2[6] = {0, };
//uint8_t tx_buf1[6] = {(uint16_t)0x3d, };
//uint8_t tx_buf2[6] = {(uint16_t)0x3d, };
//uint16_t addr1 = 0x50 << 1;
//uint16_t addr2 = 0x51 << 1;
//typedef struct sense{
//  float roll;
//  float pitch;
//  float yaw;
//} Sen;
//Sen sen1, sen2;
//bool state;
//uint32_t start_time;
//uint32_t time_interval;
////////////////////////////////////////





/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Timer interrupt function executes every 1 ms */

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
  
  if (HAL_UART_Receive_DMA(vcp.huart, (uint8_t *)vcp.rx_buffer, UART_RX_BUFF_SIZE) != HAL_OK)		//UART_RX_BUFF_SIZE = 256, TX = 128
  {
	Error_Handler();
  }  
  if (HAL_FDCAN_Start(hcan.module) != HAL_OK)
  {
	Error_Handler();
  }  
  if (HAL_FDCAN_ActivateNotification(hcan.module, hcan.activeITs, 0) != HAL_OK)
  {
	Error_Handler();
  }  
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_TX_FIFO_EMPTY|FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2) != HAL_OK)
  {	
	Error_Handler();	
  }  
  hcan.txmsg.header.IdType = FDCAN_STANDARD_ID;						// FDCAN_STANDARD_ID, FDCAN_EXTENDED_ID
  hcan.txmsg.header.TxFrameType = FDCAN_DATA_FRAME;					// FDCAN_DATA_FRAME, FDCAN_REMOTE_FRAME(FDCAN High Priority MEssage Storage)
  hcan.txmsg.header.DataLength = FDCAN_DLC_BYTES_8;					// FDCAN_DLC_BYTES_0~64: FDCAN Data Length Code
  hcan.txmsg.header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;			// Transmitting node is error active, FDCAN_ESI_PASSIVE: Transmitting node is error passive
  hcan.txmsg.header.BitRateSwitch = FDCAN_BRS_OFF;					// Tx frame w/ or w/o bit rate switching
  hcan.txmsg.header.FDFormat = FDCAN_CLASSIC_CAN;					// Tx frame classic or FD
  hcan.txmsg.header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;		// store or not store Tx events
  hcan.txmsg.header.MessageMarker = 0;								// message marker copied into Tx Event FIFO element 0 ~ 0xFF
  //  hcan.txmsg.header.Identifier;
  //	hcan.txmsg.header.IdType = FDCAN_STANDARD_ID;				// or FDCAN_EXTENDED_ID
  //	hcan.txmsg.header.TxFrameType = FDCAN_DATA_FRAME;			// or FDCAN_REMOTE_FRAME
  //	hcan.txmsg.header.DataLength;								// 8 bytes
  //	hcan.txmsg.header.ErrorStateIndicator = FDCAN_ESI_ACTIVE; 	// or FDCAN_ESI_PASSIVE,역할?
  //	hcan.txmsg.header.BitRateSwitch = FDCAN_BRS_OFF;			// FDCAN에서만 BRS 가능 https://www.datajob.com/en/definition/101/bit-rate-switch-(brs)
  //	hcan.txmsg.header.FDFormat = FDCAN_CLASSIC_CAN;				/* NO CAN-FD */
  //	hcan.txmsg.header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;	// or FDCAN_STORE_TX_EVENTS, 역할?
  //	hcan.txmsg.header.MessageMarker = 0;						// Specifies the message marker to be copied into Tx Event FIFO element\
  																	for identification of Tx message status. This parameter must be a number between 0 and 0xFF 역할?
	__enable_irq();		// iar enable interrupt
  INIT_CAN();
  for(int i=0;i<4;i++){
	if(motor[i].Statusword != 0x40)	error_res[0] = 6;
  }  
  pin_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
}
void parse_vcp(SerialHandler *h)
{	
}
int32_t tp[2] = {0, };
void loop_sync(void)
{  
  static int cnt = 0;		//	PDO timer
  //  val = func_sin(freq_cnt ++);
  if(((motor[3].Statusword >> 9) && 0x01) && (NMT_state == OP))
  {
	if(++cnt>=5)
	{
	  SYNC_FRAME();
	  cnt = 0;
	}
	else if(id_cnt == 4)
	{
	  if(id_sum == 6)
	  {
		if(I2C_TRQ_FLAG)
		{
		  motor[0].angle = motor[0].angle + rad_i2c[1];
		  motor[1].angle = motor[1].angle - rad_i2c[4];
		  trq_offset[0] = -arm_cos_f32(rad_i2c[0]);	trq_offset[2] = -arm_cos_f32(rad_i2c[0]);
		  trq_offset[1] = -arm_cos_f32(rad_i2c[3]);	trq_offset[3] = -arm_cos_f32(rad_i2c[3]);
		}
		TRQ_Calc();
		POS_Calc();
		for(int i=1;i<5;i++){SET_PDO(i);}	id_cnt = 0; id_sum = 0;
	  }
	  else
	  {
		error_res[0] = 1;
		id_cnt = 0;
		id_sum = 0;
	  }
	}
  }  
}
void loop_async(void)
{
  tick = HAL_GetTick();
  vcp.run(&vcp);    
  if(UART_FLAG)
  {
//	serial_print(&vcp, "AT+CONMAC=70B8F6977692\r");	UART_FLAG = false;
	vcp.tx_buffer[0] = NMT_state;
	vcp.tx_buffer[1] = DS_state;
	serial_write(&vcp, 2);
  }
  if(NMT_FLAG){	NMT_TRANS(NMT_state);	NMT_FLAG = false;	STATUS_FLAG = true;}
  if(DS_FLAG){	for(int i=1;i<5;i++){DS_TRANS(i, DS_state);}	DS_FLAG = false;	STATUS_FLAG = true;}
  if(QS_flag){	for(int i=1;i<5;i++){DS_TRANS(i, QS);}			QS_flag	= false;}    
  if(ANGLE_FLAG){	for(int i=1;i<5;i++){GET_Angle(i);}	ANGLE_FLAG = false;}
  if(CLEAR_ERROR_FLAG){		for(int i=1;i<5;i++){Clear_Device_Errors(i);}	CLEAR_ERROR_FLAG = false;	STATUS_FLAG = true;}
  if(SET_ANGLE_ZERO_FLAG){	for(int i=1;i<5;i++){GET_Angle(i);	motor[i-1].Position_zero = motor[i-1].Postion_actual;}	SET_ANGLE_ZERO_FLAG = false;}
  if(GET_SDO_FLAG){	GET_SDO(id_temp, object, object_sub);	GET_SDO_FLAG = false;}
  if(SET_SDO_FLAG){	SET_SDO(id_temp, length_temp, object, object_sub, data_temp);	SET_SDO_FLAG = false;}  
  if(STATUS_FLAG){	for(int i=1;i<5;i++){READ_STATUS(i);}	STATUS_FLAG = false;}
  if(I2C_FLAG){	I2C_DMA_COMM();}
  timer = HAL_GetTick() - tick;
}
int16_t func_sin(uint32_t freq)
{  
  if(freq <= period/2){
	val = (int16_t)(amp);
  }else if(freq <= period){
	val = (int16_t)(-amp);
  }else{
	freq = 0;
  }
  return (int16_t)(amp*arm_sin_f32(2.0*PI*(double)freq/period)); // 주기 0.5초 
}
void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan)
{
  txfifoemptycheck ++;
}
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_RX_FIFO0_MESSAGE_LOST)){
	__HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_RX_FIFO0_MESSAGE_LOST);
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
	  error_res[0] = 2;
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
		id_cnt ++;
		id_sum = id_sum + node_id;
		if(motor[node_id].parsing_PDO(&motor[node_id], node_id) != 1){
		  QS_flag = true;
		  NMT_state = PRE;
		}
		break;
		// case PDO2:
	  }
	}
	if(HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) >= 2){	
	  while(HAL_FDCAN_GetRxMessage(hcan.module, hcan.rxloc, &hcan.rxmsg.header, hcan.rxmsg.data) != HAL_OK);
	}
  }
}
void HAL_SYSTICK_Callback(void)	// 1mhz timer
{
  loop_sync();
}
uint8_t tx_buff[1] = {1};
uint8_t rx_buff[12] = {0, };
uint16_t mem_addr = 0x3d;
double angle_i2c[12] = {0, };
double rad_i2c[12] = {0, };
void I2C_DMA_COMM()
{
  static double coef1 = 5.493164 / 1000;
  static double coef2 = 0.0958738 / 1000;
  do
  {
	if (HAL_I2C_Mem_Write_DMA(&hi2c1, (uint16_t)(0x51 << 1), (uint16_t)mem_addr, 1, tx_buff, 1) != HAL_OK)
	{
	  Error_Handler();
	}
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
	{
	}
	
  }
  while (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);
  HAL_Delay(1);
  do
  {
	if (HAL_I2C_Mem_Read_DMA(&hi2c1, (uint16_t)(0x51 << 1), (uint16_t)mem_addr, 1, &rx_buff[0], 6) != HAL_OK)
	{
	  Error_Handler();
	}
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
	{
	}	
  }
  while (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);
  
  HAL_Delay(1);
  
  do
  {
	if (HAL_I2C_Mem_Write_DMA(&hi2c1, (uint16_t)(0x53 << 1), (uint16_t)mem_addr, 1, tx_buff, 1) != HAL_OK)
	{
	  Error_Handler();
	}
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
	{
	}	
  }
  while (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);
  HAL_Delay(1);
  do
  {
	if (HAL_I2C_Mem_Read_DMA(&hi2c1, (uint16_t)(0x53 << 1), (uint16_t)mem_addr, 1, &rx_buff[6], 6) != HAL_OK)
	{
	  Error_Handler();
	}
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
	{
	}	
  }
  while (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF); 
  
  angle_i2c[0] = ((int16_t)(rx_buff[0] | rx_buff[1] << 8)) * coef1;
  angle_i2c[1] = ((int16_t)(rx_buff[2] | rx_buff[3] << 8)) * coef1;
  angle_i2c[2] = ((int16_t)(rx_buff[4] | rx_buff[5] << 8)) * coef1;
  angle_i2c[3] = ((int16_t)(rx_buff[6] | rx_buff[7] << 8)) * coef1;
  angle_i2c[4] = ((int16_t)(rx_buff[8] | rx_buff[9] << 8)) * coef1;
  angle_i2c[5] = ((int16_t)(rx_buff[10] | rx_buff[11] << 8)) * coef1;
  
  
  rad_i2c[0] = ((int16_t)(rx_buff[0] | rx_buff[1] << 8)) * coef2;
  rad_i2c[1] = ((int16_t)(rx_buff[2] | rx_buff[3] << 8)) * coef2;
  rad_i2c[2] = ((int16_t)(rx_buff[4] | rx_buff[5] << 8)) * coef2;
  rad_i2c[3] = ((int16_t)(rx_buff[6] | rx_buff[7] << 8)) * coef2;
  rad_i2c[4] = ((int16_t)(rx_buff[8] | rx_buff[9] << 8)) * coef2;
  rad_i2c[5] = ((int16_t)(rx_buff[10] | rx_buff[11] << 8)) * coef2;  
  
}
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){ //1khz timer
//  
//}

////////////////////////////////////
//void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//  if (hi2c == &hi2c1)
//  {
//	if (!state) { 
//	  HAL_I2C_Mem_Read_IT(&hi2c1, addr1, (uint16_t)0x3d, 1, data1, 6);
//	}
//	else{
//	  HAL_I2C_Mem_Read_IT(&hi2c1, addr2, (uint16_t)0x3d, 1, data2, 6);
//	}
//  }
//}
//void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//  if (hi2c == &hi2c1)
//  {
//	if (!state) {
//	  sen1.roll = (float)(data1[0] | (data1[1] << 8)) / 32768 * 180;
//	  sen1.pitch = (float)(data1[2] | (data1[3] << 8)) / 32768 * 180;
//	  sen1.yaw = (float)(data1[4] | (data1[5] << 8)) / 32768 * 180;
//	  
//	  
//	  state = true;
//	  HAL_I2C_Mem_Write_IT(&hi2c1, addr2, (uint16_t)0x3d, (uint16_t)1, tx_buf1, (uint16_t)1);
//	}
//	else{
//	  sen2.roll = (float)(data1[0] | (data2[1] << 8)) / 32768 * 180;
//	  sen2.pitch = (float)(data1[2] | (data2[3] << 8)) / 32768 * 180;
//	  sen2.yaw = (float)(data1[4] | (data2[5] << 8)) / 32768 * 180;
//	  
//	  
//	  time_interval = HAL_GetTick() - start_time;
//	  
//	  
//	  start_time = HAL_GetTick();
//	  state = false;
//	  HAL_I2C_Mem_Write_IT(&hi2c1, addr1, (uint16_t)0x3d, (uint16_t)1, tx_buf2, (uint16_t)1);
//	}
//  }
//}
//void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
//{
//  if (hi2c == &hi2c1)
//  {
//	state = false;
//	HAL_I2C_Mem_Write_IT(&hi2c1, addr1, (uint16_t)0x3d, (uint16_t)1, tx_buf2, (uint16_t)1);
//  }
//}
////////////////////////////////////////
/* USER CODE END 4 */

/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  ERROR_FLAG = true;
  NMT_state = PRE;
  DS_state = SD;
  for(int i=1;i<5;i++){
	DS_TRANS(i, DS_state);
  }
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
