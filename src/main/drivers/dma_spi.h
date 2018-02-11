#pragma once

#ifdef USE_DMA_SPI_DEVICE

typedef enum dma_spi_read_status_e {
    DMA_SPI_READ_UNKOWN = 0,
    DMA_SPI_READ_IN_PROGRESS = 1,
    DMA_SPI_READ_DONE = 2
} dma_spi_read_status_t;

extern volatile dma_spi_read_status_t dmaSpiReadStatus;

extern void dmaSpiInit(void);
extern void dmaSpiTransmitReceive(uint8_t* txBuffer, uint8_t* rxBuffer, uint32_t size);

#endif