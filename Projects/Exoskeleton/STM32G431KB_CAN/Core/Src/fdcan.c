/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file    fdcan.c
* @brief   This file provides code for the configuration
*          of the FDCAN instances.
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

#include "stdbool.h"
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "fdcan.h"

/* USER CODE BEGIN 0 */
CAN_HandleTypeDef hcan = {
  &hfdcan1, 
  {0,}, 
  {0,}, 
  FDCAN_TX_BUFFER0,				// FDCAN_TX_BUFFER0, 1, 2
  FDCAN_RX_FIFO0,					// FDCAN_RX_FIFO0, 1
  FDCAN_IT_RX_FIFO0_NEW_MESSAGE,	// FDCANIT_RX_FIFO0_MESSAGE_LOST/FULL/NEW_MESSAGE(New message written to Rx FIFO 0)
};
double FA_limit_angle[2]	= {1.9628, 0};			//	112.46도, 0도
double UA_limit_angle[2]	= {2.5187, -1.321};	//144.31도, -75.69도
uint8_t CST_mode[4] = {1, 1, 1, 1};
uint32_t max_timeout_cnt = 100000;
bool RECV_FLAG = false;
bool TRQ_ON_FLAG = false;
bool TRQ_OFF_FLAG = false;

char arms = 'B';
double torque[4] = {0, };
double deg2rad_5 = 0.0873; // 상, 하방 limit  5도
uint8_t trq_prof[2] = {1, 1};
uint16_t cnt_max = 3000;
/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */
  
  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */
  
  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 10;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 14;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 28;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */
  FDCAN_FilterTypeDef sFilterConfig; // handle for filter config
  /* 나중에 exteded ID가 들어오는 경우 예외처리 하기 또는 유입 막기 */
  sFilterConfig.IdType = FDCAN_STANDARD_ID;				// identifier type, FDCAN_STANDARD_ID, FDCAN_EXTENDED_ID
  sFilterConfig.FilterIndex = 0;						// filter which will be initialized, 0 ~ SRAMCAN_FLS_NBR-1
  sFilterConfig.FilterType = FDCAN_FILTER_RANGE;		// FDCAN_FILTER_RANGE: FilterID1 to FilterID2, DUAL: Dual ID filter, MASK: FilterID1=filter, FilterID2 = mask
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // 
  sFilterConfig.FilterID1 = 0x000; // from
  sFilterConfig.FilterID2 = 0x7ff; // to
  
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
  {
	Error_Handler();
  }	  
  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */
	
  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
  /* USER CODE BEGIN FDCAN1_MspInit 1 */
	
  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */
	
  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */
	
  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
