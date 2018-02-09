#include "includes.h"

#define IMUF_COMMAND_HEADER 'h'
#define IMUF_COMMAND_RST '13'

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

typedef struct imufMode
{
    uint8_t  command;       //output Hz
    uint8_t  hz;            //output Hz
    uint8_t  dataOut;       //what data to send
    uint8_t  filterLevelX;  //what filter level, 0% to 100% as a uint8_t
    uint8_t  filterLevelY;  //what filter level, 0% to 100% as a uint8_t
    uint8_t  filterLevelZ;  //what filter level, 0% to 100% as a uint8_t
    uint8_t  orientation;   //what orienetation is the IMU? 0 gives raw output, if you want to use quats this must be set right
    uint16_t rotationX;     //custom orientation X, used when orientation is set to IMU_CUSTOM
    uint16_t rotationY;     //custom orientation Y, used when orientation is set to IMU_CUSTOM
    uint16_t rotationZ;     //custom orientation Z, used when orientation is set to IMU_CUSTOM
    uint8_t  param4;         //future parameters
    uint8_t  param5;         //future parameters
    uint8_t  param6;         //future parameters
    uint8_t  param7;         //future parameters
    uint8_t  param8;         //future parameters
} __attribute__((__packed__)) imufMode_t;

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
        if(HAL_GPIO_ReadPin(ports[board.gyros[0].extiPort], board.gyros[0].extiPin)) //IMU is ready to talk
        {
            inlineDigitalLo(ports[board.gyros[0].csPort], board.gyros[0].csPin);
            HAL_SPI_TransmitReceive(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle], (uint8_t *)&command, (uint8_t *)reply, sizeof(imufCommand_t), 100);
            inlineDigitalHi(ports[board.gyros[0].csPort], board.gyros[0].csPin);

            //this is the only valid reply we'll get if we're in BL mode
            if(reply->command == reply->crc && reply->command == IMUF_COMMAND_LISTENING ) //this tells us the IMU was listening for a command, else we need to reset synbc
            {
                for (attempt = 0; attempt < 100; attempt++)
                {
                    //reset command, just waiting for reply data now
                    command.command = IMUF_COMMAND_NONE;
                    command.crc     = IMUF_COMMAND_NONE;

                    delayUs(100); //give pin time to set
                    if(HAL_GPIO_ReadPin(ports[board.gyros[0].extiPort], board.gyros[0].extiPin)) //IMU is ready to talk
                    {
                        //reset attempts
                        attempt = 100;

                        inlineDigitalLo(ports[board.gyros[0].csPort], board.gyros[0].csPin);
                        HAL_SPI_TransmitReceive(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle], (uint8_t *)&command, (uint8_t *)reply, sizeof(imufCommand_t), 100);
                        inlineDigitalHi(ports[board.gyros[0].csPort], board.gyros[0].csPin);

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

int ImufInit(loopCtrl_e gyroLoop)
{
    (void)gyroLoop;

    uint32_t attempt;
    imufCommand_t reply;
    imufCommand_t data;

    mainConfig.gyroConfig.imufMode = VerifyAllowedCommMode(mainConfig.gyroConfig.imufMode);

    data.param1 = mainConfig.gyroConfig.imufMode; //todo verify this is allowed
    data.param2 = mainConfig.gyroConfig.gyroRotation;
    data.param3 = ( (uint16_t)mainConfig.gyroConfig.filterPitchQ << 16 ) | (uint16_t)mainConfig.gyroConfig.filterPitchR;
    data.param4 = ( (uint16_t)mainConfig.gyroConfig.filterRollQ << 16 ) | (uint16_t)mainConfig.gyroConfig.filterRollR;
    data.param5 = ( (uint16_t)mainConfig.gyroConfig.filterYawQ << 16 ) | (uint16_t)mainConfig.gyroConfig.filterYawR;

    for (attempt = 0; attempt < 3; attempt++)
    {
        ImufReset();
        DelayMs(200 * (attempt + 1));

        if (ImufSendReceiveCommand(IMUF_COMMAND_SETUP, &reply, &data))
        {
            //command a success
            currentCommMode = mainConfig.gyroConfig.imufMode;
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

    HAL_GPIO_DeInit(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin);
    HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 1);

    GPIO_InitStructure.Pin   = GYRO_RESET_GPIO_Pin;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(ports[GYRO_RESET_GPIO_Port], &GPIO_InitStructure);

    //reset IMU
    HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 0);
    DelayMs(100);
    HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 1);
    DelayMs(100);
    HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 0);
    DelayMs(100);
    HAL_GPIO_WritePin(ports[GYRO_RESET_GPIO_Port], GYRO_RESET_GPIO_Pin, 1);
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
                    deviceWhoAmI = (*(imufVersion_t *)&(reply.param1)).firmware;
                    gyro.
                    return deviceWhoAmI;
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
    if(gyroCalibrationCycles)
    {
        if(gyroCalibrationCycles-- == GYRO_CALIBRATION_CYCLES)
        {
            //first calibration step, let's send the command
            (*(imufCommand_t *)(&imuTxCommFrame.gyroFrame)).command = 0x63636363;
            (*(imufCommand_t *)(&imuTxCommFrame.gyroFrame)).crc     = 0x63636363;
        }
        else
        {
            (*(imufCommand_t *)(&imuTxCommFrame.gyroFrame)).command = IMUF_COMMAND_NONE;
            (*(imufCommand_t *)(&imuTxCommFrame.gyroFrame)).crc     = IMUF_COMMAND_NONE;
        }

    }

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
            julian++;
        }
        else
        {
            dpsGyroArray[0]     = -imuRxCommFrame.gyroFrame.gyroZ;
            dpsGyroArray[1]     =  imuRxCommFrame.gyroFrame.gyroX;
            dpsGyroArray[2]     = -imuRxCommFrame.gyroFrame.gyroY;
            geeForceAccArray[0] =  imuRxCommFrame.gyroFrame.accelX;
            geeForceAccArray[1] =  imuRxCommFrame.gyroFrame.accelY;
            geeForceAccArray[2] =  imuRxCommFrame.gyroFrame.accelZ;

            if(currentCommMode == GTBCM_GYRO_ACC_QUAT_FILTER_F)
            {
                attitudeFrameQuat.w =  imuRxCommFrame.imuFrame.w;
                attitudeFrameQuat.x =  imuRxCommFrame.imuFrame.x;
                attitudeFrameQuat.y =  imuRxCommFrame.imuFrame.y;
                attitudeFrameQuat.z =  imuRxCommFrame.imuFrame.z;
            }
            InlineFlightCode(dpsGyroArray);

        }


    }





}