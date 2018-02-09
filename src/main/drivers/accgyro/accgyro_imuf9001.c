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

#include "platform.h"

#include "common/axis.h"
#include "common/maths.h"

#include "drivers/bus_spi.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/sensor.h"
#include "drivers/time.h"

#include "accgyro.h"
#include "accgyro_mpu.h"
#include "accgyro_imuf9001.h"



volatile gyroToBoardCommMode_t currentCommMode;

typedef struct imufVersion
{   
    uint32_t hardware;
    uint32_t firmware;
    uint32_t bootloader;
    uint32_t uid1;
    uint32_t uid2;
    uint32_t uid3;
} __attribute__((__packed__)) imufVersion_t;

typedef struct imufCommand {
   uint32_t command;
   uint32_t param1;
   uint32_t param2;
   uint32_t param3;
   uint32_t param4;
   uint32_t param5;
   uint32_t param6;
   uint32_t param7;
   uint32_t param8;
   uint32_t param9;
   uint32_t param10;
   uint32_t crc;
   uint32_t tail;
} __attribute__ ((__packed__)) imufCommand_t;

typedef enum gyroCommands
{
    IMUF_COMMAND_NONE            = 0,
    IMUF_COMMAND_CALIBRATE       = 99,
    IMUF_COMMAND_LISTENING       = 108,
    IMUF_COMMAND_REPORT_INFO     = 121,
    IMUF_COMMAND_SETUP           = 122,
    IMUF_COMMAND_RESTART         = 127,
} gyroCommands_t;

typedef struct gyroFrame
{
    float gyroX;
    float gyroY;
    float gyroZ;
    float accelX;
    float accelY;
    float accelZ;
    float temp;
} __attribute__((__packed__)) gyroFrame_t;

typedef struct imuFrame
{
    float w;
    float x;
    float y;
    float z;
} __attribute__((__packed__)) imuFrame_t;

typedef struct imuCommFrame
{
    uint8_t     flags4;
    uint8_t     flags3;
    uint8_t     flags2;
    uint8_t     flags;
    gyroFrame_t gyroFrame;
    imuFrame_t  imuFrame;
    uint32_t    tail;
} __attribute__((__packed__)) imuCommFrame_t;

typedef enum imufLoopHz
{
    IMUF_32000 = 0,
    IMUF_16000 = 1,
    IMUF_8000  = 2,
    IMUF_4000  = 3,
    IMUF_2000  = 4,
    IMUF_1000  = 5,
    IMUF_500   = 6,
    IMUF_250   = 7,
} imufLoopHz_t;

typedef enum imufOutput
{
    IMUF_GYRO_OUTPUT = 1 << 0,
    IMUF_TEMP_OUTPUT = 1 << 1,
    IMUF_ACC_OUTPUT  = 1 << 2,
    IMUF_QUAT_OUTPUT = 1 << 3,
} imufOutput_t;

typedef enum imufOreintation
{
    IMU_CW0       = 0,
    IMU_CW90      = 1,
    IMU_CW180     = 2,
    IMU_CW270     = 3,
    IMU_CW0_INV   = 4,
    IMU_CW90_INV  = 5,
    IMU_CW180_INV = 6,
    IMU_CW270_INV = 7,
    IMU_CW45      = 8,
    IMU_CW135     = 9,
    IMU_CW225     = 10,
    IMU_CW315     = 11,
    IMU_CW45_INV  = 12,
    IMU_CW135_INV = 13,
    IMU_CW225_INV = 14,
    IMU_CW315_INV = 15,
    IMU_CUSTOM    = 16,
} imufOrientation_t;


static void imuf9001SpiInit(const busDevice_t *bus)
{
    static bool hardwareInitialised = false;

    if (hardwareInitialised) {
        return;
    }

    IOInit(bus->busdev_u.spi.csnPin, OWNER_MPU_CS, 0);
    IOConfigGPIO(bus->busdev_u.spi.csnPin, SPI_IO_CS_CFG);
    IOHi(bus->busdev_u.spi.csnPin);

    //config exti as input, not exti for now
    IOInit(gyro->mpuIntExtiTag, OWNER_MPU_EXTI, 0);
    IOConfigGPIO(gyro->mpuIntExtiTag, IOCFG_IPD);

    spiSetDivisor(bus->busdev_u.spi.instance, SPI_CLOCK_FAST);

    hardwareInitialised = true;
}

