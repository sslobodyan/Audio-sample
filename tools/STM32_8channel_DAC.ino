/**
	TVL5630 - 8 channel 12 bit DAC application - for Maple Mini STM32

	Description:
	This sketch generates 2 complementary 1kHz sine waves (0..2V) on output ports A and B.

	Using the second SPI port (SPI_2)
	SS    <-->  PB12 <-->  BOARD_SPI2_NSS_PIN
	SCK   <-->  PB13 <-->  BOARD_SPI2_SCK_PIN
	MISO  <-->  PB14 <-->  BOARD_SPI2_MISO_PIN
	MOSI  <-->  PB15 <-->  BOARD_SPI2_MOSI_PIN
*/

#include <SPI.h>

// the settings may be different for the SPI ports
SPISettings spi2Settings(24000000, MSBFIRST, SPI_MODE2, DATA_SIZE_16BIT);

SPIClass SPI_2(2); //Create an instance of the SPI Class called SPI_2 that uses the 2nd SPI Port

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
//uint16_t sig[1024];
int16 i;
// complete sine table comprising 256 samples
uint16_t sig[] __attribute__ ((packed, aligned(2))) = {
0x800,0x832,0x864,0x896,0x8c8,0x8fa,0x92c,0x95e,0x98f,0x9c0,0x9f1,0xa22,0xa52,0xa82,0xab1,0xae0,
0xb0f,0xb3d,0xb6b,0xb98,0xbc5,0xbf1,0xc1c,0xc47,0xc71,0xc9a,0xcc3,0xceb,0xd12,0xd39,0xd5f,0xd83,
0xda7,0xdca,0xded,0xe0e,0xe2e,0xe4e,0xe6c,0xe8a,0xea6,0xec1,0xedc,0xef5,0xf0d,0xf24,0xf3a,0xf4f,
0xf63,0xf76,0xf87,0xf98,0xfa7,0xfb5,0xfc2,0xfcd,0xfd8,0xfe1,0xfe9,0xff0,0xff5,0xff9,0xffd,0xffe,
0xfff,0xffe,0xffd,0xff9,0xff5,0xff0,0xfe9,0xfe1,0xfd8,0xfcd,0xfc2,0xfb5,0xfa7,0xf98,0xf87,0xf76,
0xf63,0xf4f,0xf3a,0xf24,0xf0d,0xef5,0xedc,0xec1,0xea6,0xe8a,0xe6c,0xe4e,0xe2e,0xe0e,0xded,0xdca,
0xda7,0xd83,0xd5f,0xd39,0xd12,0xceb,0xcc3,0xc9a,0xc71,0xc47,0xc1c,0xbf1,0xbc5,0xb98,0xb6b,0xb3d,
0xb0f,0xae0,0xab1,0xa82,0xa52,0xa22,0x9f1,0x9c0,0x98f,0x95e,0x92c,0x8fa,0x8c8,0x896,0x864,0x832,
0x800,0x7cd,0x79b,0x769,0x737,0x705,0x6d3,0x6a1,0x670,0x63f,0x60e,0x5dd,0x5ad,0x57d,0x54e,0x51f,
0x4f0,0x4c2,0x494,0x467,0x43a,0x40e,0x3e3,0x3b8,0x38e,0x365,0x33c,0x314,0x2ed,0x2c6,0x2a0,0x27c,
0x258,0x235,0x212,0x1f1,0x1d1,0x1b1,0x193,0x175,0x159,0x13e,0x123,0x10a,0x0f2,0x0db,0x0c5,0x0b0,
0x09c,0x089,0x078,0x067,0x058,0x04a,0x03d,0x032,0x027,0x01e,0x016,0x00f,0x00a,0x006,0x002,0x001,
0x000,0x001,0x002,0x006,0x00a,0x00f,0x016,0x01e,0x027,0x032,0x03d,0x04a,0x058,0x067,0x078,0x089,
0x09c,0x0b0,0x0c5,0x0db,0x0f2,0x10a,0x123,0x13e,0x159,0x175,0x193,0x1b1,0x1d1,0x1f1,0x212,0x235,
0x258,0x27c,0x2a0,0x2c6,0x2ed,0x314,0x33c,0x365,0x38e,0x3b8,0x3e3,0x40e,0x43a,0x467,0x494,0x4c2,
0x4f0,0x51f,0x54e,0x57d,0x5ad,0x5dd,0x60e,0x63f,0x670,0x6a1,0x6d3,0x705,0x737,0x769,0x79b,0x7cd};
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
	sendSPI2(0x90E0);	// CTRL1: use slow speed, channel A and /B, the rest is powered down
	// prepare the signal table
/*
	i = 0;	int16_t j;
	int16_t inc = 20;
	for (j=0; j<0x1000; j+=inc)	{*p++ = 0xC000+j; i++;}
	for (j=0xfff; j>=0; j-=inc)	{*p++ = 0xC000+j; i++;}
*/
	for (i=0; i<256; i++)	{sig[i] += 0xC000;}	// add TLV5630 address bits for channel A and /B
}
/*****************************************************************************/
void loop()
{
/*
	for (uint16_t val=0xC000; val<0xD000; val+=16)	sendSPI2(val);
	for (uint16_t val=0xD000; val>0xC000; val-=16)	sendSPI2(val);
	*/
	for (i = 0; i<256; i++)
		sendSPI2(sig[i]);

}
/*****************************************************************************/
void sendSPI2(uint16_t data)
{
	int16_t tim = 14;	while ( (tim--)>0 ) asm("NOP");	// extra timing for an exact 1 kHz period
//	DBG_PIN_SET;
	NSS2_PIN_CLEAR;
//	DBG_PIN_CLEAR;
	spi_tx_reg(SPI2, data); // Write the data item to be transmitted into the SPI_DR register (this clears the TXE flag)
	while (spi_is_tx_empty(SPI2) == 0); // Wait until TXE=1
	while (spi_is_busy(SPI2) != 0); // and then wait until BSY=0
	NSS2_PIN_SET;
}