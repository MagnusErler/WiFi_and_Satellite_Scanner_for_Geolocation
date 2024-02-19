/**
  ******************************************************************************
  * @file           : spi.c
  * @brief          : Containing all SPI code
  ******************************************************************************
  */

#include "spi.h"

#include "led.h"

#include "main.h"       // for HAL_DBG_TRACE-functions

// Comment out the following line to disable debug messages
//#define DEBUG

radio_t* radio;

lr1110_spi_status_t _waitForBusyState( GPIO_PinState state, uint32_t timeout_ms ) {

    uint32_t start = HAL_GetTick();

    if( state == GPIO_PIN_RESET ) {
        while( HAL_GPIO_ReadPin( radio->busy.port, radio->busy.pin ) == GPIO_PIN_SET ) {
            if( ( uint32_t )( HAL_GetTick() - start ) > ( uint32_t ) timeout_ms ) {
                HAL_DBG_TRACE_PRINTF("\r\n");
                HAL_DBG_TRACE_ERROR("Timeout occured while waiting for BUSY to become LOW\r\n");
                return LR1110_SPI_STATUS_TIMEOUT;
            }
        };
    } else {
        while( HAL_GPIO_ReadPin( radio->busy.port, radio->busy.pin ) == GPIO_PIN_RESET ) {
            if( ( uint32_t )( HAL_GetTick() - start ) > ( uint32_t ) timeout_ms ) {
                HAL_DBG_TRACE_PRINTF("\r\n");
                HAL_DBG_TRACE_ERROR("Timeout occured while waiting for BUSY to become HIGH\r\n");
                return LR1110_SPI_STATUS_TIMEOUT;
            }
        };
    }
    return LR1110_SPI_STATUS_OK;
}

