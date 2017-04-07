/************************************************************************/
/*																		*/
/*	MIC3.cpp		--		Definition for MIC3 library 	    		*/
/*																		*/
/************************************************************************/
/*	Author:		Eric  Marsh												*/
/*	Copyright 2016, Digilent Inc.										*/
/************************************************************************/
/*  File Description:													*/
/*		This file defines functions for MIC3							*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	07/28/2016(EricM): created											*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include "MIC3.h"
#include <Dspi.h>


/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*        MIC3::MIC3
**
**        Synopsis:
**				
**        Parameters:
**
**
**
**        Return Values:
**                void 
**
**        Errors:
**
**
**        Description:
**			Class constructor. Performs variables initialization tasks
**
**
*/
MIC3::MIC3()
{
	pdspi = NULL;
}

/* ------------------------------------------------------------ */
/*        MIC3::GetIntegerValue
**
**        Synopsis:
**				wIntegerValue = GetIntegerValue();
**        Parameters:
**
**
**        Return Values:
**                uint16_t	- the 12 bits value read from PmodMIC3
**
**        Errors:
**			If module is not initialized (using begin), the function does nothing and returns 0
**
**        Description:
**			This function returns the 12 bits value read from the PmodMIC3, obtained by reading 16 bits through the SPI interface. 
**
**
*/
uint16_t MIC3::GetIntegerValue()
{
	uint16_t wResult = 0;
	uint8_t *pbResult = (uint8_t *)&wResult;
	if(pdspi != NULL)
	{
		// make SS active
		digitalWrite(m_SSPin, LOW);		
		
		// read from SPI, two separate 8 bits values
		*(pbResult + 1) = pdspi->transfer((uint32_t) 0); // high byte
		*pbResult = pdspi->transfer((uint32_t) 0);	// low byte

		// make SS inactive
		digitalWrite(m_SSPin, HIGH);
	}
	return wResult;
}

/* ------------------------------------------------------------ */
/*        MIC3::GetPhysicalValue
**
**        Synopsis:
**				dPhysicalValue = GetPhysicalValue();
**        Parameters:
**				- float dReference - the value corresponding to the maximum converter value. If this parameter is not provided, it has a default value of 3.3.
**									
**
**        Return Values:
**                float	- the value corresponding to the value read from the PmodMIC3 and to the reference value
**
**        Errors:
**			If module is not initialized (using begin), the function does nothing and returns 0
**
**        Description:
**			This function returns the value corresponding to the value read from the PmodMIC3 and to the selected reference value.
**			If the function argument is missing, 3.3 value is used as reference value.
**
**
*/
#ifdef MIC3_FLOATING_POINT

float MIC3::GetPhysicalValue(float dReference)
{
	uint16_t wIntegerValue = GetIntegerValue();
	float dValue = (float)wIntegerValue * (dReference /((1<<MIC3_NO_BITS) - 1));
	return dValue;
}

#endif

/* ------------------------------------------------------------ */
/*        MIC3::begin
**
**        Synopsis:
**				myADCSPI.begin(PAR_ACCESS_SPI0);
**        Parameters:
**				uint8_t bAccessType	- the SPI interface where the Pmod is connected. It can be one of:
**					0	PAR_ACCESS_SPI0	- indicates SPI port 2, connetor JB
**					1	PAR_ACCESS_SPI1	- indicates SPI port 1, connector J1
**
**
**        Return Values:
**                void 
**
**        Errors:
**
**
**        Description:
**				This function initializes the specific SPI interface used, setting the SPI frequency to a default value of 1 MHz.
**
**
*/
void MIC3::begin(uint8_t bAccessType)
{
	if(bAccessType == PAR_ACCESS_SPI0)
	{
		pdspi = new DSPI0();
		m_SSPin = PIN_DSPI0_SS;	// default SS pin for SPI0
	}
	if(bAccessType == PAR_ACCESS_SPI1)
	{
		pdspi = new DSPI1();
		m_SSPin = PIN_DSPI1_SS;	// default SS pin for SPI1
	}
	if(pdspi != NULL)
	{
		pdspi->begin(m_SSPin);	// this defines SS pin as output, sets SS high
		pdspi->setMode(DSPI_MODE0);
		pdspi->setSpeed(MIC3_SPI_FREQ);
	}
}





