/**
	TVL5630 - 8 channel 12 bit DAC application - for Maple Mini STM32

	Description:
	This sketch generates 4 pairs of complementary sine waves of 1kHz (0..2V) with ~90° shift.
	The table values are sequentially output to the SPI port, complemented with extra fine-timing.

	Using the second SPI port (SPI_2)
	SS    <-->  PB12 <-->  BOARD_SPI2_NSS_PIN
	SCK   <-->  PB13 <-->  BOARD_SPI2_SCK_PIN
	MISO  <-->  PB14 <-->  BOARD_SPI2_MISO_PIN
	MOSI  <-->  PB15 <-->  BOARD_SPI2_MOSI_PIN
*/

#include <SPI.h>

#define OUTPUT_FREQUENCY	1000	// Hz

// the settings may be different for the SPI ports
SPISettings spi2Settings(18000000, MSBFIRST, SPI_MODE2, DATA_SIZE_16BIT);

SPIClass SPI_2(2); // create an instance of the SPI Class that uses the 2nd SPI Port

/*****************************************************************************/
#define PORT_PIN_SET(port, pin)	( port->regs->BSRR = (1U << pin) )
#define PORT_PIN_CLEAR(port, pin)	( port->regs->BSRR = (1U << pin) << 16 )
/*****************************************************************************/
#define SPI2_NSS_PIN		PB12   // SPI_2 Chip Select pin
#define NSS2_PIN_SET		PORT_PIN_SET(GPIOB, 12)
#define NSS2_PIN_CLEAR	PORT_PIN_CLEAR(GPIOB, 12)
/*****************************************************************************/
#define DBG_PIN			PB4
#define DBG_PIN_SET		PORT_PIN_SET(GPIOB, 4)
#define DBG_PIN_CLEAR	PORT_PIN_CLEAR( GPIOB, 4)
/*****************************************************************************/
#define NR_POINTS	100
/*****************************************************************************/
int16 i;
// complete sine tables comprising 100 samples, each shifted ~90°
uint16_t sig1[NR_POINTS] __attribute__ ((packed, aligned(2))) = {
0x000,0x004,0x010,0x024,0x040,0x064,0x090,0x0c3,0x0fd,0x13f,
0x187,0x1d6,0x22b,0x286,0x2e6,0x34c,0x3b6,0x425,0x498,0x50e,
0x587,0x602,0x680,0x6ff,0x77f,0x800,0x880,0x900,0x97f,0x9fd,
0xa78,0xaf1,0xb67,0xbda,0xc49,0xcb3,0xd19,0xd79,0xdd4,0xe29,
0xe78,0xec0,0xf02,0xf3c,0xf6f,0xf9b,0xfbf,0xfdb,0xfef,0xffb,
0xfff,0xffb,0xfef,0xfdb,0xfbf,0xf9b,0xf6f,0xf3c,0xf02,0xec0,
0xe78,0xe29,0xdd4,0xd79,0xd19,0xcb3,0xc49,0xbda,0xb67,0xaf1,
0xa78,0x9fd,0x97f,0x900,0x880,0x800,0x77f,0x6ff,0x680,0x602,
0x587,0x50e,0x498,0x425,0x3b6,0x34c,0x2e6,0x286,0x22b,0x1d6,
0x187,0x13f,0x0fd,0x0c3,0x090,0x064,0x040,0x024,0x010,0x004,};
uint16_t sig2[NR_POINTS] __attribute__ ((packed, aligned(2))) = {
0x425,0x498,0x50e,0x587,0x602,0x680,0x6ff,0x77f,0x800,0x880,
0x900,0x97f,0x9fd,0xa78,0xaf1,0xb67,0xbda,0xc49,0xcb3,0xd19,
0xd79,0xdd4,0xe29,0xe78,0xec0,0xf02,0xf3c,0xf6f,0xf9b,0xfbf,
0xfdb,0xfef,0xffb,0xfff,0xffb,0xfef,0xfdb,0xfbf,0xf9b,0xf6f,
0xf3c,0xf02,0xec0,0xe78,0xe29,0xdd4,0xd79,0xd19,0xcb3,0xc49,
0xbda,0xb67,0xaf1,0xa78,0x9fd,0x97f,0x900,0x880,0x800,0x77f,
0x6ff,0x680,0x602,0x587,0x50e,0x498,0x425,0x3b6,0x34c,0x2e6,
0x286,0x22b,0x1d6,0x187,0x13f,0x0fd,0x0c3,0x090,0x064,0x040,
0x024,0x010,0x004,0x000,0x004,0x010,0x024,0x040,0x064,0x090,
0x0c3,0x0fd,0x13f,0x187,0x1d6,0x22b,0x286,0x2e6,0x34c,0x3b6,};
uint16_t sig3[NR_POINTS] __attribute__ ((packed, aligned(2))) = {
0xbda,0xc49,0xcb3,0xd19,0xd79,0xdd4,0xe29,0xe78,0xec0,0xf02,
0xf3c,0xf6f,0xf9b,0xfbf,0xfdb,0xfef,0xffb,0xfff,0xffb,0xfef,
0xfdb,0xfbf,0xf9b,0xf6f,0xf3c,0xf02,0xec0,0xe78,0xe29,0xdd4,
0xd79,0xd19,0xcb3,0xc49,0xbda,0xb67,0xaf1,0xa78,0x9fd,0x97f,
0x900,0x880,0x800,0x77f,0x6ff,0x680,0x602,0x587,0x50e,0x498,
0x425,0x3b6,0x34c,0x2e6,0x286,0x22b,0x1d6,0x187,0x13f,0x0fd,
0x0c3,0x090,0x064,0x040,0x024,0x010,0x004,0x000,0x004,0x010,
0x024,0x040,0x064,0x090,0x0c3,0x0fd,0x13f,0x187,0x1d6,0x22b,
0x286,0x2e6,0x34c,0x3b6,0x425,0x498,0x50e,0x587,0x602,0x680,
0x6ff,0x77f,0x800,0x880,0x900,0x97f,0x9fd,0xa78,0xaf1,0xb67,};
uint16_t sig4[NR_POINTS] __attribute__ ((packed, aligned(2))) = {
0x800,0x880,0x900,0x97f,0x9fd,0xa78,0xaf1,0xb67,0xbda,0xc49,
0xcb3,0xd19,0xd79,0xdd4,0xe29,0xe78,0xec0,0xf02,0xf3c,0xf6f,
0xf9b,0xfbf,0xfdb,0xfef,0xffb,0xfff,0xffb,0xfef,0xfdb,0xfbf,
0xf9b,0xf6f,0xf3c,0xf02,0xec0,0xe78,0xe29,0xdd4,0xd79,0xd19,
0xcb3,0xc49,0xbda,0xb67,0xaf1,0xa78,0x9fd,0x97f,0x900,0x880,
0x800,0x77f,0x6ff,0x680,0x602,0x587,0x50e,0x498,0x425,0x3b6,
0x34c,0x2e6,0x286,0x22b,0x1d6,0x187,0x13f,0x0fd,0x0c3,0x090,
0x064,0x040,0x024,0x010,0x004,0x000,0x004,0x010,0x024,0x040,
0x064,0x090,0x0c3,0x0fd,0x13f,0x187,0x1d6,0x22b,0x286,0x2e6,
0x34c,0x3b6,0x425,0x498,0x50e,0x587,0x602,0x680,0x6ff,0x77f,};

