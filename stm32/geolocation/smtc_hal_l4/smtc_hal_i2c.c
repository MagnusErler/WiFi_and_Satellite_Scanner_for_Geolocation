/*!
 * @file      smtc_hal_i2c.c
 *
 * @brief     Implements the i2c HAL functions
 *
 * Revised BSD License
 * Copyright Semtech Corporation 2020. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include "stm32l4xx_hal.h"
#include "smtc_hal_gpio_pin_names.h"
#include "smtc_hal_i2c.h"
#include "smtc_hal_mcu.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */
static hal_i2c_t hal_i2c[] = {
    [0] =
    {
        .interface = I2C1,
        .handle    = { NULL },
        .pins =
        {
            .sda = NC,
            .scl = NC,
        },
    },
    [2] =
    {
        .interface = I2C3,
        .handle    = { NULL },
        .pins =
        {
            .sda = NC,
            .scl = NC,
        },
    },
};

static i2c_addr_size i2c_internal_addr_size;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*!
 * @brief Write data buffer to the I2C device
 *
 * @param [in] id               I2C interface id [1:N]
 * @param [in] deviceAddr       device address
 * @param [in] addr             data address
 * @param [in] buffer           data buffer to write
 * @param [in] size             number of data bytes to write
 *
 * @returns status [SUCCESS, FAIL]
 */
static uint8_t i2c_write_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer,
                                 uint16_t size );

/*!
 * @brief Write data buffer to the I2C device
 *
 * @param [in] id               I2C interface id [1:N]
 * @param [in] deviceAddr       device address
 * @param [in] addr             data address
 * @param [in] buffer           data buffer to write
 * @param [in] size             number of data bytes to write
 *
 * @returns status [SUCCESS, FAIL]
 */
static uint8_t i2c_read_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void hal_i2c_init( const uint32_t id, const hal_gpio_pin_names_t sda, const hal_gpio_pin_names_t scl ) {
    assert_param( ( id > 0 ) && ( ( id - 1 ) < sizeof( hal_i2c ) ) );
    uint32_t local_id = id - 1;

    hal_i2c[local_id].handle.Instance              = hal_i2c[local_id].interface;
    hal_i2c[local_id].handle.Init.Timing           = 0x10909CEC;
    hal_i2c[local_id].handle.Init.OwnAddress1      = 0;
    hal_i2c[local_id].handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    hal_i2c[local_id].handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    hal_i2c[local_id].handle.Init.OwnAddress2      = 0;
    hal_i2c[local_id].handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hal_i2c[local_id].handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    hal_i2c[local_id].handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    hal_i2c[local_id].pins.sda = sda;
    hal_i2c[local_id].pins.scl = scl;

    if( HAL_I2C_Init( &hal_i2c[local_id].handle ) != HAL_OK ) {
        mcu_panic( );
    }

    /**Configure Analogue filter
     */
    if( HAL_I2CEx_ConfigAnalogFilter( &hal_i2c[local_id].handle, I2C_ANALOGFILTER_ENABLE ) != HAL_OK ) {
        mcu_panic( );
    }

    /**Configure Digital filter
     */
    if( HAL_I2CEx_ConfigDigitalFilter( &hal_i2c[local_id].handle, 0 ) != HAL_OK ) {
        mcu_panic( );
    }
}

void hal_i2c_deinit( const uint32_t id ) {
    assert_param( ( id > 0 ) && ( ( id - 1 ) < sizeof( hal_i2c ) ) );
    uint32_t local_id = id - 1;

    HAL_I2C_DeInit( &hal_i2c[local_id].handle );
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
    //   Error_Handler();
      mcu_panic( );
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }

}

void HAL_I2C_MspDeInit( I2C_HandleTypeDef* i2cHandle ) {
    uint32_t local_id = 0;

    if( i2cHandle->Instance == hal_i2c[0].interface ) {
        local_id = 0;
        __HAL_RCC_I2C1_FORCE_RESET( );
        __HAL_RCC_I2C1_RELEASE_RESET( );
        __HAL_RCC_I2C1_CLK_DISABLE( );
    } else if( i2cHandle->Instance == hal_i2c[2].interface ) {
        local_id = 2;
        __HAL_RCC_I2C3_FORCE_RESET( );
        __HAL_RCC_I2C3_RELEASE_RESET( );
        __HAL_RCC_I2C3_CLK_DISABLE( );
    } else {
        mcu_panic( );
    }

    HAL_GPIO_DeInit( ( GPIO_TypeDef* )( AHB2PERIPH_BASE + ( ( hal_i2c[local_id].pins.sda & 0xF0 ) << 6 ) ),
                     ( 1 << ( hal_i2c[local_id].pins.sda & 0x0F ) ) | ( 1 << ( hal_i2c[local_id].pins.scl & 0x0F ) ) );
}

uint8_t hal_i2c_write( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t data ) {
    return i2c_write_buffer( id, device_addr, addr, &data, 1u );
}

uint8_t hal_i2c_write_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size ) {
    return i2c_write_buffer( id, device_addr, addr, buffer, size );

}

uint8_t hal_i2c_read( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* data ) {
    return ( i2c_read_buffer( id, device_addr, addr, data, 1 ) );
}

uint8_t hal_i2c_read_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size ) {
    return ( i2c_read_buffer( id, device_addr, addr, buffer, size ) );
}

void hal_i2c_set_addr_size( i2c_addr_size addr_size ) { i2c_internal_addr_size = addr_size; }

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static uint8_t i2c_write_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size ) {
    uint16_t memAddSize   = 0u;

    assert_param( ( id > 0 ) && ( ( id - 1 ) < sizeof( hal_i2c ) ) );
    uint32_t local_id = id - 1;

    if( i2c_internal_addr_size == I2C_ADDR_SIZE_8 ) {
        memAddSize = I2C_MEMADD_SIZE_8BIT;
    } else {
        memAddSize = I2C_MEMADD_SIZE_16BIT;
    }

    return !HAL_I2C_Mem_Write( &hal_i2c[local_id].handle, device_addr, addr, memAddSize, buffer, size, 2000u );
}

static uint8_t i2c_read_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size ) {
    uint16_t memAddSize = 0u;

    assert_param( ( id > 0 ) && ( ( id - 1 ) < sizeof( hal_i2c ) ) );
    uint32_t local_id = id - 1;

    if( i2c_internal_addr_size == I2C_ADDR_SIZE_8 ) {
        memAddSize = I2C_MEMADD_SIZE_8BIT;
    } else {
        memAddSize = I2C_MEMADD_SIZE_16BIT;
    }

    return !HAL_I2C_Mem_Read( &hal_i2c[local_id].handle, device_addr, addr, memAddSize, buffer, size, 5000 );
}
