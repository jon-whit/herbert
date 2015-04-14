/////////////////////////////////////////////////////////////
//
//  spi.h
//
//  SPI Driver

#ifndef spi_h
#define spi_h

#include <types.h>
#include <xspi.h>


#define SPI_BUFFER_SIZE 5000


typedef struct
{
    volatile uint32 sensorAaddr;        // Sensor Board A Address (4 bits)
    volatile uint32 sensorBaddr;        // Sensor Board B Address (4 bits)
    volatile uint32 ledChannel;         // LED Channel (and PHOTO_SEL) (4 bits)
    volatile uint32 ledSpiAddr;         // LED SPI Addr (3 bits)
    volatile uint32 ledFanEnable;       // LED Fan Enable (1 bit)
    volatile uint32 ledEnable;          // LED Illumination Enable (1 bit)
    volatile uint32 ledAnalogControl;   // LED Analog Control Enable (1 bit)
} SPIControlRegs;


#define SPI_CONTROL  (*((SPIControlRegs*)XPAR_SPI_CONTROL_BASEADDR))





typedef enum
{
    SPI_STATUS_TX_STARTED            = 0,
    SPI_STATUS_ERROR_TX_IN_PROCESS   = 1,
    SPI_STATUS_TX_ERROR              = 2,
    SPI_STATUS_RX_OK                 = 3,
    SPI_STATUS_ERROR_RX_OVERRUN      = 4,
    SPI_STATUS_ERROR_NO_RX_AVAILABLE = 5,
} SpiStatus;

typedef enum
{
    SPI_STATE_IDLE             = 0,
    SPI_STATE_TRANSMITTING     = 1,
    SPI_STATE_DONE             = 2,
    SPI_STATE_DONE_WITH_ERRORS = 3,
} SpiState;

enum SPI_OPTIONS
{
    SPI_INVERTED_DATA = 1,
};


typedef struct
{
    XSpi     hwInterface;
    volatile SpiState state;
    unsigned writeLength;
    uint8    writeBuffer[SPI_BUFFER_SIZE];
    unsigned readLength;
    uint8    readBuffer[SPI_BUFFER_SIZE];
    bool     invertedDataCopy;
} SpiInterface;



void         spiInterfaceInit(SpiInterface* interface, uint16 deviceId, uint8 interruptId, uint32 xilOptions, uint32 options);

bool         spiWaitForIdle(SpiInterface* interface, uint32 timeout_us);
bool         spiWaitForReady(SpiInterface* interface, uint32 timeout_us);

SpiStatus    spiStartTransmit(SpiInterface* interface, uint8* writeBuffer, uint16 writeLength);
SpiStatus    spiReadData(SpiInterface* interface, uint8* readBuffer, uint16 maxReadLength, uint16* readLength);

SpiStatus    spiWaitAndStartTransmit(SpiInterface* interface, uint8* writeBuffer, uint16 writeLength, uint32 timeout_us);
SpiStatus    spiWaitAndReadData(SpiInterface* interface, uint8* readBuffer, uint16 maxReadLength, uint16* readLength, uint32 timeout_us);

void         spiReset(SpiInterface* interface);

void         spiEmpty(SpiInterface* interface);

void         spiSetOptions(SpiInterface* interface, uint32 xilOptions);

#endif
