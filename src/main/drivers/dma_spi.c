#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "dma_spi.h"

#ifdef USE_DMA_SPI_DEVICE

static void dmaSpiCsLo(void)
{
    gpio_write_pin(DMA_SPI_NSS_PORT, DMA_SPI_NSS_PIN, 0);
}

static void dmaSpiCsHi(void)
{
    gpio_write_pin(DMA_SPI_NSS_PORT, DMA_SPI_NSS_PIN, 1);
}

static void dmaSpicleanupspi(void)
{
    //clear DMA flags
    DMA_ClearFlag(DMA_SPI_ALL_DMA_FLAGS);

    //disable DMAs
    DMA_Cmd(DMA_SPI_TX_DMA,DISABLE);
    DMA_Cmd(DMA_SPI_RX_DMA,DISABLE);  

    //disable SPI DMA requests
    SPI_I2S_DMACmd(DMA_SPI_SPI, SPI_I2S_DMAReq_Tx, DISABLE);
    SPI_I2S_DMACmd(DMA_SPI_SPI, SPI_I2S_DMAReq_Rx, DISABLE);

    //disable SPI
    SPI_Cmd(DMA_SPI_SPI, DISABLE);
}

void DMA_SPI_RX_DMA_HANDLER(void)
{
    if(DMA_GetITStatus(GYRO_RX_DMA_FLAG_TC))
    {
        dmaSpiCsHi();
        dmaSpicleanupspi();
        //callback here, maybe do it with a flag?
        //gyroUpdateSensor(&gyroSensor1, currentTimeUs);
        DMA_ClearITPendingBit(GYRO_RX_DMA_FLAG_TC);         
    }
}

void dmaSpiInit(void)
{
    GPIO_InitTypeDef gpioInitStruct;
    SPI_InitTypeDef spiInitStruct;
    DMA_InitTypeDef dmaInitStruct;
    NVIC_InitTypeDef nvicInitStruct;

    //config pins
    gpioInitStruct.GPIO_Pin = DMA_SPI_NSS_PIN;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
    gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    gpioInitStruct.GPIO_OType = GPIO_OType_PP;
    gpioInitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(DMA_SPI_NSS_PORT, &gpioInitStruct);

    //set default CS state (high)
    GPIO_SetBits(DMA_SPI_NSS_PORT, DMA_SPI_NSS_PIN);

    gpioInitStruct.GPIO_Mode = GPIO_Mode_AF; 
    gpioInitStruct.GPIO_Pin = DMA_SPI_SCK_PIN;
    GPIO_Init(DMA_SPI_SCK_PORT, &gpioInitStruct);

    gpioInitStruct.GPIO_Pin = DMA_SPI_MISO_PIN;
    GPIO_Init(DMA_SPI_MISO_PORT, &gpioInitStruct);

    gpioInitStruct.GPIO_Pin = DMA_SPI_MOSI_PIN;
    GPIO_Init(DMA_SPI_MOSI_PORT, &gpioInitStruct);

    //set AF map
    GPIO_PinAFConfig(DMA_SPI_SCK_PORT,  DMA_SPI_SCK_PIN_SRC,  DMA_SPI_SCK_AF);
    GPIO_PinAFConfig(DMA_SPI_MISO_PORT, DMA_SPI_MISO_PIN_SRC, DMA_SPI_MISO_AF);
    GPIO_PinAFConfig(DMA_SPI_MOSI_PORT, DMA_SPI_MOSI_PIN_SRC, DMA_SPI_MOSI_AF);

    //config SPI
    SPI_StructInit(&spiInitStruct);
    spiInitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spiInitStruct.SPI_Mode = SPI_Mode_Master;
    spiInitStruct.SPI_DataSize = SPI_DataSize_8b;
    spiInitStruct.SPI_CPOL = DMA_SPI_CPOL;
    spiInitStruct.SPI_CPHA = DMA_SPI_CPHA;
    spiInitStruct.SPI_NSS = SPI_NSS_Soft;
    spiInitStruct.SPI_BaudRatePrescaler = DMA_SPI_BAUDRATE;
    spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(DMA_SPI_SPI, &spiInitStruct);

    //set DMA to default state
    DMA_DeInit(DMA_SPI_TX_DMA_STREAM);
    DMA_DeInit(DMA_SPI_RX_DMA_STREAM);

    DMA_StructInit(&dmaInitStruct);
    dmaInitStruct.DMA_Channel = DMA_SPI_TX_DMA_CHANNEL;
    dmaInitStruct.DMA_Mode = DMA_Mode_Normal;
    dmaInitStruct.DMA_Priority = DMA_Priority_Medium;
    dmaInitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;

    dmaInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dmaInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmaInitStruct.DMA_PeripheralBaseAddr = (uint32_t)&DMA_SPI_SPI->DR;

    dmaInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dmaInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmaInitStruct.DMA_Memory0BaseAddr = 0; //this is set later when we fire the DMA

    dmaInitStruct.DMA_BufferSize = 1;     //this is set later when we fire the DMA, can't be 0

    DMA_Init(DMA_SPI_TX_DMA_STREAM, &dmaInitStruct);

    dmaInitStruct.DMA_Channel = DMA_SPI_RX_DMA_CHANNEL;
    dmaInitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;

    DMA_Init(DMA_SPI_RX_DMA_STREAM, &dmaInitStruct);

    //setup interrupt
    nvicInitStruct.NVIC_IRQChannel = DMA_SPI_RX_DMA_IRQn;
    nvicInitStruct.NVIC_IRQChannelPreemptionPriority = DMA_SPI_DMA_RX_PRE_PRI;
    nvicInitStruct.NVIC_IRQChannelSubPriority = DMA_SPI_DMA_RX_SUB_PRI;
    nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInitStruct);
    DMA_ITConfig(DMA_SPI_RX_DMA_STREAM, DMA_IT_TC, ENABLE);
}

void dmaSpiTransmitReceive(uint8_t* txBuffer, uint8_t* rxBuffer, uint32_t size)
{
    //set buffer size
    DMA_SetCurrDataCounter(DMA_SPI_TX_DMA_STREAM, size);
    DMA_SetCurrDataCounter(DMA_SPI_RX_DMA_STREAM, size);

    //set buffer
    DMA_SPI_TX_DMA_STREAM->CMAR = (uint32_t)txBuffer;
    DMA_SPI_RX_DMA_STREAM->CMAR = (uint32_t)rxBuffer;

    //enable DMA SPI streams
    DMA_Cmd(DMA_SPI_TX_DMA_STREAM, ENABLE);
    DMA_Cmd(DMA_SPI_RX_DMA_STREAM, ENABLE);

    //enable  CS
    dmaSpiCsLo();

    //enable DMA SPI requests
    SPI_I2S_DMACmd(DMA_SPI_SPI, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(DMA_SPI_SPI, SPI_I2S_DMAReq_Rx, ENABLE);

    //enable and send
    SPI_Cmd(DMA_SPI_SPI, ENABLE);
}
#endif