float timeout[2] = {0.0, };
void SET_SDO(uint8_t id, uint8_t length, Obj_dict_t addr, uint8_t addr_sub, int32_t data)
{
  hcan.txmsg.header.Identifier = 0x600 + id;
  switch (length){
  case 1:
	hcan.txmsg.data[0] = 0x2F;
	hcan.txmsg.data[4] = data;
	break;	
  case 2:
	hcan.txmsg.data[0] = 0x2B;
	hcan.txmsg.data[4] = data & 0xff;
	hcan.txmsg.data[5] = (data >> 8) & 0xff;
	break;
  case 4:
	hcan.txmsg.data[0] = 0x23;
	hcan.txmsg.data[4] = data & 0xff;
	hcan.txmsg.data[5] = (data >> 8) & 0xff;
	hcan.txmsg.data[6] = ((data >> 8)>>8) & 0xff;
	hcan.txmsg.data[7] = (((data >> 8)>>8)>>8) & 0xff;
	break;
  default:
	hcan.txmsg.data[0] = 0x22;
  }
  hcan.txmsg.data[1] = addr & 0xFF;
  hcan.txmsg.data[2] = (addr >> 8) & 0xFF;  
  hcan.txmsg.data[3] = addr_sub;
  
  SEND_FRAME(&hcan);
}
//void SET_PDO(const Motor_t* h){ // const: 주소값 안의 내용만 참조 
void SET_PDO(uint8_t id)
{
  hcan.txmsg.header.Identifier = 0x200 + id;
  //  hcan.txmsg.data[0] = h->Controlword& 0xff;
  //  hcan.txmsg.data[1] = (h->Controlword >> 8) & 0xff;
  if(CST_mode[id-1])
  {
	hcan.txmsg.data[0] = motor[id-1].Target_torque & 0xff;
	hcan.txmsg.data[1] = (motor[id-1].Target_torque >> 8) & 0xff;  
  }else
  {
	hcan.txmsg.data[0] = motor[id-1].Target_position & 0xff;
	hcan.txmsg.data[1] = (motor[id-1].Target_position>> 8) & 0xff;  
	hcan.txmsg.data[2] = (motor[id-1].Target_position >> 16) & 0xff;  
	hcan.txmsg.data[3] = (motor[id-1].Target_position >> 24) & 0xff;  
  }
  SEND_FRAME(&hcan); 
}
void NMT_TRANS(NMT_state_t state)
{
  memset(hcan.txmsg.data, 0, 8);
  hcan.txmsg.header.Identifier = 0x00;
  hcan.txmsg.data[0] = state;
  //  hcan.txmsg.data[1] = id;
  hcan.txmsg.data[1] = 0; // all nodes  
  SEND_FRAME(&hcan);
}
void DS_TRANS(uint8_t id, DS_state_t state)
{
  timeout[0] = 0;
  SET_SDO(id, sizeof(uint16_t), CONTROLWORD, 0, state);	// RECV_FLAG = false;
  while(!RECV_FLAG && motor[id-1].Object != CONTROLWORD){
	if(++timeout[0] >= max_timeout_cnt){
	  error_res[0] = 5;
	  Error_Handler();
	}
  }
}
void Parsing_SDO(Motor_t *h, uint8_t id)
{
  RECV_FLAG = true;
  h->id			= id+1 ;
  h->Object	= (Obj_dict_t)(hcan.rxmsg.data[1] | (hcan.rxmsg.data[2] << 8));
  switch(h->Object){
  case STATUSWORD:
	h->Statusword = hcan.rxmsg.data[4] | (hcan.rxmsg.data[5] << 8);
	break;
  case Error_code:
	h-> Error_code[h->error_index] = hcan.rxmsg.data[4] | (hcan.rxmsg.data[5] << 8);
	h-> error_index = h->error_index + 1;
	if(h-> error_index == 5)	h->error_index = 0;
	break;
  case Position_actual:
	h->Position_actual = hcan.rxmsg.data[4] | (hcan.rxmsg.data[5] << 8) | (hcan.rxmsg.data[6] << 16) | (hcan.rxmsg.data[7] << 24);
	h->angle = (h->Position_actual - h->Position_zero) * 0.000767; // 2*Pi / 8192
	h->angle = h->angle / gear_ratio[id];
	break;
  default:
	if(hcan.rxmsg.data[0] == 0x4F)	h->value = hcan.rxmsg.data[4];
	else if(hcan.rxmsg.data[0] == 0x4B) h->value = hcan.rxmsg.data[4] | (hcan.rxmsg.data[5] << 8);
	else if(hcan.rxmsg.data[0] == 0x43) h->value = hcan.rxmsg.data[4] | (hcan.rxmsg.data[5] << 8) | (hcan.rxmsg.data[6] << 16) | (hcan.rxmsg.data[7] << 24);
	break;
  }  
}
uint8_t Parsing_PDO(Motor_t *h, uint8_t id)
{
  h->id = id+1;
  h->Object = PDO_OBJ;
  h->Statusword = hcan.rxmsg.data[0] | (hcan.rxmsg.data[1] << 8);
  h->Torque_actual = hcan.rxmsg.data[2] | (hcan.rxmsg.data[3] << 8);
  h->Position_actual = hcan.rxmsg.data[4] | (hcan.rxmsg.data[5] << 8) | (hcan.rxmsg.data[6] << 16) | (hcan.rxmsg.data[7] << 24);
  h->angle = (h->Position_actual - h->Position_zero) * 0.000767; // 2*Pi / 8192
  h->angle = h->angle / gear_ratio[id];
  if((h->Statusword >> 3) & 0x01){	//  Fault
	return 0;
  }else{
	return 1;
  }
}
void SYNC_FRAME(void)
{
  hcan.txmsg.header.Identifier = 0x80;
  memset(hcan.txmsg.data,0,8);  
  SEND_FRAME(&hcan);
}
void SEND_FRAME(CAN_HandleTypeDef *h)
{    
  RECV_FLAG = false;
  timeout[0] = 0;  
  while(__HAL_FDCAN_GET_FLAG(h->module, FDCAN_FLAG_TX_FIFO_EMPTY))
  {
	if(++timeout[0] >= max_timeout_cnt)
	{
	  error_res[0] = 3;
	  Error_Handler();
	}
  }
  timeout[1] = error_res[1];
  arm_max_no_idx_f32(timeout, 2, &error_res[1]);
  //	if (HAL_FDCAN_GetTxFifoFreeLevel(hcan.module) > 0)
  timeout[0]= 0;
  while(HAL_FDCAN_GetTxFifoFreeLevel(h->module) == 0)
  {
	if(++timeout[0] >= max_timeout_cnt){
	  error_res[0] = 4;
	  Error_Handler();
	}
  }
  timeout[1] = error_res[1];
  arm_max_no_idx_f32(timeout, 2, &error_res[1]);
  if (HAL_FDCAN_AddMessageToTxFifoQ(h->module, &h->txmsg.header, h->txmsg.data) != HAL_OK)
  {
	error_res[0] = 5;
	Error_Handler();
  }  
  if(__HAL_FDCAN_GET_FLAG(h->module, FDCAN_FLAG_TX_EVT_FIFO_ELT_LOST))
  {
	error_res[0] = 6;
	Error_Handler();
  }   
}
void GET_SDO(uint8_t id, Obj_dict_t addr, uint8_t addr_sub)
{
  hcan.txmsg.header.Identifier = 0x600 + id;
  hcan.txmsg.data[0] = 0x40;  
  hcan.txmsg.data[1] = addr & 0xFF;
  hcan.txmsg.data[2] = (addr >> 8) & 0xFF;  
  hcan.txmsg.data[3] = addr_sub;
  SEND_FRAME(&hcan);    
}
void READ_STATUS(uint8_t id)
{
  timeout[0] = 0;
  GET_SDO(id, STATUSWORD, 0);		// RECV_FLAG = false;
  while(!RECV_FLAG && motor[id-1].Object != STATUSWORD){
	if(++timeout[0] >= max_timeout_cnt){
	  error_res[0] = 5;
	  Error_Handler();
	}
  }
  timeout[1] = error_res[1];
  arm_max_no_idx_f32(timeout, 2, &error_res[1]);
  if((motor[id-1].Statusword >> 3) & 0x01){	// if error states
	GET_SDO(id, Error_code, 0);						// RECV_FLAG = false;, get error code
	while(!RECV_FLAG && motor[id-1].Object == Error_code);
	if(motor[id-1].Error_code[(motor[id-1].error_index+4)%5] == (uint16_t)0x8110){
	  Clear_Device_Errors(id);
	}
  }
}
void GET_Angle(uint8_t id)
{
  timeout[0] = 0;
  GET_SDO(id, Position_actual, 0);
  while(!RECV_FLAG && motor[id-1].Object != Position_actual)
  {
	if(++timeout[0] >= max_timeout_cnt){
	  error_res[0] = 5;
	  Error_Handler();
	}
  }
  timeout[1] = error_res[1];
  arm_max_no_idx_f32(timeout, 2, &error_res[1]);
}
void Clear_Device_Errors(uint8_t id)
{
  //  SET_SDO(id, sizeof(uint8_t), Error_history, 0, 0);
  DS_TRANS(id, FR);
  READ_STATUS(id);
}
void TRQ_Calc()
{  
  const double FA_limit_trq_angle[2] = {FA_limit_angle[0] - deg2rad_5, FA_limit_angle[1] + deg2rad_5};	// forearm torque limit angle, 상방 limit angle - 5°, 하방 zero angle + 5°
  const double UA_limit_trq_angle[2] = {UA_limit_angle[0] - deg2rad_5, UA_limit_angle[1] + deg2rad_5};	// upperarm torque limit angle, 상방 limit angle - 5°, 하방 limit anlge + 5°
  static double theta2[2] = {0, };
  static uint16_t trq_cnt_old;
  static uint16_t trq_cnt_new;
  pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
  if(pin_state == GPIO_PIN_SET)		//stretch_btn flag 추가하기
  {
	if(TRQ_ON_FLAG)
	{
	  TRQ_ON_FLAG = false;
	}
	if(!TRQ_OFF_FLAG)
	{
	  trq_cnt_old = HAL_GetTick();
	  TRQ_OFF_FLAG = true;
	}
  }
  else
  {
	if(!TRQ_ON_FLAG)
	{
	  trq_cnt_old = HAL_GetTick();
	  TRQ_ON_FLAG = true;
	  TRQ_OFF_FLAG = false;
	}
	theta2[0] = motor[0].angle + motor[2].angle;		// Right forearm angle
	theta2[1] = motor[1].angle + motor[3].angle;		// Left forearm angle
	
	/* trq profile selection */
	switch(trq_prof[1])
	{
	case 1:
	  /** V4 SHEET 7  **/
	  torque[2] = (0.36673 * theta2[0] * theta2[0] + 0.285765 * theta2[0] - 5.19364) * 1000 * (-theta2[0]);
	  torque[3] = (0.36673 * theta2[1] * theta2[1] + 0.285765 * theta2[1] - 5.19364) * 1000 * (-theta2[1]);
	  break;
	case 2:
	  torque[2] = (cg_forearm * weight_forearm + l2 * weight) * (16.4864* theta2[0] * theta2[0] - 26 * theta2[0] - 87.0745) * theta2[0]  * (-8.153);
	  torque[3] = (cg_forearm * weight_forearm + l2 * weight) * (16.4864* theta2[1] * theta2[1] - 26 * theta2[1] - 87.0745) * theta2[1]  * (-8.153);
	  //		  torque[2] = (82.835 * theta2[0] * theta2[0] - 130.61 * theta2[0] - 437.5) * theta2[0]  * (-8.153);	//up 0.5 down 0.5
	  //		  torque[3] = (82.835 * theta2[1] * theta2[1] - 130.61 * theta2[1] - 437.5) * theta2[1]  * (-8.153);	//up 0.5 down 0.5
	  break;
	case 3:
	  /** up 0.3 down 0.7 통합 문서1 Sheet 9 **/
	  //		torque[2] = (cg_forearm * weight_forearm + l2 * weight) * (13.9318 * theta2[0] * theta2[0] - 23.9016 * theta2[0] - 74.8051) * theta2[0]  * (-8.153); // up 0.5 down 0.5, -8.153 = gear_ratio[3] * rated_torque / 1000.0;
	  //		torque[3] = (cg_forearm * weight_forearm + l2 * weight) * (13.9318 * theta2[1] * theta2[1] - 23.9016 * theta2[1] - 74.8051) * theta2[1]  * (-8.153); // up 0.5 down 0.5, -8.153 = gear_ratio[3] * rated_torque / 1000.0;
	  //		  torque[2] = (70 * theta2[0] * theta2[0] - 120.1 * theta2[0] - 375.8546) * theta2[0]  * (-8.153); // up 0.5 down 0.5, -8.153 = gear_ratio[3] * rated_torque / 1000.0;  ver.1 
	  
	  /** up 0.5 down 0.5 v2 Sheet 2 upper arm angle 90°**/
	  torque[2] = (cg_forearm * weight_forearm + l2 * weight) * (1.902 * theta2[0] * theta2[0] + 39.3152 * theta2[0] - 146.0728) * theta2[0]  * (-8.153); // up 0.5 down 0.5, -8.153 = gear_ratio[3] * rated_torque / 1000.0;
	  torque[3] = (cg_forearm * weight_forearm + l2 * weight) * (1.902 * theta2[1] * theta2[1] + 39.3152 * theta2[1] - 146.0728) * theta2[1]  * (-8.153); // up 0.5 down 0.5, -8.153 = gear_ratio[3] * rated_torque / 1000.0;
	  //		  torque[2] = (10.7849 * theta2[0] * theta2[0] + 230.8379* theta2[0] - 857.6604) * theta2[0]  * (-8.153); //  ver.2 08.08 @ 90degree		
	  break;		
	case 4:
	  /**  up0.4 down 0.6 v3 sheet 3 **/
	  torque[2] = (0.972412 * theta2[0] * theta2[0] -1.476404 * theta2[0] - 4.444624) * 1000 * (-theta2[0]);
	  torque[3] = (0.972412 * theta2[1] * theta2[1] -1.476404 * theta2[1] - 4.444624) * 1000 * (-theta2[1]);
	  break;
	  
	case 5:
	  /**  V4 SHHET 3 UP V1 DOWN 0.1 **/
	  //		torque[2] = (0.74157 * theta2[0] * theta2[0] -1.071351 * theta2[0] - 4.3864) * 1000 * (-theta2[0]);
	  torque[2] = (0.75233 * theta2[0] * theta2[0] -1.1051 * theta2[0] - 4.30216) * 1000 * (-theta2[0]);
	  //		torque[3] = (0.74157 * theta2[1] * theta2[1] -1.071351 * theta2[1] - 4.3864) * 1000 * (-theta2[1]);
	  torque[3] = (0.75233 * theta2[1] * theta2[1] -1.1051 * theta2[1] - 4.30216) * 1000 * (-theta2[1]);
	  break;
	  
	case 6:
	  torque[2] = (cg_forearm * weight_forearm + l2 * weight) * arm_sin_f32(theta2[0]) * 1000;
	  torque[3] = (cg_forearm * weight_forearm + l2 * weight) * arm_sin_f32(theta2[1]) * 1000;
	  break;
	}
	
	switch(trq_prof[0])
	{
	case 1:
	  /** V4 SHHET 7 **/
	  torque[0] = torque[2] + (-1.19887 * motor[0].angle * motor[0].angle -0.23544 * motor[0].angle + 10.86467) * 1000 * motor[0].angle;
	  torque[1] = torque[3] + (-1.19887 * motor[1].angle * motor[1].angle -0.23544 * motor[1].angle + 10.86467) * 1000 * motor[1].angle;
	  break;
	case 2:
	  /** ver 1 **/
	  torque[0] = torque[2] + (cg_upperarm * weight_upperarm + l1 * weight) * (-7.0957* motor[0].angle * motor[0]. angle + 3.6319 * motor[0].angle + 52.0854) * motor[0].angle * 16.17822;
	  torque[1] = torque[3] + (cg_upperarm * weight_upperarm + l1 * weight) * (-7.0957* motor[1].angle * motor[1]. angle + 3.6319 * motor[1].angle + 52.0854) * motor[1].angle * 16.17822;
	  //			torque[0] = torque[2] + (-72.2326 * motor[0].angle * motor[0].angle + 36.97* motor[0].angle + 530.22) *  motor[0].angle * 16.17822; // up 0.4 down 0.6 16.17822 = gear_ratio[0] * rated_torque / 1000.0;
	  //			torque[1] = torque[3] + (-72.2326 * motor[1].angle * motor[1].angle + 36.97* motor[1].angle + 530.22) *  motor[1].angle * 16.17822; // up 0.4 down 0.6 16.17822 = gear_ratio[0] * rated_torque / 1000.0;
	  break;
	case 3:
	  /** v2 sheet 5 up 0.5 down 0.5**/
	  //			torque[0] = torque[2] + (-94.18751475 * motor[0].angle * motor[0].angle + 64.89141175 * motor[0].angle + 587.1707543) *  motor[0].angle * 16.17822; // up 0.4 down 0.6 16.17822 = gear_ratio[0] * rated_torque / 1000.0;
	  torque[0] = torque[2] + (cg_upperarm * weight_upperarm + l1 * weight) * (-8.7628* motor[0].angle * motor[0]. angle + 6.0372 * motor[0].angle + 54.628) * motor[0].angle * 16.17822;
	  torque[1] = torque[3] + (cg_upperarm * weight_upperarm + l1 * weight) * (-8.7628* motor[1].angle * motor[1]. angle + 6.0372 * motor[1].angle + 54.628) * motor[1].angle * 16.17822;		
	  break;
	case 4:
	  /** v3 sheet 1 up 0.5 down 0.5 **/
	  torque[0] = torque[2] + (-1.19672 * motor[0].angle * motor[0].angle + 0.080715 * motor[0].angle + 9.855825) * 1000 * motor[0].angle;
	  torque[1] = torque[3] + (-1.19672 * motor[1].angle * motor[1].angle + 0.080715 * motor[1].angle + 9.855825) * 1000 * motor[1].angle;
	  break;
	case 5:
	  /** v4 sheet 4 up v3 & down v2 **/
	  torque[0] = torque[2] + (-0.577883 * motor[0].angle * motor[0].angle - 0.72674		* motor[0].angle + 9.7339) * 1000 * motor[0].angle;
	  torque[1] = torque[3] + (-0.577883 * motor[1].angle * motor[1].angle - 0.72674 		* motor[1].angle + 9.7339) * 1000 * motor[1].angle;
	  break;
	  
	case 6:
	  torque[0] = torque[2] + (cg_upperarm * weight_upperarm + l1 * weight) * arm_sin_f32(motor[0].angle) * 1000;
	  torque[1] = torque[3] + (cg_upperarm * weight_upperarm + l1 * weight) * arm_sin_f32(motor[1].angle) * 1000;
	  break;
	}
	
	switch(arms)
	{
	/* actuating Right arm, node id 1, 3(motor[0], motor[2]) */
	case 'R' :
	  torque[1] = 0;
	  torque[3] = 0;	  
	  if(motor[2].angle >= FA_limit_trq_angle[0]) torque[2] = 0;
	  if(motor[0].angle >= UA_limit_trq_angle[0]) torque[0] = 0;
	  
	  torque[2] = torque[2] * trq_offset[2];
	  torque[0] = torque[0] * trq_offset[0];	
	  break;
	  
	/* actuating Left arm, node id 2, 4(motor[1], motor[3]) */
	case 'L' :
	  torque[0] = 0;
	  torque[2] = 0;	    
	  if(motor[3].angle <= FA_limit_trq_angle[1]) torque[3] = 0;	  
	  if(motor[1].angle <= UA_limit_trq_angle[1]) torque[1] = 0;	  
	  torque[3] = torque[3] * trq_offset[3];
	  torque[1] = torque[1] * trq_offset[1];	  
	  break;
	  /* actuating both arms */
	case 'B' :
	  if(motor[2].angle >= FA_limit_trq_angle[0]) torque[2] = 0;
	  else	torque[2] = torque[2] * trq_offset[2];
	  if(motor[0].angle >= UA_limit_trq_angle[0]) torque[0] = 0;
	  else	torque[0] = torque[0] * trq_offset[0];	
	  
	  if(motor[3].angle <= FA_limit_trq_angle[1]) torque[3] = 0;
	  else	torque[3] = torque[3] * trq_offset[3];
	  if(motor[1].angle <= UA_limit_trq_angle[1]) torque[1] = 0;
	  else	torque[1] = torque[1] * trq_offset[1];	  	  
	  break;
	  
	default :
	  torque[0] = 0;	torque[1] = 0;	torque[2] = 0;	torque[3] = 0;
	}
	
	if(TRQ_ON_FLAG)
	{
	  trq_cnt_new = HAL_GetTick() - trq_cnt_old;
	  if(trq_cnt_new < cnt_max)
	  {
		for(int i=0;i<4;i++)
		{
		  torque[i] = torque[i] * trq_cnt_new / cnt_max;
		}
	  }
	}
  }
  if(TRQ_OFF_FLAG)
  {
	trq_cnt_new = HAL_GetTick() - trq_cnt_old;
	if(trq_cnt_new < cnt_max/2)
	{
	  for(int i=0;i<4;i++)
	  {
		torque[i] = torque[i] * (cnt_max/2 - trq_cnt_new)/(cnt_max/2);
		if(torque[i] < 0.001)	torque[i] = 0;
	  }
	}
  }	
  
  /* gear efficiency */
  for(int i=0;i<4;i++)
  {
	motor[i].Target_torque = (int16_t)(torque[i] / gear_ratio[i] / rated_torque * 1000.0  * gear_efficiency[i]);
  }
}
void TRQ_Calc_2(void)
{
  const double FA_limit_trq_angle[2] = {FA_limit_angle[0] - deg2rad_5, FA_limit_angle[1] + deg2rad_5};	// forearm torque limit angle, 상방 limit angle - 5°, 하방 zero angle + 5°
  const double UA_limit_trq_angle[2] = {UA_limit_angle[0] - deg2rad_5, UA_limit_angle[1] + deg2rad_5};	// upperarm torque limit angle, 상방 limit angle - 5°, 하방 limit anlge + 5°
  static uint16_t trq_cnt_old;
  static uint16_t trq_cnt_new;
  pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
  if(pin_state == GPIO_PIN_SET)		//stretch_btn flag 추가하기
  {
	if(TRQ_ON_FLAG)
	{
	  TRQ_ON_FLAG = false;
	}
	if(!TRQ_OFF_FLAG)
	{
	  trq_cnt_old = HAL_GetTick();
	  TRQ_OFF_FLAG = true;
	}
  }
  else
  {
	if(!TRQ_ON_FLAG)
	{
	  trq_cnt_old = HAL_GetTick();
	  TRQ_ON_FLAG = true;
	  TRQ_OFF_FLAG = false;
	}
	torque[0] = 0;
	torque[2] = 0;
	
	torque[3] = (l2 * weight + cg_forearm * weight_forearm) * arm_sin_f32(motor[1].angle + motor[3].angle);
	torque[1] = torque[3] + (l1 * weight + cg_upperarm + weight_upperarm) * arm_sin_f32(motor[1].angle);
	
	if(motor[3].angle <= FA_limit_trq_angle[1]) torque[3] = 0;
	else	torque[3] = torque[3] * trq_offset[3];
	if(motor[1].angle <= UA_limit_trq_angle[1]) torque[1] = 0;
	else	torque[1] = torque[1] * trq_offset[1];
	
	if(TRQ_ON_FLAG)
	{
	  trq_cnt_new = HAL_GetTick() - trq_cnt_old;
	  if(trq_cnt_new < cnt_max)
	  {
		for(int i=0;i<4;i++)
		{
		  torque[i] = torque[i] * trq_cnt_new / cnt_max;
		}
	  }
	}
  }
  if(TRQ_OFF_FLAG)
  {
	trq_cnt_new = HAL_GetTick() - trq_cnt_old;
	if(trq_cnt_new < cnt_max/2)
	{
	  for(int i=0;i<4;i++)
	  {
		torque[i] = torque[i] * (cnt_max/2 - trq_cnt_new)/(cnt_max/2);
		if(torque[i] < 0.001)	torque[i] = 0;;
	  }
	}
  }	
  for(int i=0;i<4;i++)
  {
	motor[i].Target_torque = (int16_t)(torque[i] / gear_ratio[i] / rated_torque * 1000.0  * gear_efficiency[i] * 1000.0);
  }
}

