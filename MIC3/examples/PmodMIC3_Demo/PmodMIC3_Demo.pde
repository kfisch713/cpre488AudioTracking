/************************************************************************/
/*				        	                        */
/*	  PmodMIC3 Demo Project 		                        */
/*					    	                        */
/************************************************************************/
/*	Author: Eric Marsh					        */
/*	Copyright 2016, Digilent Inc.					*/
/************************************************************************/
/*  File Description: 			             		        */
/*					                        	*/
/* This file implements a simple demo application that demonstrates     */
/* how to setup and use the PmodMIC3.				        */
/*									*/
/*	Functionality:							*/
/*									*/
/* In the setup() function, the PmodMIC3 is initialized through         */
/* calling the MIC3 library.                                            */
/*                                                                      */
/*                                                                      */
/* In the loop() function, the application repeatedly reads             */
/* the PmodMIC3.                                                        */
/*					       	                        */
/*	Required Hardware:		                                */
/*	  1. PIC32 based Microcontroller    	                        */
/*	  2. PmodMIC3                                             	*/
/*			                                                */
/************************************************************************/
/*  Revision History:			        			*/
/*					                        	*/
/*	07/27/2016(EricM): Created	       			        */
/*                                                                      */
/*					      	                        */
/************************************************************************/

/* -------------------------------------------------------------------- */
/*		        Include File Definitions                     	*/
/* -------------------------------------------------------------------- */
#include <MIC3.h>
#include <DSPI.h>

/* -------------------------------------------------------------------- */
/*		            Global Variables                     	*/
/* -------------------------------------------------------------------- */
MIC3 myMIC3;
int intValue;
float phyValue;

void setup()
{
  // Initialize serial monitor.
  Serial.begin(9600);
  
  // Initialize PmodMIC3.
  myMIC3.begin(PAR_ACCESS_SPI0);
  
}

void loop()
{
  // Receive an integer value reading of the PmodMIC3
  intValue = myMIC3.GetIntegerValue();
  
  // Receive an physical, decimal value reading of the PmodMIC3
  phyValue = myMIC3.GetPhysicalValue();
  
  // Print out these readings
  Serial.print("Integer value: ");
  Serial.print(intValue);
  Serial.print("  Physical value: ");
  Serial.println(phyValue);
  
  // Wait a bit
  delay(100);
}