lr1110_spi_status_t _lr1110_spi_write( SPI_TypeDef* spi, const uint8_t* cbuffer, uint16_t cbuffer_length, uint32_t timeout_ms ) {

    uint8_t rbuffer[cbuffer_length];

    for( uint16_t i = 0; i < cbuffer_length; i++ ) {

        uint32_t start = HAL_GetTick();

        turnOnLED(TX_LED_GPIO_Port, TX_LED_Pin);
        while( LL_SPI_IsActiveFlag_TXE( spi ) == 0 ) {
            if( ( uint32_t )( HAL_GetTick() - start ) > ( uint32_t ) timeout_ms ) {
                HAL_DBG_TRACE_PRINTF("\r\n");
                HAL_DBG_TRACE_ERROR("_lr1110_spi_write(): Timeout occured while waiting for SPI to become ready for transmission\r\n");
                return LR1110_SPI_STATUS_TIMEOUT;
            }
        };
        LL_SPI_TransmitData8( spi, cbuffer[i] );
        turnOffLED(TX_LED_GPIO_Port, TX_LED_Pin);

        turnOnLED(RX_LED_GPIO_Port, RX_LED_Pin);
        while( LL_SPI_IsActiveFlag_RXNE( spi ) == 0 ) {
            if( ( uint32_t )( HAL_GetTick() - start ) > ( uint32_t ) timeout_ms ) {
                HAL_DBG_TRACE_PRINTF("\r\n");
                HAL_DBG_TRACE_ERROR("_lr1110_spi_write(): Timeout occured while waiting for SPI to become ready for reception\r\n");
                return LR1110_SPI_STATUS_TIMEOUT;
            }
        };
        rbuffer[i] = LL_SPI_ReceiveData8( spi );
        turnOffLED(RX_LED_GPIO_Port, RX_LED_Pin);
    }

    #ifdef DEBUG
    HAL_DBG_TRACE_PRINTF("\r\n---Stat1 Results---\r\n");
    switch ( rbuffer[0] & 0x01 ) {
        case 0:
            HAL_DBG_TRACE_PRINTF("\r\nNo interrupt active\r\n");
            break;
        case 1:
            HAL_DBG_TRACE_PRINTF("\r\nAt least 1 interrupt active\r\n");
            break;
    }
    #endif

    switch( ( rbuffer[0] & 0x0E ) >> 1 ) {
        case 0:
            HAL_DBG_TRACE_ERROR("CMD_FAIL: The last command could not be executed\r\n");
            break;
        case 1:
            HAL_DBG_TRACE_WARNING("CMD_PERR: The last command could not be processed (wrong opcode, arguments). It is possible to generate an interrupt on DIO if a command error occurred\r\n");
            break;
        #ifdef DEBUG
        case 2:
            HAL_DBG_TRACE_PRINTF("\r\nCMD_OK: The last command was processed successfully\r\n");
            break;
        case 3:
            HAL_DBG_TRACE_PRINTF("\r\nCMD_DAT: The last command was successfully processed, and data is currently transmitted instead of IRQ status\r\n");
            break;
        #endif
    }

    #ifdef DEBUG
    HAL_DBG_TRACE_PRINTF("\r\n---Stat2 Results---");
    switch( ( rbuffer[1] & 0x0F ) >> 4 ) {
        case 0:
            HAL_DBG_TRACE_PRINTF("\r\nBootloader: currently executes from boot-loader\r\n");
            break;
        case 1:
            HAL_DBG_TRACE_PRINTF("\r\nBootloader: currently executes from flash. The ResetStatus field is cleared on the first GetStatus() command after a reset. It is not cleared by any other command\r\n");
            break;
    }
    #endif

    #ifdef DEBUG
    switch( ( rbuffer[1] & 0x0E ) >> 1 ) {
        case 0:
            HAL_DBG_TRACE_PRINTF("\r\nChip mode: Sleep\r\n");
            break;
        case 1:
            HAL_DBG_TRACE_PRINTF("\r\nChip mode: Standby with RC Oscillator\r\n");
            break;
        case 2:
            HAL_DBG_TRACE_PRINTF("\r\nChip mode: Standby with external Oscillator\r\n");
            break;
        case 3:
            HAL_DBG_TRACE_PRINTF("\r\nChip mode: FS\r\n");
            break;
        case 4:
            HAL_DBG_TRACE_PRINTF("\r\nChip mode: RX\r\n");
            break;
        case 5:
            HAL_DBG_TRACE_PRINTF("\r\nChip mode: TX\r\n");
            break;
        case 6:
            HAL_DBG_TRACE_PRINTF("\r\nChip mode: Wi-Fi or GNSS geolocation\r\n");
            break;
    }
    #endif

    #ifdef DEBUG
    switch( rbuffer[1] & 0x0F ) {
        case 0:
            HAL_DBG_TRACE_PRINTF("\r\nReset status: Cleared (no active reset)\r\n");
            break;
        case 1:
            HAL_DBG_TRACE_PRINTF("\r\nReset status: Analog reset (Power On Reset, Brown-Out Reset)\r\n");
            break;
        case 2:
            HAL_DBG_TRACE_PRINTF("\r\neset status: External reset (NRESET pin)\r\n");
            break;
        case 3:
            HAL_DBG_TRACE_PRINTF("\r\nReset status: System reset\r\n");
            break;
        case 4:
            HAL_DBG_TRACE_PRINTF("\r\nReset status: Watchdog reset\r\n");
            break;
        case 5:
            HAL_DBG_TRACE_PRINTF("\r\nReset status: Wakeup NSS toggling\r\n");
            break;
        case 6:
            HAL_DBG_TRACE_PRINTF("\r\nReset status: RTC restart\r\n");
            break;
    }
    #endif

    #ifdef DEBUG
    HAL_DBG_TRACE_PRINTF("IRQStat(31:24): 0x%X\r\n", rbuffer[2]);
    HAL_DBG_TRACE_PRINTF("IRQStat(23:16): 0x%X\r\n", rbuffer[3]);
    HAL_DBG_TRACE_PRINTF("IRQStat(15:8): 0x%X\r\n", rbuffer[4]);
    HAL_DBG_TRACE_PRINTF("IRQStat(7:0): 0x%X\r\n", rbuffer[5]);
    #endif

    return LR1110_SPI_STATUS_OK;
}

