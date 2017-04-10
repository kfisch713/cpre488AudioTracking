/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xspi.h"
#include "xspi_l.h"
#include "xparameters.h"
#include <stdint.h>

// XPAR_SPI_0_BASEADDR
// XPAR_SPI_0_DEVICE_ID

// XSpi_SetControlReg()
// XSpi_GetControlReg()
// XSpi_GetStatusReg()
// XSpi_SetSlaveSelectReg()
// XSpi_GetSlaveSelectReg()
// XSpi_Enable()
// XSpi_Disable()
// XSpi_Initialize()

// XSpi_Config *XSpi_LookupConfig(u16 DeviceId);
// int XSpi_SetOptions(XSpi *InstancePtr, u32 Options);
// u32 XSpi_GetOptions(XSpi *InstancePtr);
// void XSpi_GetStats(XSpi *InstancePtr, XSpi_Stats *StatsPtr);
// void XSpi_ClearStats(XSpi *InstancePtr);
// int XSpi_SelfTest(XSpi *InstancePtr);
// int XSpi_CfgInitialize(XSpi *InstancePtr, XSpi_Config * Config,
//		       u32 EffectiveAddr);
//
// int XSpi_Start(XSpi *InstancePtr);
// int XSpi_Stop(XSpi *InstancePtr);
//
// void XSpi_Reset(XSpi *InstancePtr);
//
// int XSpi_SetSlaveSelect(XSpi *InstancePtr, u32 SlaveMask);
// u32 XSpi_GetSlaveSelect(XSpi *InstancePtr);
//
// int XSpi_Transfer(XSpi *InstancePtr, u8 *SendBufPtr, u8 *RecvBufPtr,
//		  unsigned int ByteCount);
//
// void XSpi_SetStatusHandler(XSpi *InstancePtr, void *CallBackRef,
//			   XSpi_StatusHandler FuncPtr);
// void XSpi_InterruptHandler(void *InstancePtr);

#define LEFT_MICROPHONE 2
#define RIGHT_MICROPHONE 1

char *SWs = (char *)XPAR_SWS_8BITS_BASEADDR;
char *BTNs = (char *)XPAR_BTNS_5BITS_BASEADDR;

#define KILL_SWITCH 7
#define RECV_BUFFER_LENGTH 32

enum button_val {
	BTN_L,
	BTN_R,
	BTN_U,
	BTN_D,
	BTN_C
};

int SW(unsigned int x) {
	return ((*SWs >> x) & 0x01);
}

int BTN(unsigned int x) {
	return ((*BTNs >> x) & 0x01);
}

int init_spi(XSpi_Config * config);

XSpi spi0;

int main()
{
	int ss;
	int status;
	uint8_t recv[RECV_BUFFER_LENGTH];

    init_platform();
    XSpi_Config config;
    init_spi(&config);
    print("Hello Quad Cities, but mostly Davenport\n\r");
    XSpi_SetSlaveSelect(&spi0, RIGHT_MICROPHONE);

    while (!SW(KILL_SWITCH)) {
    	XSpi_SetSlaveSelect(&spi0, RIGHT_MICROPHONE);
    	ss = XSpi_GetSlaveSelect(&spi0);
    	printf("ss: %d\n", ss);
    	sleep(5);
    	XSpi_SetSlaveSelect(&spi0, LEFT_MICROPHONE);
    	ss = XSpi_GetSlaveSelect(&spi0);
    	printf("ss: %d\n", ss);
    	sleep(5);

//    	status = XSpi_Transfer(&spi0, spi0.RecvBufferPtr, spi0.RecvBufferPtr, 2);
//    	if (status != XST_SUCCESS){
//    		printf("ERROR status: %d\n", status);
//    	}
//    	else{
//    		printf("data recv: %d\n", (uint16_t)recv);
//    	}
    }


    return 0;
}

int init_spi(XSpi_Config * config) {
	int status;

	xil_printf("Initializing spi...\n");

	config = XSpi_LookupConfig(XPAR_SPI_0_DEVICE_ID);
	if (!config) {
		return XST_FAILURE;
	}
	xil_printf("Look up config success\n");

	status = XSpi_CfgInitialize(&spi0, config, config->BaseAddress);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	int cr = XSpi_GetControlReg(&spi0);
	xil_printf("Control reg is 0x%X\n", cr);

	status = XSpi_SetOptions(&spi0, XSP_MASTER_OPTION); // Enable Master mode
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	cr = XSpi_GetControlReg(&spi0);
	xil_printf("Control reg is now 0x%X\n", cr);

	XSpi_SetControlReg(&spi0, 4); // Enable Master transactions

	cr = XSpi_GetControlReg(&spi0);
	xil_printf("Control reg is now 0x%X\n", cr);

	XSpi_Start(&spi0);

	cr = XSpi_GetControlReg(&spi0);
	xil_printf("Control reg is now 0x%X\n", cr);

	XSpi_IntrGlobalDisable(&spi0);
	xil_printf("Global Interrupt flag disabled\n");

	if(spi0.IsReady){
		xil_printf("SPI is ready\n");
	}
	else{
		xil_printf("SPI is not ready. ABORT MISSION!\n");
	}


	return XST_SUCCESS;
}