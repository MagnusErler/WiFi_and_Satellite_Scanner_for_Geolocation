
#include "gnss.h"

#include "spi.h"

#include "main.h"       // for HAL_DBG_TRACE-functions


lr1110_spi_status_t lr11xx_gnss_get_nb_detected_satellites( const void* context, uint8_t* nb_detected_satellites ) {
  HAL_DBG_TRACE_INFO("Getting number of detected satellites... ");

  uint8_t cbuffer[LR1110_GET_NUMBER_OF_SATELLITES_CMD_LENGTH];
  uint8_t rbuffer[LR1110_GET_NUMBER_OF_SATELLITES_LENGTH] = { 0 };

  cbuffer[0] = ( uint8_t )( LR1110_GET_NUMBER_OF_SATELLITES_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_GET_NUMBER_OF_SATELLITES_CMD >> 0 );

  if (lr1110_spi_read( context, cbuffer, LR1110_GET_NUMBER_OF_SATELLITES_CMD_LENGTH, rbuffer, LR1110_GET_NUMBER_OF_SATELLITES_LENGTH ) == LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_INFO_VALUE("%d\r\n", (uint8_t)rbuffer[0]);
    return LR1110_SPI_STATUS_OK;
  } else {
    HAL_DBG_TRACE_ERROR("Failed to get GNSS satellites\r\n");
    return LR1110_SPI_STATUS_ERROR;
  }
}

lr1110_spi_status_t lr11xx_gnss_scan_autonomous( const void* context, const lr11xx_gnss_date_t date, const lr11xx_gnss_search_mode_t effort_mode,
                                             const uint8_t gnss_input_parameters, const uint8_t nb_sat ) {
  HAL_DBG_TRACE_INFO("Scanning GNSS satellites... ");

  uint8_t cbuffer[LR1110_SCAN_GNSS_AUTONOMOUS_CMD_LENGTH];

  cbuffer[0] = ( uint8_t )( LR1110_SCAN_GNSS_AUTONOMOUS_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_SCAN_GNSS_AUTONOMOUS_CMD >> 0 );
  cbuffer[2] = ( uint8_t )( date >> 24 );
  cbuffer[3] = ( uint8_t )( date >> 16 );
  cbuffer[4] = ( uint8_t )( date >> 8 );
  cbuffer[5] = ( uint8_t )( date >> 0 );
  cbuffer[6] = ( uint8_t ) effort_mode;
  cbuffer[7] = ( uint8_t ) gnss_input_parameters;
  cbuffer[8] = ( uint8_t ) nb_sat;

  if (lr1110_spi_write( context, cbuffer, LR1110_SCAN_GNSS_AUTONOMOUS_CMD_LENGTH ) == LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_MSG_COLOR("DONE\r\n", HAL_DBG_TRACE_COLOR_GREEN);
    return LR1110_SPI_STATUS_OK;
  } else {
    HAL_DBG_TRACE_ERROR("Failed to scan GNSS satellites\r\n");
    return LR1110_SPI_STATUS_ERROR;
  }
}

void scanLR1110_GNSS_Satellites( const void* context ) {
  
  if (lr11xx_gnss_scan_autonomous( context, 1708458239, LR11XX_GNSS_OPTION_DEFAULT, 0, 1 ) != LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_ERROR("Failed to scan GNSS satellites\r\n");
    return;
  }

  HAL_Delay( 5000 );

  uint8_t nb_sat;
  if (lr11xx_gnss_get_nb_detected_satellites( context, &nb_sat ) != LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_ERROR("Failed to get GNSS satellites\r\n");
    return;
  }
}

void getLR1110_GNSS_Version( const void* context ) {
  HAL_DBG_TRACE_INFO("Getting GNSS version... ");

  uint8_t cbuffer[LR1110_GET_GNSS_VERSION_CMD_LENGTH];
  uint8_t rbuffer[LR1110_GET_GNSS_VERSION_LENGTH] = { 0 };

  cbuffer[0] = ( uint8_t )( LR1110_GET_GNSS_VERSION_CMD >> 8 );
  cbuffer[1] = ( uint8_t )( LR1110_GET_GNSS_VERSION_CMD >> 0 );

  if (lr1110_spi_read( context, cbuffer, LR1110_GET_GNSS_VERSION_CMD_LENGTH, rbuffer, LR1110_GET_GNSS_VERSION_LENGTH ) == LR1110_SPI_STATUS_OK) {
    HAL_DBG_TRACE_INFO_VALUE("firmware version: %d, ", rbuffer[0]);
    HAL_DBG_TRACE_INFO_VALUE("almanac version: %d\n\r", rbuffer[1]);
  } else {
    HAL_DBG_TRACE_ERROR("Failed to get GNSS version\r\n");
  }
}