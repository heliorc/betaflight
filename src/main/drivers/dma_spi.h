#pragma once

#ifdef USE_DMA_SPI_DEVICE
extern void dmaSpiInit(void);
extern void dmaSpiTransmitReceive(uint8_t* txBuffer, uint8_t* rxBuffer, uint32_t size);
#endif