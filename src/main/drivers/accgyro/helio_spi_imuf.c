#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "accgyro.h"
#include "accgyro_mpu.h"
#include "helio_spi_imuf.h"
#include "accgyro_imuf9001.h"

#define IMUF_COMMAND_HEADER 'h'
#define IMUF_COMMAND_RST '13'

static imuCommFrame_t imuRxCommFrame;
static imuCommFrame_t imuTxCommFrame;

static int ImufSendReceiveCommand(gyroCommands_t commandToSend, imufCommand_t *reply, imufCommand_t *data);
static gyroToBoardCommMode_t VerifyAllowedCommMode(uint32_t commMode);
static void ImufReset(void);

static int ImufSendReceiveCommand(gyroCommands_t commandToSend, imufCommand_t *reply, imufCommand_t *data)
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
        delayUs(1000);
    //     if(HAL_GPIO_ReadPin(ports[board.gyros[0].extiPort], board.gyros[0].extiPin)) //IMU is ready to talk
    //     {
    //         inlineDigitalLo(ports[board.gyros[0].csPort], board.gyros[0].csPin);
    //         HAL_SPI_TransmitReceive(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle], (uint8_t *)&command, (uint8_t *)reply, sizeof(imufCommand_t), 100);
    //         inlineDigitalHi(ports[board.gyros[0].csPort], board.gyros[0].csPin);

    //         //this is the only valid reply we'll get if we're in BL mode
    //         if(reply->command == reply->crc && reply->command == IMUF_COMMAND_LISTENING ) //this tells us the IMU was listening for a command, else we need to reset synbc
    //         {
    //             for (attempt = 0; attempt < 100; attempt++)
    //             {
    //                 //reset command, just waiting for reply data now
    //                 command.command = IMUF_COMMAND_NONE;
    //                 command.crc     = IMUF_COMMAND_NONE;

    //                 delayUs(100); //give pin time to set
    //                 if(HAL_GPIO_ReadPin(ports[board.gyros[0].extiPort], board.gyros[0].extiPin)) //IMU is ready to talk
    //                 {
    //                     //reset attempts
    //                     attempt = 100;

    //                     inlineDigitalLo(ports[board.gyros[0].csPort], board.gyros[0].csPin);
    //                     HAL_SPI_TransmitReceive(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle], (uint8_t *)&command, (uint8_t *)reply, sizeof(imufCommand_t), 100);
    //                     inlineDigitalHi(ports[board.gyros[0].csPort], board.gyros[0].csPin);

    //                     if(reply->command == reply->crc && reply->command == commandToSend ) //this tells us the IMU understood the last command
    //                     {
    //                         return 1;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    }
    return 0;
}

static gyroToBoardCommMode_t VerifyAllowedCommMode(uint32_t commMode)
{
    switch (commMode)
    {
        case GTBCM_SETUP:
        case GTBCM_GYRO_ONLY_PASSTHRU:
        case GTBCM_GYRO_ACC_PASSTHRU:
        case GTBCM_GYRO_ONLY_FILTER_F:
        case GTBCM_GYRO_ACC_FILTER_F:
        case GTBCM_GYRO_ACC_QUAT_FILTER_F:
            return (gyroToBoardCommMode_t)commMode;
            break;
        default:
            return GTBCM_DEFAULT;
    }
}

int ImufInit()
{
    // (void)gyroLoop;

    uint32_t attempt;
    imufCommand_t reply;
    imufCommand_t data;

    // mainConfig.gyroConfig.imufMode = VerifyAllowedCommMode(mainConfig.gyroConfig.imufMode);

    // data.param1 = mainConfig.gyroConfig.imufMode; //todo verify this is allowed
    // data.param2 = mainConfig.gyroConfig.gyroRotation;
    // data.param3 = ( (uint16_t)mainConfig.gyroConfig.filterPitchQ << 16 ) | (uint16_t)mainConfig.gyroConfig.filterPitchR;
    // data.param4 = ( (uint16_t)mainConfig.gyroConfig.filterRollQ << 16 ) | (uint16_t)mainConfig.gyroConfig.filterRollR;
    // data.param5 = ( (uint16_t)mainConfig.gyroConfig.filterYawQ << 16 ) | (uint16_t)mainConfig.gyroConfig.filterYawR;

    for (attempt = 0; attempt < 3; attempt++)
    {
        ImufReset();
        DelayMs(200 * (attempt + 1));

        if (ImufSendReceiveCommand(IMUF_COMMAND_SETUP, &reply, &data))
        {
            //command a success
            // currentCommMode = mainConfig.gyroConfig.imufMode;
            return(1);
        }
    }
    return (0);

}

