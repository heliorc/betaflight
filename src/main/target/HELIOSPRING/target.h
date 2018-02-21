/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once


#define TARGET_BOARD_IDENTIFIER "HESP"
#define USBD_PRODUCT_STRING     "HELIOSPRING"

#ifdef OPBL
#define USBD_SERIALNUMBER_STRING "0x8020000"
#endif

#define LED0_PIN                PA8

#define BEEPER                  PC15

#define USE_DSHOT
#define USE_DSHOT_DMAR

#define INVERTER_PIN_UART1      PA9

#define INVERTER_PIN_UART2      PA3

#define USE_GYRO
#define USE_ACC



// #define USE_GYRO_MPU6500
// #define USE_GYRO_SPI_MPU6500
// #define GYRO_MPU6500_ALIGN      CW0_DEG

// #define USE_ACC_MPU6500
// #define USE_ACC_SPI_MPU6500
// #define ACC_MPU6500_ALIGN       CW0_DEG


// MPU6000 interrupts
#define USE_FAKE_GYRO
// #define USE_IMUF


#define USE_FAST_SPI_DRIVER
#define USE_GYRO_IMUF9001
#define IMUF9001_CS_PIN         PB1
#define IMUF9001_RST_PIN        PA4 
#define IMUF9001_SPI_INSTANCE   SPI1
#define USE_EXTI
#define MPU_INT_EXTI            PB0
#define USE_MPU_DATA_READY_SIGNAL


#define M25P16_CS_PIN           PB3
#define M25P16_SPI_INSTANCE     SPI3
#define USE_FLASHFS
#define USE_FLASH_M25P16

#define USE_VCP

#define VBUS_SENSING_PIN        PC5

#define USE_OSD
#define USE_MAX7456
#define MAX7456_SPI_INSTANCE    SPI3
#define MAX7456_SPI_CS_PIN      PA15
#define MAX7456_SPI_CLK         (SPI_CLOCK_STANDARD) // 10MHz
#define MAX7456_RESTORE_CLK     (SPI_CLOCK_FAST)

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define USE_UART3
#define UART3_RX_PIN            PB11
#define UART3_TX_PIN            PB10

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART5
#define UART5_RX_PIN            PC12
#define UART5_TX_PIN            PD2

#define USE_SOFTSERIAL1
#define USE_SOFTSERIAL2

#define SERIAL_PORT_COUNT       8 //VCP, USART1, USART2, USART3, UART4, USART5, SOFTSERIAL x 2

#define USE_ESCSERIAL
#define ESCSERIAL_TIMER_TX_PIN  PC8  // (HARDARE=0,PPM)

#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PB3
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PA15
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

#undef USE_I2C

#define USE_ADC
#define CURRENT_METER_ADC_PIN   PA1
#define VBAT_ADC_PIN            PA0

#define RSSI_ADC_PIN            PA4

#define USE_TARGET_CONFIG
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
// #define USE_TRANSPONDER

#define DEFAULT_RX_FEATURE      FEATURE_RX_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART2
#define DEFAULT_FEATURES        (FEATURE_SOFTSERIAL | FEATURE_TELEMETRY | FEATURE_OSD | FEATURE_AIRMODE | FEATURE_LED_STRIP | FEATURE_ANTI_GRAVITY)

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define USABLE_TIMER_CHANNEL_COUNT 7
#define USED_TIMERS             ( TIM_N(1) | TIM_N(3) | TIM_N(4) | TIM_N(12) )
