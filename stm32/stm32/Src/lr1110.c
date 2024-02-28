/**
  ******************************************************************************
  * @file           : lr1110.c
  * @brief          : Containing utility functions for LR1110
  ******************************************************************************
  */

#include "lr1110.h"

#include "spi.h"
#include "main.h"       // for HAL_DBG_TRACE-functions

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

void getLR1110_ChipEUI( const void* context ) {
  HAL_DBG_TRACE_INFO("Getting LR1110 ChipEUI... ");

  uint8_t cbuffer[LR1110_GET_CHIPEUI_CMD_LENGTH];
  uint8_t rbuffer[LR1110_GET_CHIPEUI_LENGTH] = { 0 };

  cbuffer[0] = ( uint8_t )( LR1110_GET_CHIPEUI_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_GET_CHIPEUI_CMD >> 0 );

  if (lr1110_spi_read( context, cbuffer, LR1110_GET_CHIPEUI_CMD_LENGTH, rbuffer, LR1110_GET_CHIPEUI_LENGTH ) == LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_INFO_VALUE("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", rbuffer[0], rbuffer[1], rbuffer[2], rbuffer[3], rbuffer[4], rbuffer[5], rbuffer[6], rbuffer[7]);
  } else {
    HAL_DBG_TRACE_ERROR("Failed to get LR1110 ChipEUI\r\n");
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
  cbuffer[2] = ( uint8_t ) 0x02;

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