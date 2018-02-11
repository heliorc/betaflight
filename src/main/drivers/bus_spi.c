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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <platform.h>

#ifdef USE_SPI

#include "drivers/bus.h"
#include "drivers/bus_spi.h"
#include "drivers/bus_spi_impl.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/rcc.h"

spiDevice_t spiDevice[SPIDEV_COUNT];

SPIDevice spiDeviceByInstance(SPI_TypeDef *instance)
{
#ifdef USE_SPI_DEVICE_1
    if (instance == SPI1)
        return SPIDEV_1;
#endif

#ifdef USE_SPI_DEVICE_2
    if (instance == SPI2)
        return SPIDEV_2;
#endif

#ifdef USE_SPI_DEVICE_3
    if (instance == SPI3)
        return SPIDEV_3;
#endif

#ifdef USE_SPI_DEVICE_4
    if (instance == SPI4)
        return SPIDEV_4;
#endif

    return SPIINVALID;
}

SPI_TypeDef *spiInstanceByDevice(SPIDevice device)
{
    if (device >= SPIDEV_COUNT) {
        return NULL;
    }

    return spiDevice[device].dev;
}

bool spiInit(SPIDevice device)
{
    switch (device) {
    case SPIINVALID:
        return false;
    case SPIDEV_1:
#if defined(USE_SPI_DMA_DEVICE_1)
        spiDmaInitDevice(device);
        return true;
#elif defined(USE_SPI_DEVICE_1)
        spiInitDevice(device);
        return true;
#else
        break;
#endif
    case SPIDEV_2:
#if defined(USE_SPI_DMA_DEVICE_2)
        spiDmaInitDevice(device);
        return true;
#elif defined(USE_SPI_DEVICE_2)
        spiInitDevice(device);
        return true;
#else
        break;
#endif
    case SPIDEV_3:
#if defined(USE_SPI_DMA_DEVICE_3) && !defined(STM32F1)
        spiDmaInitDevice(device);
        return true;
#elif defined(USE_SPI_DEVICE_3) && !defined(STM32F1)
        spiInitDevice(device);
        return true;
#else
        break;
#endif
    case SPIDEV_4:
#if defined(USE_SPI_DMA_DEVICE_4)
        spiDmaInitDevice(device);
        return true;
#elif defined(USE_SPI_DEVICE_4)
        spiInitDevice(device);
        return true;
#else
        break;
#endif
    }
    return false;
}

void SPI1_RX_DMA_HANDLER()
{
    if()
}

void SPI2_RX_DMA_HANDLER()
{

}

void SPI3_RX_DMA_HANDLER()
{

}

void spiDmaInit(const busDevice_t *bus)
{
    if(bus->busdev_u.spi.instance == SPI1)
    {

    }
    //bus->spiCallbackFunction = 
    typedef void (*spi_tx_done_callback)(void);

    //set fifo threashold for 8 bit i/o
    SPI_RxFIFOThresholdConfig(bus->busdev_u.spi.instance,SPI_RxFIFOThreshold_QF);

    //set DMA to default state
    DMA_DeInit(bus->busdev_u.spi.TxDmaChannel);
    DMA_DeInit(bus->busdev_u.spi.RxDmaChannel);

    dmaInitStruct.DMA_M2M = DMA_M2M_Disable;
    dmaInitStruct.DMA_Mode = DMA_Mode_Normal;
    dmaInitStruct.DMA_Priority = DMA_Priority_Medium;
    dmaInitStruct.DMA_DIR = DMA_DIR_PeripheralDST;

    dmaInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dmaInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmaInitStruct.DMA_PeripheralBaseAddr = (uint32_t)&bus->busdev_u.spi.instance->DR;

    dmaInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dmaInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmaInitStruct.DMA_MemoryBaseAddr = 0; //this is set later when we fire the DMA

    dmaInitStruct.DMA_BufferSize = 1;     //this is set later when we fire the DMA

    DMA_Init(bus->busdev_u.spi.TxDmaChannel, &dmaInitStruct);

    dmaInitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;

    DMA_Init(bus->busdev_u.spi.RxDmaChannel, &dmaInitStruct);

    //setup interrupt
    nvicInitStruct.NVIC_IRQChannel = GYRO_SPI_RX_DMA_IRQn;
    nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x0e;
    nvicInitStruct.NVIC_IRQChannelSubPriority = 0x0e;
    nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInitStruct);

    //set rx done irq
    DMA_ITConfig(bus->busdev_u.spi.RxDmaChannel, DMA_IT_TC, ENABLE);
}

