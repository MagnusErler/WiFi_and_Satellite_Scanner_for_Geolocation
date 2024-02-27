/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdlib.h> // used for malloc function
#include <string.h> // used for strlen function
#include <stdarg.h> // used for va_list, va_start, va_end functions
#include <stdio.h>  // used for vsprintf function

#include "led.h"

#include "spi.h"

#include "wifi.h"

#include "gnss.h"


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
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/*!
 * @brief Get LR1110 version
 *
 * @param [in] context Radio abstraction
 */
static void getLR1110_Bootloader_Version( const void* context );

/*!
 * @brief Get LR1110 temperature
 *
 * @param [in] context Radio abstraction
 * 
 * @return temperature in Celsius
 */
static float getLR1110_Temperature( const void* context );

/*!
 * @brief Get LR1110 DevEUI
 *
 * @param [in] context Radio abstraction
 */
static void getLR1110_DevEUI( const void* context );

/*!
 * @brief Get LR1110 Semtech JoinEui
 *
 * @param [in] context Radio abstraction
 * 
 */
static void getLR1110_Semtech_JoinEui( const void* context );

/*!
 * @brief Get LR1110 Battery Voltage
 *
 * @param [in] context Radio abstraction
 * 
 * @return battery voltage in Volts
 */
static float getLR1110_Battery_Voltage( const void* context );

/*!
 * @brief Setup LR1110 TCXO
 *
 * @param [in] context Radio abstraction
 */
static void setupLR1110_TCXO( const void* context );

/*!
 * @brief Reset LR1110
 *
 * @param [in] context Radio abstraction
 */