int inc1 = 50;
int inc2 = 50;
bool sign1_flag = true;
bool sign3_flag = true;
GPIO_PinState pin_state_old = GPIO_PIN_SET;

void POS_Calc()
{
  pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
  
  /**		tp[1] elbow angle torque check		**/
  
  if(pin_state == GPIO_PIN_SET)
  {
	if(pin_state != pin_state_old)
	{
	  //	  tp[0] = motor[1].Postion_actual;
	  tp[1] = motor[3].Position_actual;
	  sign3_flag = !sign3_flag;
	  pin_state_old = pin_state;
	}
  }else
  {
	pin_state_old = pin_state;
	if(sign3_flag)
	{
	  inc1 = abs(inc1);	  
	  //	  tp[0] = tp[0] + sign_;
	  tp[1] = tp[1] + inc1;
	}else
	{
	  inc1 = (-1) * abs(inc1);
	  //	  tp[0] = tp[0] + sign_;
	  tp[1] = tp[1] + inc1;
	}
  }
  if(tp[1] >= motor[3].Position_zero)		tp[1] = motor[3].Position_zero;
  
  
  /**		tp[0] shoulder angle torque check		**/
  
  if(!m1_flag)
  {
	if(m1_flag != m1_flag_old)
	{
	  tp[0] = motor[1].Position_actual;
	  sign1_flag = !sign1_flag;
	  m1_flag_old = m1_flag;
	}
  }else
  {
	m1_flag_old = m1_flag;
	if(sign1_flag)
	{
	  inc2 = abs(inc2);
	  tp[0] = tp[0] + inc2;
	  if(tp[0] >= motor[1].Position_zero + rad135*gear_ratio[1]*1303.7973)
		tp[0] = (int32_t)(rad135 * gear_ratio[1] * 1303.7973 + motor[1].Position_zero + abs(inc2));	// 8192 : 2π = x : 1 → x = 1303.7973, 90도 위치
	}else
	{
	  inc2 = (-1) * abs(inc2);
	  tp[0] = tp[0] + inc2;
	  if(tp[0] <= motor[1].Position_zero - rad30*gear_ratio[1]*1303.7923 + 1)
		tp[0] = (int32_t)(motor[1].Position_zero - rad30*gear_ratio[1]*1303.7923 + abs(inc2));
	}	
  }  
  motor[1].Target_position = tp[0];
  motor[3].Target_position = tp[1];
}
void POS_Calc_2(void)
{
  static uint16_t cnt, cnt_new;
  static int32_t tp_goal[2] = {0, 0};
  static int32_t tp_old[2] = {0, 0};
  pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
  if(pin_state == GPIO_PIN_SET)
  {
	if(pin_state != pin_state_old)
	{
	  tp[0] = motor[1].Position_actual;
	  tp[1] = motor[3].Position_actual;
	  pin_state_old = pin_state;
	}
  }else
  {
	if(pin_state != pin_state_old)
	{
	  cnt = HAL_GetTick();
	  pin_state_old = pin_state;
	  tp_goal[0] = motor[1].Position_zero + (int32_t)(tp_degree[0] * 22.7556 * gear_ratio[1]);	// 8192:360°=x:1° → x = 22.7556
	  tp_goal[1] = motor[3].Position_zero + (int32_t)(tp_degree[1] * 22.7556 * gear_ratio[3]);	// 8192:360°=x:1° → x = 22.7556
	  tp_old[0] = motor[1].Position_actual;
	  tp_old[1] = motor[3].Position_actual;
	}
	cnt_new = HAL_GetTick() - cnt;
	if(cnt_new <= cnt_max)
	{
	  tp[0] = tp_old[0] + ((tp_goal[0] - tp_old[0]) * cnt_new / cnt_max);
	  tp[1] = tp_old[1] + ((tp_goal[1] - tp_old[1]) * cnt_new / cnt_max);
	}
  }
  motor[1].Target_position = tp[0];
  motor[3].Target_position = tp[1];
}
Motor_t motor[4] = {
  MOTOR_DEFAULT,
  MOTOR_DEFAULT,
  MOTOR_DEFAULT,
  MOTOR_DEFAULT
};
void INIT_CAN()
{
  for(int i=1;i<5;i++){
	READ_STATUS(i);
	if(CST_mode[i-1])
	{
	  SET_SDO(i, sizeof(uint8_t), MOP, 0x00, 10);								// CST
	  SET_SDO(i, sizeof(uint8_t), RXPDO1, 0x00, 0);
	  SET_SDO(i, sizeof(uint32_t), RXPDO1, 0x01, 0x60710010);		// RXPDO1 target torque
	  SET_SDO(i, sizeof(uint8_t), RXPDO1, 0x00, 1);
	}else
	{
	  SET_SDO(i, sizeof(uint8_t), MOP, 0x00, 8);								// CSP
	  SET_SDO(i, sizeof(uint8_t), RXPDO1, 0x00, 0);
	  SET_SDO(i, sizeof(uint32_t), RXPDO1, 0x01, 0x607A0020);		// RXPOD1 target position
	  SET_SDO(i, sizeof(uint8_t), RXPDO1, 0x00, 1);
	  SET_SDO(i, sizeof(uint32_t), FlW_ERR_WIN, 0x00, 2500);
	}	
	SET_SDO(i, sizeof(uint8_t), CAN_bit_rate, 0x00, 0);	
	SET_SDO(i, sizeof(uint32_t), MAX_MOTOR_SPEED, 0x00, 5600);		// max motor speed 4000 RPM
	SET_SDO(i, sizeof(uint32_t), TRQ_CONST, 0x05, 42172);				// rated torque 167 / nominal current 3960 * 1000
	
	DS_TRANS(i, DV);
  }
  HAL_Delay(50);
  NMT_TRANS(PRE);
  //	while(!__HAL_FDCAN_GET_FLAG(&hfdcan1, FDCAN_FLAG_RX_FIFO0_NEW_MESSAGE)){}
}
int Check_status(void)
{  
  if(((motor[0].Statusword >> 3) & 0x01) || ((motor[1].Statusword >> 3) & 0x01) || ((motor[2].Statusword >> 3) & 0x01) || ((motor[3].Statusword >> 3) & 0x01))
  {
	return 0;		// fault state
  }else if(motor[0].Statusword != motor[1].Statusword || motor[0].Statusword != motor[2].Statusword || motor[0].Statusword != motor[3].Statusword || motor[1].Statusword != motor[2].Statusword \
	|| motor[1].Statusword != motor[3].Statusword || motor[2].Statusword != motor[3].Statusword)
  {
	//	STATUS_FLAG = true;	
	return 1;		// different state
  }
  else if((motor[0].Statusword >> 9) == 0x00)	return 2;		// SDO state
  else if((motor[0].Statusword >> 6) & 0x01)		return 3;		// PDO, DV state
  else if((motor[0].Statusword & 0x01) && ((motor[0].Statusword >> 2) & 0x01))	return 5;		// PDO, EN state
  else if(motor[0].Statusword & 0x01)				return 4;		// PDO, SD state
  else	return 6;
}
/* USER CODE END 1 */
