/************************************************************************/
/*																		*/
/*	MIC3.h	--	Declaration for MIC3 library 	    					*/
/*																		*/
/************************************************************************/
/*	Author:		Eric Marsh 												*/
/*	Copyright 2016, Digilent Inc.										*/
/************************************************************************/
/*  File Description:													*/
/*	This file declares the MIC3 library functions and the constants	*/
/*	involved.															*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	07/28/2016(EricM): created											*/
/*																		*/
/************************************************************************/
#if !defined(MIC3_H)
#define MIC3_H

#define MIC3_FLOATING_POINT

/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include <inttypes.h>
#include <DSPI.h>

/* ------------------------------------------------------------ */
/*					Definitions									*/
/* ------------------------------------------------------------ */
#define MIC3_NO_BITS		12

#define MIC3_SPI_FREQ	1000000 // 1 MHz - default spi freq
#define	PAR_ACCESS_SPI0			0
#define	PAR_ACCESS_SPI1			1
#define	PAR_ACCESS_I2C			2	
/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */


class MIC3 {
private: 
	DSPI *pdspi;
	uint8_t m_SSPin;

public:
	uint16_t GetIntegerValue();

#ifdef MIC3_FLOATING_POINT
	float GetPhysicalValue(float dReference = 3.3);
#endif

	MIC3();
	void begin(uint8_t bAccessType);
};



#endif
