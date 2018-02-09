#pragma once

extern uint32_t skipGyro;

void AccGyroDeinit(void);
// all gyroscopes should define this function which initializes all requesite
// hardware resources
uint32_t AccGyroInit(loopCtrl_e gyroLoop);

// functions used to read and write to hardware

extern void GyroExtiCallback(uint32_t callbackNumber);
extern void GyroRxDmaCallback(uint32_t callbackNumber);

extern uint32_t AccGyroReadWriteData(uint8_t *txData, uint8_t *rxData, uint8_t length, int useDma);