static void resetLR1110( const void* context );

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  void* lr1110_context = (void*) malloc(sizeof(radio_t));
  ((radio_t*)lr1110_context)->spi         = SPI1;
  ((radio_t*)lr1110_context)->nss.port    = NSS_GPIO_Port;
  ((radio_t*)lr1110_context)->nss.pin     = NSS_Pin;
  ((radio_t*)lr1110_context)->reset.port  = RESET_GPIO_Port;
  ((radio_t*)lr1110_context)->reset.pin   = RESET_Pin;
  ((radio_t*)lr1110_context)->busy.port   = BUSY_GPIO_Port;
  ((radio_t*)lr1110_context)->busy.pin    = BUSY_Pin;

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  HAL_DBG_TRACE_MSG("\r\n\r\n-----------------------------------------------------\r\n\r\n");
  resetLR1110(lr1110_context);

  blinkLED(GPIOC, RX_LED_Pin|TX_LED_Pin, 100, 5, true);

  setupLR1110_TCXO(lr1110_context);    // Seems like the first time LR1110 is called it returns with an error, so we call it twice
  setupLR1110_TCXO(lr1110_context);

  getLR1110_Bootloader_Version(lr1110_context);
  getLR1110_WiFi_Version(lr1110_context);
  getLR1110_DevEUI(lr1110_context);
  getLR1110_Semtech_JoinEui(lr1110_context);
  getLR1110_Temperature(lr1110_context);
  getLR1110_Battery_Voltage(lr1110_context);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {

    scanLR1110_WiFi_Networks(lr1110_context, 0x04, 0x3FFF, 0x04, 32, 3, 500, true);
    //scanLR1110_WiFi_Country_Code(lr1110_context, 0x3FFF, 32, 3, 500, true);
    uint8_t numberOfResults = getLR1110_WiFi_Number_of_Results(lr1110_context);
    //getLR1110_WiFi_Results(lr1110_context, 0, 6, 4);

    for( int i = 0; i < numberOfResults; i++ ) {
      getWiFiFullResults( lr1110_context, i, 1 );
    }


    getLR1110_GNSS_GET_CONSUMPTION(lr1110_context);


    // scanLR1110_GNSS_Satellites(lr1110_context, 0, 0, 0);
    // uint8_t numberOfDetectedSatellites = getLR1110_GNSS_Number_of_Detected_Satellites(lr1110_context);
    // if (numberOfDetectedSatellites > 0) {
    //   getLR1110_GNSS_Detected_Satellites(lr1110_context, numberOfDetectedSatellites);
    // }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_DBG_TRACE_MSG("Waiting for next while loop...\r\n")
    HAL_Delay(5000);
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
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
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  __HAL_SPI_ENABLE( &hspi1 );

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RX_LED_Pin|TX_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RESET_Pin|NSS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LNA_CTRL_Pin|SNIFFING_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RX_LED_Pin TX_LED_Pin */
  GPIO_InitStruct.Pin = RX_LED_Pin|TX_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RESET_Pin NSS_Pin */
  GPIO_InitStruct.Pin = RESET_Pin|NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LNA_CTRL_Pin SNIFFING_LED_Pin */
  GPIO_InitStruct.Pin = LNA_CTRL_Pin|SNIFFING_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BUSY_Pin */
  GPIO_InitStruct.Pin = BUSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUSY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EVENT_Pin */
  GPIO_InitStruct.Pin = EVENT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EVENT_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

void HAL_DBG_TRACE_PRINT( const char* fmt, ... ) {
  va_list argp;
  va_start( argp, fmt );

  char string[255];
  if( 0 < vsprintf( string, fmt, argp ) ) {
      if (HAL_UART_Transmit(&huart2, (uint8_t*) string, (uint16_t) strlen((const char*) string), 1000) != HAL_OK) {
          Error_Handler();
      }
  }

  va_end( argp );
}

void getLR1110_Bootloader_Version( const void* context ) {
  HAL_DBG_TRACE_INFO("Getting LR1110 version... ");

  uint8_t cbuffer[LR1110_GET_VERSION_CMD_LENGTH];
  uint8_t rbuffer[LR1110_GET_VERSION_LENGTH] = { 0 };

  cbuffer[0] = ( uint8_t )( LR1110_GET_VERSION_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_GET_VERSION_CMD >> 0 );
  
  if (lr1110_spi_read(context, cbuffer, LR1110_GET_VERSION_CMD_LENGTH, rbuffer, LR1110_GET_VERSION_LENGTH ) == LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_INFO_VALUE("HW: %d (0x%X), ", rbuffer[0], rbuffer[0]);
    HAL_DBG_TRACE_INFO_VALUE("FW: %d.%d (0x%X.0x%X), ", rbuffer[2], rbuffer[3], rbuffer[2], rbuffer[3]);
    switch (rbuffer[1]) {
        case 1:
            HAL_DBG_TRACE_INFO_VALUE("Bootloader type: LR1110 (0x%X)\r\n", rbuffer[1]);
            break;
        case 2:
            HAL_DBG_TRACE_INFO_VALUE("Bootloader type: LR1120 (0x%X)\r\n", rbuffer[1]);
            break;
        case 3:
            HAL_DBG_TRACE_INFO_VALUE("Bootloader type: LR1121 (0x%X)\r\n", rbuffer[1]);
            break;
    }
  } else {
    HAL_DBG_TRACE_ERROR("Failed to get LR1110 bootloader version\r\n");
  }
}

float getLR1110_Temperature( const void* context ) {
  HAL_DBG_TRACE_INFO("Getting LR1110 temperature... ");

  uint8_t cbuffer[LR1110_GET_TEMPERATURE_CMD_LENGTH];
  uint8_t rbuffer[LR1110_GET_TEMPERATURE_LENGTH] = { 0 };

  cbuffer[0] = ( uint8_t )( LR1110_GET_TEMPERATURE_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_GET_TEMPERATURE_CMD >> 0 );

  if (lr1110_spi_read( context, cbuffer, LR1110_GET_TEMPERATURE_CMD_LENGTH, rbuffer, LR1110_GET_TEMPERATURE_LENGTH ) == LR1110_SPI_STATUS_OK) {

    uint16_t temp_10_0 = ((rbuffer[0] << 8) | rbuffer[1]) & 0x7FF;
    float temperature = 25 + (1000/(-1.7)) * ((temp_10_0/2047.0) * 1.35 - 0.7295);
    
    HAL_DBG_TRACE_INFO_VALUE("%d.%d °C\r\n", (uint8_t)temperature, (uint8_t)((temperature - (uint8_t)temperature) * 100));
    if ((uint8_t)temperature > 50) {
      HAL_DBG_TRACE_ERROR("LR1110 temperature is too high. TCXO mode is maybe not set up correctly\r\n");
    }
    return temperature;
  } else {
    HAL_DBG_TRACE_ERROR("Failed to get LR1110 temperature\r\n");
    return -1;
  }
}

void getLR1110_DevEUI( const void* context ) {
  HAL_DBG_TRACE_INFO("Getting LR1110 DevEUI... ");

  uint8_t cbuffer[LR1110_GET_DEVEUI_CMD_LENGTH];
  uint8_t rbuffer[LR1110_GET_DEVEUI_LENGTH] = { 0 };

  cbuffer[0] = ( uint8_t )( LR1110_GET_DEVEUI_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_GET_DEVEUI_CMD >> 0 );

  if (lr1110_spi_read( context, cbuffer, LR1110_GET_DEVEUI_CMD_LENGTH, rbuffer, LR1110_GET_DEVEUI_LENGTH ) == LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_INFO_VALUE("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", rbuffer[0], rbuffer[1], rbuffer[2], rbuffer[3], rbuffer[4], rbuffer[5], rbuffer[6], rbuffer[7]);
  } else {
    HAL_DBG_TRACE_ERROR("Failed to get LR1110 DevEUI\r\n");
  }
}

void getLR1110_Semtech_JoinEui( const void* context ) {
  HAL_DBG_TRACE_INFO("Getting LR1110 Semtech JoinEUI... ");

  uint8_t cbuffer[LR1110_GET_SEMTECH_JOINEUI_CMD_LENGTH];
  uint8_t rbuffer[LR1110_GET_SEMTECH_JOINEUI_LENGTH] = { 0 };

  cbuffer[0] = ( uint8_t )( LR1110_GET_SEMTECH_JOINEUI_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_GET_SEMTECH_JOINEUI_CMD >> 0 );

  if (lr1110_spi_read( context, cbuffer, LR1110_GET_SEMTECH_JOINEUI_CMD_LENGTH, rbuffer, LR1110_GET_SEMTECH_JOINEUI_LENGTH ) == LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_INFO_VALUE("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", rbuffer[0], rbuffer[1], rbuffer[2], rbuffer[3], rbuffer[4], rbuffer[5], rbuffer[6], rbuffer[7]);
  } else {
    HAL_DBG_TRACE_ERROR("Failed to get LR1110 Semtech JoinEUI\r\n");
  }
}

float getLR1110_Battery_Voltage( const void* context ) {
  HAL_DBG_TRACE_INFO("Getting LR1110 battery voltage... ");

  uint8_t cbuffer[LR1110_GET_BATTERY_VOLTAGE_CMD_LENGTH];
  uint8_t rbuffer[LR1110_GET_BATTERY_VOLTAGE_LENGTH] = { 0 };

  cbuffer[0] = ( uint8_t )( LR1110_GET_BATTERY_VOLTAGE_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_GET_BATTERY_VOLTAGE_CMD >> 0 );

  if (lr1110_spi_read( context, cbuffer, LR1110_GET_BATTERY_VOLTAGE_CMD_LENGTH, rbuffer, LR1110_GET_BATTERY_VOLTAGE_LENGTH ) == LR1110_SPI_STATUS_OK) {

    float batteryVoltage = (((5 * rbuffer[0])/255.0) - 1) * 1.35;
    HAL_DBG_TRACE_INFO_VALUE("%d.%d V\r\n", (uint8_t)batteryVoltage, (uint8_t)((batteryVoltage - (uint8_t)batteryVoltage) * 100));
    return batteryVoltage;
  } else {
    HAL_DBG_TRACE_ERROR("Failed to get LR1110 battery voltage\r\n");
    return -1;
  }
}

void setupLR1110_TCXO( const void* context ) {
  HAL_DBG_TRACE_INFO( "Setting up LR1110 TCXO mode... " );

  uint8_t cbuffer[LR1110_SET_TCXO_MODE_CMD_LENGTH];

  cbuffer[0] = ( uint8_t )( LR1110_SET_TCXO_MODE_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_SET_TCXO_MODE_CMD >> 0 );
  cbuffer[2] = ( uint8_t ) LR1110_TCXO_CTRL_1_8V;

  const uint8_t timeout = ( 5 * 1000 ) / 30.52;  // BOARD_TCXO_WAKEUP_TIME = 5               // 163

  cbuffer[3] = ( uint8_t )( timeout >> 16 );
  cbuffer[4] = ( uint8_t )( timeout >> 8 );
  cbuffer[5] = ( uint8_t )( timeout >> 0 );

  if (lr1110_spi_write( context, cbuffer, LR1110_SET_TCXO_MODE_CMD_LENGTH ) == LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_MSG_COLOR("DONE\r\n", HAL_DBG_TRACE_COLOR_GREEN);
  } else {
    HAL_DBG_TRACE_ERROR("Failed to set LR1110 TCXO mode\r\n");
  }
}

void resetLR1110( const void* context ) {

  radio_t* radio = (radio_t*) context;

  HAL_DBG_TRACE_INFO("Resetting LR1110... ");
  HAL_GPIO_WritePin(radio->reset.port, radio->reset.pin, GPIO_PIN_RESET);
  HAL_Delay(200);   // At least 100ms
  HAL_GPIO_WritePin(radio->reset.port, radio->reset.pin, GPIO_PIN_SET);
  HAL_DBG_TRACE_MSG_COLOR("DONE\r\n", HAL_DBG_TRACE_COLOR_GREEN);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */

  HAL_DBG_TRACE_ERROR( "%s\n", __func__ );
  HAL_DBG_TRACE_ERROR( "PANIC" );

  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();

  /* Restart system */
  NVIC_SystemReset( );

  while (1) {  }
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