/*****************************************************************************/
void setup()
{
	// Setup SPI 2
	SPI_2.beginTransaction(SPI2_NSS_PIN, spi2Settings); //Initialize the SPI_2 port.
	pinMode(SPI2_NSS_PIN, OUTPUT);
	NSS2_PIN_SET; // set NSS pin inactive
	// debug
	pinMode(DBG_PIN, OUTPUT);
	// setup TLV5630
	sendSPI2(0x8004);	// CTRL0: use internal 1V reference
	sendSPI2(0x9000);	// CTRL1: use slow speed
	// prepare the signal tables
	for (i=0; i<NR_POINTS; i++) {	// use complementary channels
		sig1[i] = 0xC400+(sig1[i]/2);	// add TLV5630 address bits for channels A and /B
		sig2[i] = 0xD400+(sig2[i]/2);	// add TLV5630 address bits for channels C and /D
		sig3[i] = 0xE400+(sig3[i]/2);	// add TLV5630 address bits for channels E and /F
		sig4[i] = 0xF400+(sig4[i]/2);	// add TLV5630 address bits for channels G and /H
	}
}
/*****************************************************************************/
void loop()
{
	for (i = 0; i<NR_POINTS; i++) {
		sendSPI2(sig1[i]);
		sendSPI2(sig2[i]);
		sendSPI2(sig3[i]);
		sendSPI2(sig4[i]);
	}
}
/*****************************************************************************/
void sendSPI2(uint16_t data)
{
	//int32_t tim = 6;	while ( (tim--)>0 ) asm("NOP");	// extra timing for an exact 1 kHz period
	NSS2_PIN_CLEAR;
	spi_tx_reg(SPI2, data); // Write the data item to be transmitted into the SPI_DR register (this clears the TXE flag)
	while (spi_is_tx_empty(SPI2) == 0); // Wait until TXE=1
	while (spi_is_busy(SPI2) != 0); // and then wait until BSY=0
	NSS2_PIN_SET;
	for (int32_t i=3; i>0; i--)	{	// extra timing for an exact 1 kHz period
		NSS2_PIN_SET;
		asm("NOP");
		NSS2_PIN_SET;
		asm("NOP");
	}
}