bool imufSendReceiveSpiBlocking(const busDevice_t *bus, uint8_t *dataTx, uint8_t *daRx, uint8_t length)
{
    IOLo(bus->busdev_u.spi.csnPin);
    spiTransfer(bus->busdev_u.spi.instance, dataTx, daRx, length);
    IOHi(bus->busdev_u.spi.csnPin);

    return true;
}

static int imuf9001SendReceiveCommand(gyroCommands_t commandToSend, imufCommand_t *reply, imufCommand_t *data)
{

    imufCommand_t command;
    uint32_t attempt;
    int failCount = 500;

    memset(reply, 0, sizeof(command));

    if (data)
    {
        memcpy(&command, data, sizeof(command));
    }
    else
    {
        memset(&command, 0, sizeof(command));
    }

    command.command = commandToSend;
    command.crc     = commandToSend;


    while (failCount--)
    {
        delayMicroseconds(1000);
        if( IORead(IOGetByTag(gyro->mpuIntExtiTag)) ) //IMU is ready to talk
        {
            imufSendReceiveSpiBlocking(bus, (uint8_t *)&command, (uint8_t *)reply, sizeof(imufCommand_t));

            //this is the only valid reply we'll get if we're in BL mode
            if(reply->command == reply->crc && reply->command == IMUF_COMMAND_LISTENING ) //this tells us the IMU was listening for a command, else we need to reset synbc
            {
                for (attempt = 0; attempt < 100; attempt++)
                {
                    //reset command, just waiting for reply data now
                    command.command = IMUF_COMMAND_NONE;
                    command.crc     = IMUF_COMMAND_NONE;

                    delayMicroseconds(100); //give pin time to set

                    if( IORead(IOGetByTag(gyro->mpuIntExtiTag)) ) //IMU is ready to talk
                    {
                        //reset attempts
                        attempt = 100;

                        imufSendReceiveSpiBlocking(bus, (uint8_t *)&command, (uint8_t *)reply, sizeof(imufCommand_t));

                        if(reply->command == reply->crc && reply->command == commandToSend ) //this tells us the IMU understood the last command
                        {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

uint8_t imuf9001SpiDetect(const busDevice_t *bus)
{
    mpu6500SpiInit(bus);

    const uint8_t whoAmI = spiBusReadRegister(bus, MPU_RA_WHO_AM_I);

    uint8_t mpuDetected = MPU_NONE;
    switch (whoAmI) {
    case WHOAMI_9001:
        mpuDetected = IMUF_9001_SPI;
        break;
    default:
        mpuDetected = MPU_NONE;
    }
    return mpuDetected;
}

void mpu6500SpiAccInit(accDev_t *acc)
{
    mpu6500AccInit(acc);
}

void mpu6500SpiGyroInit(gyroDev_t *gyro)
{
    spiSetDivisor(gyro->bus.busdev_u.spi.instance, SPI_CLOCK_SLOW);
    delayMicroseconds(1);

    mpu6500GyroInit(gyro);

    // Disable Primary I2C Interface
    spiBusWriteRegister(&gyro->bus, MPU_RA_USER_CTRL, MPU6500_BIT_I2C_IF_DIS);
    delay(100);

    spiSetDivisor(gyro->bus.busdev_u.spi.instance, SPI_CLOCK_FAST);
    delayMicroseconds(1);
}

bool mpu6500SpiAccDetect(accDev_t *acc)
{
    // MPU6500 is used as a equivalent of other accelerometers by some flight controllers
    switch (acc->mpuDetectionResult.sensor) {
    case MPU_65xx_SPI:
    case MPU_9250_SPI:
    case ICM_20608_SPI:
    case ICM_20602_SPI:
        break;
    default:
        return false;
    }

    acc->initFn = mpu6500SpiAccInit;
    acc->readFn = mpuAccRead;

    return true;
}

bool mpu6500SpiGyroDetect(gyroDev_t *gyro)
{
    // MPU6500 is used as a equivalent of other gyros by some flight controllers
    switch (gyro->mpuDetectionResult.sensor) {
    case MPU_65xx_SPI:
    case MPU_9250_SPI:
    case ICM_20608_SPI:
    case ICM_20602_SPI:
        break;
    default:
        return false;
    }

    gyro->initFn = mpu6500SpiGyroInit;
    gyro->readFn = mpuGyroReadSPI;

    // 16.4 dps/lsb scalefactor
    gyro->scale = 1.0f / 16.4f;

    return true;
}
