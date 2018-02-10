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
#include "helio_spi_imuf.h"
#include "accgyro_imuf9001.h"

static gyroDev_t *gyroD;
static accDev_t *accD;
static void imuf9001SpiInit(const busDevice_t *bus)
{
    static bool hardwareInitialised = false;

    if (hardwareInitialised) {
        return;
    }

    IOInit(bus->busdev_u.spi.csnPin, OWNER_MPU_CS, 0);
    IOConfigGPIO(bus->busdev_u.spi.csnPin, SPI_IO_CS_CFG);
    IOHi(bus->busdev_u.spi.csnPin);

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
        if( IORead(IOGetByTag(gyroD->mpuIntExtiTag)) ) //IMU is ready to talk
        {
            imufSendReceiveSpiBlocking(gyroD->bus, (uint8_t *)&command, (uint8_t *)reply, sizeof(imufCommand_t));

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

                        imufSendReceiveSpiBlocking(gyroD->bus, (uint8_t *)&command, (uint8_t *)reply, sizeof(imufCommand_t));

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

void imufSpiAccInit(accDev_t *acc)
{
    imuf9001SpiInit(&(acc->bus);
     //    ////making sausages
    IOConfigGPIO(acc->mpuIntExtiTag, IOCFG_IPD);

    //config exti as input, not exti for now
    IOInit(acc->mpuIntExtiTag, OWNER_MPU_EXTI, 0);
    IOConfigGPIO(acc->mpuIntExtiTag, IOCFG_IPD);

    spiSetDivisor(acc->bus.busdev_u.spi.instance, SPI_CLOCK_SLOW);
    delayMicroseconds(1);

    imufCommand_t txData;
    imufCommand_t rxData;
    imuf9001SendReceiveCommand(IMUF_COMMAND_SETUP, &txData, &rxData);

    // Disable Primary I2C Interface
    spiBusWriteRegister(&gyro->bus, MPU_RA_USER_CTRL, MPU6500_BIT_I2C_IF_DIS);
    delay(100);

    spiSetDivisor(gyro->bus.busdev_u.spi.instance, SPI_CLOCK_FAST);
    delayMicroseconds(1);
}

void imufSpiGyroInit(gyroDev_t *gyro)
{

    imuf9001SpiInit(&(gyro->bus);
    //    ////making sausages
    IOConfigGPIO(gyro->mpuIntExtiTag, IOCFG_IPD);

    //config exti as input, not exti for now
    IOInit(gyro->mpuIntExtiTag, OWNER_MPU_EXTI, 0);
    IOConfigGPIO(gyro->mpuIntExtiTag, IOCFG_IPD);

    spiSetDivisor(gyro->bus.busdev_u.spi.instance, SPI_CLOCK_SLOW);
    delayMicroseconds(1);

    
    imufCommand_t txData;
    imufCommand_t rxData;
    imuf9001SendReceiveCommand(IMUF_COMMAND_SETUP, &txData, &rxData);

    // Disable Primary I2C Interface
    spiBusWriteRegister(&gyro->bus, MPU_RA_USER_CTRL, MPU6500_BIT_I2C_IF_DIS);
    delay(100);

    spiSetDivisor(gyro->bus.busdev_u.spi.instance, SPI_CLOCK_FAST);
    delayMicroseconds(1);
}


bool imuf9001ReadGyro(gyroDev_t *gyro)
{
    imufCommand_t txData;
    imufCommand_t rxData;
    imuf9001SendReceiveCommand(IMUF_COMMAND_LISTENING, txData, rxData);
    // get me data imuf9001SendReceiveCommand()
    //set the gyro data for each axis.
    
    // static const uint8_t dataToSend[7] = {MPU_RA_GYRO_XOUT_H | 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    // uint8_t data[7];

    // const bool ack = spiBusTransfer(&gyro->bus, dataToSend, data, 7);
    // if (!ack) {
    //     return false;
    // }

    // gyro->gyroADCRaw[X] = (int16_t)((data[1] << 8) | data[2]);
    // gyro->gyroADCRaw[Y] = (int16_t)((data[3] << 8) | data[4]);
    // gyro->gyroADCRaw[Z] = (int16_t)((data[5] << 8) | data[6]);

    return true;
}

uint8_t imuf9001ReadAcc(const accDev_t *gyro)
{
    imufCommand_t txData;
    imufCommand_t rxData;
    imuf9001SendReceiveCommand(IMUF_COMMAND_LISTENING, txData, rxData);
    //read to the accDev_t

    // get me data imuf9001SendReceiveCommand

    // const uint8_t whoAmI = spiBusReadRegister(gyro->bus, MPU_RA_WHO_AM_I);

    // uint8_t mpuDetected = MPU_NONE;
    // switch (whoAmI) {
    // case WHOAMI_9001:
    //     mpuDetected = IMUF_9001_SPI;
    //     break;
    // default:
    //     mpuDetected = MPU_NONE;
    // }
    // return mpuDetected;
}

bool imufAccDetect(accDev_t *acc)
{
    accD = acc
    // MPU6500 is used as a equivalent of other accelerometers by some flight controllers
    switch (acc->mpuDetectionResult.sensor) {
    case IMUF_9001_SPI:
        break;
    default:
        return false;
    }

    acc->initFn = imufSpiAccInit;
    acc->readFn = imuf9001ReadAcc;

    return true;
}

bool imufGyroDetect(gyroDev_t *gyro)
{
    gyroD = gyro;
    // MPU6500 is used as a equivalent of other gyros by some flight controllers
    switch (gyro->mpuDetectionResult.sensor) {
    case IMUF_9001_SPI:
        break;
    default:
        return false;
    }

    gyro->initFn = imufSpiGyroInit;
    gyro->readFn = imuf9001ReadGyro;

    // 16.4 dps/lsb scalefactor
    gyro->scale = 1.0f / 16.4f;

    return true;
}
