#pragma once

typedef enum gyroToBoardCommMode
{
    GTBCM_SETUP                  = 52, //max number
    GTBCM_GYRO_ONLY_PASSTHRU     = 6,  //no crc, gyro, 3*2 bytes
    GTBCM_GYRO_ACC_PASSTHRU      = 14, //no crc, acc, temp, gyro, 3*2, 1*2, 3*2 bytes
    GTBCM_GYRO_ONLY_FILTER_F     = 16, //gyro, filtered, 3*4 bytes, 4 bytes crc
    GTBCM_GYRO_ACC_FILTER_F      = 28, //gyro, filtered, acc, 3*4, 3*4, 4 byte crc
    GTBCM_GYRO_ACC_QUAT_FILTER_F = 48, //gyro, filtered, temp, filtered, acc, quaternions, filtered, 3*4, 3*4, 4*4, 1*4, 4 byte crc
    GTBCM_DEFAULT                = GTBCM_GYRO_ACC_QUAT_FILTER_F, //default mode
} gyroToBoardCommMode_t;

extern volatile gyroToBoardCommMode_t currentCommMode;


bool mpu9250SpiAccDetect(accDev_t *acc);
bool mpu9250SpiGyroDetect(gyroDev_t *gyro);

extern int ImufDetect(gyroDev_t *gyro);
extern void ImufDeviceRead(void);
extern void ImufDeviceReadComplete(void);