static void ImufReset(void)
{
    //return; //testing
    //init Reset Pin
    GPIO_InitTypeDef GPIO_InitStructure;

    // HAL_GPIO_DeInit(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin);
    // HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 1);

    // GPIO_InitStructure.Pin   = GYRO_RESET_GPIO_Pin;
    // GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    // GPIO_InitStructure.Pull  = GPIO_PULLUP;
    // HAL_GPIO_Init(ports[GYRO_RESET_GPIO_Port], &GPIO_InitStructure);

    //reset IMU
    // HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 0);
    // DelayMs(100);
    // HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 1);
    // DelayMs(100);
    // HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 0);
    // DelayMs(100);
    // HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 1);
}

int ImufDetect(gyroDev_t *gyro)
{
    uint32_t attempt;
    imufCommand_t reply;

    for (attempt = 0; attempt < 3; attempt++)
    {
            //rest IMU-f to known state
        ImufReset();
        DelayMs(200 * (attempt + 1));
        if (ImufSendReceiveCommand(IMUF_COMMAND_REPORT_INFO, &reply, NULL))
        {
            switch ( (*(imufVersion_t *)&(reply.param1)).firmware )
            {
                case 101: //version 101 allowed right now
                    // deviceWhoAmI = (*(imufVersion_t *)&(reply.param1)).firmware;
                    // return deviceWhoAmI;
                break;
                default:
                break;
            }
        }
    }
    return (0);
}

void ImufDeviceRead(void)
{
    AccGyroReadWriteData((uint8_t*)&imuTxCommFrame.gyroFrame, (uint8_t*)&imuRxCommFrame.gyroFrame, currentCommMode+2, 1);
}

void ImufDeviceReadComplete(void)
{
    //need to calibrate the imuf, we wait 1000 cycles to do it, or about 7ms at 16 KHz input.
    // if(gyroCalibrationCycles)
    // {
    //     if(gyroCalibrationCycles-- == GYRO_CALIBRATION_CYCLES)
    //     {
    //         //first calibration step, let's send the command
    //         (*(imufCommand_t *)(&imuTxCommFrame.gyroFrame)).command = 0x63636363;
    //         (*(imufCommand_t *)(&imuTxCommFrame.gyroFrame)).crc     = 0x63636363;
    //     }
    //     else
    //     {
    //         (*(imufCommand_t *)(&imuTxCommFrame.gyroFrame)).command = IMUF_COMMAND_NONE;
    //         (*(imufCommand_t *)(&imuTxCommFrame.gyroFrame)).crc     = IMUF_COMMAND_NONE;
    //     }

    // }

    if ( (currentCommMode == GTBCM_GYRO_ACC_FILTER_F) || (currentCommMode == GTBCM_GYRO_ACC_QUAT_FILTER_F))
    {
        if(
            ( imuRxCommFrame.gyroFrame.gyroX < -2000.0f || imuRxCommFrame.gyroFrame.gyroX > 2000.0f) ||
            ( imuRxCommFrame.gyroFrame.gyroY < -2000.0f || imuRxCommFrame.gyroFrame.gyroY > 2000.0f) ||
            ( imuRxCommFrame.gyroFrame.gyroZ < -2000.0f || imuRxCommFrame.gyroFrame.gyroZ > 2000.0f) ||
            ( imuRxCommFrame.gyroFrame.accelX < -20.0f || imuRxCommFrame.gyroFrame.accelX > 20.0f) ||
            ( imuRxCommFrame.gyroFrame.accelY < -20.0f || imuRxCommFrame.gyroFrame.accelY > 20.0f) ||
            ( imuRxCommFrame.gyroFrame.accelZ < -20.0f || imuRxCommFrame.gyroFrame.accelZ > 20.0f)
        )
        {
            // julian++;
        }
        else
        {
            // dpsGyroArray[0]     = -imuRxCommFrame.gyroFrame.gyroZ;
            // dpsGyroArray[1]     =  imuRxCommFrame.gyroFrame.gyroX;
            // dpsGyroArray[2]     = -imuRxCommFrame.gyroFrame.gyroY;
            // geeForceAccArray[0] =  imuRxCommFrame.gyroFrame.accelX;
            // geeForceAccArray[1] =  imuRxCommFrame.gyroFrame.accelY;
            // geeForceAccArray[2] =  imuRxCommFrame.gyroFrame.accelZ;

            if(currentCommMode == GTBCM_GYRO_ACC_QUAT_FILTER_F)
            {
                // attitudeFrameQuat.w =  imuRxCommFrame.imuFrame.w;
                // attitudeFrameQuat.x =  imuRxCommFrame.imuFrame.x;
                // attitudeFrameQuat.y =  imuRxCommFrame.imuFrame.y;
                // attitudeFrameQuat.z =  imuRxCommFrame.imuFrame.z;
            }
            // InlineFlightCode(dpsGyroArray);

        }


    }





}