void spiBusDmaTransferStart(const busDevice_t *bus, const uint8_t *txData, uint8_t *rxData, int length)
{

    //enable  CS
    IOLo(bus->busdev_u.spi.csnPin);

    //set buffer size
    DMA_SetCurrDataCounter(bus->busdev_u.spi.TxDmaChannel, size);
    DMA_SetCurrDataCounter(bus->busdev_u.spi.RxDmaChannel, size);

    //set buffer
    bus->busdev_u.spi.TxDmaChannel->CMAR = (uint32_t)txBuffer;
    bus->busdev_u.spi.RxDmaChannel->CMAR = (uint32_t)rxBuffer;

    //enable DMA SPI streams
    DMA_Cmd(bus->busdev_u.spi.TxDmaChannel, ENABLE);
    DMA_Cmd(bus->busdev_u.spi.RxDmaChannel, ENABLE);

    //enable DMA SPI requests
    SPI_I2S_DMACmd(bus->busdev_u.spi.instance, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(bus->busdev_u.spi.instance, SPI_I2S_DMAReq_Rx, ENABLE);

    //enable and send
    SPI_Cmd(bus->busdev_u.spi.instance, ENABLE);

}

void spiBusDmaTransferDone(void)
{
    //disable cs
    IOHi(bus->busdev_u.spi.csnPin);

    //clear DMA flags
    DMA_ClearFlag(GYRO_SPI_ALL_DMA_FLAGS);

    //disable DMAs
    DMA_Cmd(bus->busdev_u.spi.TxDmaChannel,DISABLE);
    DMA_Cmd(bus->busdev_u.spi.RxDmaChannel,DISABLE);  

    //disable SPI DMA requests
    SPI_I2S_DMACmd(bus->busdev_u.spi.instance, SPI_I2S_DMAReq_Tx, DISABLE);
    SPI_I2S_DMACmd(bus->busdev_u.spi.instance, SPI_I2S_DMAReq_Rx, DISABLE);

    //disable SPI
    SPI_Cmd(bus->busdev_u.spi.instance, DISABLE);

    //call read done calback
    bus->busdev_u.spi.spiCallbackFunction();
}

uint32_t spiTimeoutUserCallback(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return -1;
    }
    spiDevice[device].errorCount++;
    return spiDevice[device].errorCount;
}

bool spiBusTransfer(const busDevice_t *bus, const uint8_t *txData, uint8_t *rxData, int length)
{
    IOLo(bus->busdev_u.spi.csnPin);
    spiTransfer(bus->busdev_u.spi.instance, txData, rxData, length);
    IOHi(bus->busdev_u.spi.csnPin);
    return true;
}

uint16_t spiGetErrorCounter(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return 0;
    }
    return spiDevice[device].errorCount;
}

void spiResetErrorCounter(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device != SPIINVALID) {
        spiDevice[device].errorCount = 0;
    }
}

bool spiBusWriteRegister(const busDevice_t *bus, uint8_t reg, uint8_t data)
{
    IOLo(bus->busdev_u.spi.csnPin);
    spiTransferByte(bus->busdev_u.spi.instance, reg);
    spiTransferByte(bus->busdev_u.spi.instance, data);
    IOHi(bus->busdev_u.spi.csnPin);

    return true;
}

bool spiBusReadRegisterBuffer(const busDevice_t *bus, uint8_t reg, uint8_t *data, uint8_t length)
{
    IOLo(bus->busdev_u.spi.csnPin);
    spiTransferByte(bus->busdev_u.spi.instance, reg | 0x80); // read transaction
    spiTransfer(bus->busdev_u.spi.instance, NULL, data, length);
    IOHi(bus->busdev_u.spi.csnPin);

    return true;
}

uint8_t spiBusReadRegister(const busDevice_t *bus, uint8_t reg)
{
    uint8_t data;
    IOLo(bus->busdev_u.spi.csnPin);
    spiTransferByte(bus->busdev_u.spi.instance, reg | 0x80); // read transaction
    spiTransfer(bus->busdev_u.spi.instance, NULL, &data, 1);
    IOHi(bus->busdev_u.spi.csnPin);

    return data;
}

void spiBusSetInstance(busDevice_t *bus, SPI_TypeDef *instance)
{
    bus->bustype = BUSTYPE_SPI;
    bus->busdev_u.spi.instance = instance;
}
#endif