lr1110_spi_status_t _lr1110_spi_read_with_dummy_byte( SPI_TypeDef* spi, uint8_t* rbuffer, uint16_t rbuffer_length, uint32_t timeout_ms ) {

    for( uint16_t i = 0; i < rbuffer_length; i++ ) {

        uint32_t start = HAL_GetTick( );

        turnOnLED(TX_LED_GPIO_Port, TX_LED_Pin);
        while( LL_SPI_IsActiveFlag_TXE( spi ) == 0 ) {
            if( ( uint32_t )( HAL_GetTick() - start ) > ( uint32_t ) timeout_ms ) {
                HAL_DBG_TRACE_PRINTF("\r\n");
                HAL_DBG_TRACE_ERROR("_lr1110_spi_read_with_dummy_byte(): Timeout occured while waiting for SPI to become ready for transmission\r\n");
                return LR1110_SPI_STATUS_TIMEOUT;
            }
        };
        LL_SPI_TransmitData8( spi, 0x00 );
        turnOffLED(TX_LED_GPIO_Port, TX_LED_Pin);

        turnOnLED(RX_LED_GPIO_Port, RX_LED_Pin);
        while( LL_SPI_IsActiveFlag_RXNE( spi ) == 0 ) {
            if( ( uint32_t )( HAL_GetTick() - start ) > ( uint32_t ) timeout_ms ) {
                HAL_DBG_TRACE_PRINTF("\r\n");
                HAL_DBG_TRACE_ERROR("_lr1110_spi_read_with_dummy_byte(): Timeout occured while waiting for SPI to become ready for reception\r\n");
                return LR1110_SPI_STATUS_TIMEOUT;
            }
        };
        rbuffer[i] = LL_SPI_ReceiveData8( spi );
        turnOffLED(RX_LED_GPIO_Port, RX_LED_Pin);
    }

    return LR1110_SPI_STATUS_OK;
}

lr1110_spi_status_t lr1110_spi_read( const void* context, const uint8_t* cbuffer, const uint16_t cbuffer_length, uint8_t* rbuffer, const uint16_t rbuffer_length ) {

    radio = (radio_t*) context;
    uint8_t dummy_byte = 0x00;

    // Wait for BUSY to become LOW -> LR1110 is ready for a new command
    if (_waitForBusyState( GPIO_PIN_RESET, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }

    // Start of 1st SPI transaction
    HAL_GPIO_WritePin( radio->nss.port, radio->nss.pin, GPIO_PIN_RESET );
    if (_lr1110_spi_write( radio->spi, cbuffer, cbuffer_length, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }
    HAL_GPIO_WritePin( radio->nss.port, radio->nss.pin, GPIO_PIN_SET );
    // End of 1st SPI transaction

    // Wait for BUSY to become LOW -> LR1110 is ready for a new command
    if (_waitForBusyState( GPIO_PIN_RESET, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }

    // Start of 2nd SPI transaction
    HAL_GPIO_WritePin( radio->nss.port, radio->nss.pin, GPIO_PIN_RESET );
    if (_lr1110_spi_write( radio->spi, &dummy_byte, 1, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }
    if (_lr1110_spi_read_with_dummy_byte( radio->spi, rbuffer, rbuffer_length, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }
    HAL_GPIO_WritePin( radio->nss.port, radio->nss.pin, GPIO_PIN_SET );
    // End of 2nd SPI transaction

    // Wait for BUSY to become LOW -> LR1110 is ready for a new command
    if (_waitForBusyState( GPIO_PIN_RESET, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }

    return LR1110_SPI_STATUS_OK;
}

lr1110_spi_status_t lr1110_spi_write( const void* context, const uint8_t* cbuffer, uint16_t cbuffer_length ) {

    lr1110_spi_status_t status = LR1110_SPI_STATUS_OK;

    radio = (radio_t*) context;

    // Wait for BUSY to become LOW -> LR1110 is ready for a new command
    if (_waitForBusyState( GPIO_PIN_RESET, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }

    // Start of SPI transaction
    HAL_GPIO_WritePin( radio->nss.port, radio->nss.pin, GPIO_PIN_RESET );
    if (_lr1110_spi_write( radio->spi, cbuffer, cbuffer_length, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }
    HAL_GPIO_WritePin( radio->nss.port, radio->nss.pin, GPIO_PIN_SET );
    // End of SPI transaction

    // Wait for BUSY to become LOW -> LR1110 is ready for a new command
    if (_waitForBusyState( GPIO_PIN_RESET, 1000 ) != LR1110_SPI_STATUS_OK) {
        return LR1110_SPI_STATUS_ERROR;
    }
    
    return status;
}
