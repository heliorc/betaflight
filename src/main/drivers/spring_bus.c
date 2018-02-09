#include "includes.h"


uint32_t skipGyro     = 1;
volatile uint32_t gyroInterrupting;

static void ImufSpiInit(uint32_t baudRatePrescaler)
{

    //init CS pin
    GPIO_InitTypeDef GPIO_InitStructure;

    HAL_GPIO_WritePin(ports[board.gyros[0].csPort], board.gyros[0].csPin, 1);

    GPIO_InitStructure.Pin   = board.gyros[0].csPin;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(ports[board.gyros[0].csPort], &GPIO_InitStructure);
    HAL_GPIO_WritePin(ports[board.gyros[0].csPort], board.gyros[0].csPin, 1);

	spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Instance               = spiInstance[board.gyros[0].spiNumber];
    HAL_SPI_DeInit(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle]);

    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.Mode              = SPI_MODE_MASTER;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.Direction         = SPI_DIRECTION_2LINES;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.DataSize          = SPI_DATASIZE_8BIT;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.CLKPolarity       = SPI_POLARITY_LOW;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.CLKPhase          = SPI_PHASE_1EDGE;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.NSS               = SPI_NSS_SOFT;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.BaudRatePrescaler = baudRatePrescaler;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.FirstBit          = SPI_FIRSTBIT_MSB;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.TIMode            = SPI_TIMODE_DISABLE;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle].Init.CRCPolynomial     = 7;

    if (HAL_ImufSpiInit(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle]) != HAL_OK)
    {
        ErrorHandler(GYRO_SPI_INIT_FAILIURE);
    }

}

void DeInitGyroExti(void)
{
	EXTI_Deinit(ports[board.gyros[0].extiPort], board.gyros[0].extiPin, board.gyros[0].extiIRQn);
}

void InitGyroExti(void)
{
	EXTI_Init(ports[board.gyros[0].extiPort], board.gyros[0].extiPin, board.gyros[0].extiIRQn, 2, 0, GPIO_MODE_IT_RISING, GPIO_PULLUP);
}

void AccGyroDeinit(void)
{

    //ensure the interrupt is not running
	HAL_NVIC_DisableIRQ(board.gyros[0].extiIRQn);

    //deinit exti
    EXTI_Deinit(ports[board.gyros[0].extiPort], board.gyros[0].extiPin, board.gyros[0].extiIRQn);

    //SPI DeInit will disable the GPIOs, DMAs, IRQs and SPIs attached to this SPI handle
    HAL_SPI_DeInit(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle]); //TODO: Remove all HAL and place these functions in the stm32.c file so we can support other MCU families.

	//Deinit the EXTI
    DeInitGyroExti();
}

uint32_t AccGyroInit(loopCtrl_e loopCtrl)
{

    //prepare DMA array
	if (board.dmasSpi[board.spis[board.gyros[0].spiNumber].RXDma].enabled)
    {
		memcpy( &board.dmasActive[board.spis[board.gyros[0].spiNumber].RXDma], &board.dmasSpi[board.spis[board.gyros[0].spiNumber].RXDma], sizeof(board_dma) );
	}
	if (board.dmasSpi[board.spis[board.gyros[0].spiNumber].TXDma].enabled)
    {
		memcpy( &board.dmasActive[board.spis[board.gyros[0].spiNumber].TXDma], &board.dmasSpi[board.spis[board.gyros[0].spiNumber].TXDma], sizeof(board_dma) );
	}

    InitializeGpioInput(ports[ACTUATOR1_GPIO], ACTUATOR1_PIN, GPIO_PULLUP);
    //TESTI: //testing
    ////////////////////////TEST, wait for motor 1 to ground before running
    //DelayMs(100); //testing
    //while(1) //testing
    //{ //testing
    //    if(!HAL_GPIO_ReadPin(ports[ACTUATOR1_GPIO], ACTUATOR1_PIN)) //testing
    //    { //testing
    //        break; //testing
    //    } //testing
    //} //testing

    //deinit gyro
	AccGyroDeinit();

    //init SPI, slow speed
	ImufSpiInit(board.gyros[0].spiSlowBaud);

    //init extiPin as input so we can see gyro ready satus in blocking mode
    InitializeGpioInput(ports[board.gyros[0].extiPort], board.gyros[0].extiPin, GPIO_NOPULL);

    if (!AccGyroDeviceDetect()) 
    {
        //make sure IMU is a known version
        return 0; //testing
    }

    if (!AccGyroDeviceInit(loopCtrl)) {
        return 0; //testing
    }

    //goto TESTI; //testing
   	ImufSpiInit(board.gyros[0].spiFastBaud);

    // after the gyro is started, start up the interrupt
	InitGyroExti();

    skipGyro = 0;

    return 1;
}

void GyroExtiCallback(uint32_t callbackNumber)
{

	HAL_GPIO_EXTI_IRQHandler(board.gyros[0].extiPin);

	(void)(callbackNumber);

	gyroInterrupting = 1;
    if (!skipGyro)
    {
        ImuDeviceRead();
    }
}

void GyroRxDmaCallback(uint32_t callbackNumber)
{
    volatile HAL_DMA_StateTypeDef dmaState;
	(void)(callbackNumber);
    dmaState = HAL_DMA_GetState(&dmaHandles[board.dmasActive[board.spis[board.gyros[0].spiNumber].RXDma].dmaHandle]);
    //while(HAL_DMA_GetState(&dmaHandles[board.dmasActive[board.spis[board.gyros[0].spiNumber].RXDma].dmaHandle]) == HAL_DMA_STATE_BUSY);
    if (dmaState == HAL_DMA_STATE_READY)
    {
        // run callback for completed gyro read
        ImuDeviceReadComplete();
    }
}


uint32_t AccGyroReadWriteData(uint8_t *txData, uint8_t *rxData, uint8_t length, int useDma)
{

    int x;
    volatile HAL_DMA_StateTypeDef dmaState;
    volatile HAL_SPI_StateTypeDef spiState;

    if (useDma)
    {
        dmaState = HAL_DMA_GetState(&dmaHandles[board.dmasActive[board.spis[board.gyros[0].spiNumber].RXDma].dmaHandle]);
        spiState = HAL_SPI_GetState(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle]);

        if (dmaState == HAL_DMA_STATE_READY && spiState == HAL_SPI_STATE_READY)
        {
            HAL_SPI_TransmitReceive_DMA(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle], txData, rxData, length);
            return 1;
        }
        else
        {
            return 0;
        }

    }
    else
    {
        // poll until SPI is ready in case of ongoing DMA
        for (x=99999999;x>0;x--)
        {
            //only sit and spool for a limited amount of iterations
            if (HAL_SPI_GetState(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle]) == HAL_SPI_STATE_READY)
            {
                break;
            }
        }

        if(!x)
        {
            //if x is 0 then the spi was never ready
            return 0;
        }

        volatile HAL_StatusTypeDef  cat = 0;
        cat = HAL_SPI_TransmitReceive(&spiHandles[board.spis[board.gyros[0].spiNumber].spiHandle], txData, rxData, length, 100);
        cat = 1;
        return cat;
    }

}