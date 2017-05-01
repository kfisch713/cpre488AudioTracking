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
#include "xparameters.h"
#include "xspi.h"
#include "xstatus.h"
#include "xintc.h"
#include "xil_exception.h"
#include "sleep.h"

#define SS_0 (0x1)
#define SS_1 (0x2)

#define BUFFER_SIZE 10

XSpi spi0;   // spi instance
XIntc intr0; // interrupt instance

//see XSpi data type
void print_spi_debug(){
	printf("base address: 0x%x\n", spi0.BaseAddr);
	printf("is ready: 0x%x\n", spi0.IsReady);
	printf("is started: 0x%x\n", spi0.IsStarted);
	printf("has FIFOs: %d\n", spi0.HasFifos);
	printf("slave only: %ld\n", spi0.SlaveOnly);
	printf("number slave select bits: %d\n", spi0.NumSlaveBits);
	printf("data width: %d\n", spi0.DataWidth);
	printf("spi mode: %d\n", spi0.SpiMode);
	printf("slave select mask: %ld\n", spi0.SlaveSelectMask);
	printf("slave select reg %ld\n", spi0.SlaveSelectReg);
	printf("\n");
	printf("send buffer address %d\n", spi0.SendBufferPtr);
	printf("receive buffer address: %d\n", spi0.RecvBufferPtr);
	printf("total bytes to transfer: %u\n", spi0.RequestedBytes);
	printf("bytes left to transfer: %u\n", spi0.RemainingBytes);
	printf("is busy: %d\n", spi0.IsBusy);
	printf("\n");
}

void SpiIntrHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount)
{
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		printf("some weird status event\n");
	}
	else{
		printf("transfer complete\n");
	}
}

//TODO check LSB/MSB
//TODO check Control Reg again. All items are default low.
XStatus spi_init(){

	XSpi_Initialize(&spi0, XPAR_SPI_0_DEVICE_ID);

	XSpi_IntrGlobalEnable(&spi0); //enable interrupts
	//XSpi_IntrEnable(&spi0, XSP_INTR_RX_OVERRUN_MASK | XSP_INTR_RX_FULL_MASK); //enable receive data overrun interrupt and receiver buffer full interrupt
	XSpi_IntrEnable(&spi0, XSP_INTR_RX_FULL_MASK); //enable receiver buffer full interrupt
	XSpi_SetControlReg(&spi0, XSP_CR_TXFIFO_RESET_MASK | XSP_CR_RXFIFO_RESET_MASK | XSP_CR_MASTER_MODE_MASK | XSP_CR_ENABLE_MASK); // reset FIFOs, spi set to master, system enabled
	XSpi_SetSlaveSelectReg(&spi0, SS_0); // defautl to reading slave select 1
	XSpi_Enable(&spi0); //enable spi

	XStatus status = XSpi_SelfTest(&spi0);
	if (status != XST_SUCCESS) {
		printf("self test failed. Error code %d.\n look up error code in xstatus.h\n", status);
	}
	else{
		printf("self test success\n");
	}

	XSpi_SetStatusHandler(&spi0, &spi0,
			 		(XSpi_StatusHandler) SpiIntrHandler);

	return status;
}

void intr_init(){
	//TODO this function needs work. Check the documentation and examples in mss or mhs.
	XStatus Status = XIntc_Initialize(&intr0, XPAR_AXI_INTC_0_DEVICE_ID);
	Status = XIntc_Connect(&intr0, XPAR_INTC_0_SPI_0_VEC_ID,
		 			(XInterruptHandler) XSpi_InterruptHandler,
					(void *)&spi0);

	Status = XIntc_Start(&intr0, XIN_REAL_MODE);
	XIntc_Enable(&intr0, XPAR_INTC_0_SPI_0_VEC_ID);
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)
				XIntc_InterruptHandler,
				&intr0);
	Xil_ExceptionEnable();
}

int main()
{
	int i;
	u8 write[BUFFER_SIZE], read[BUFFER_SIZE];


    init_platform();
    spi_init();
    print_spi_debug();
    XSpi_Start(&spi0);
    print_spi_debug();

    for(i=0; i<BUFFER_SIZE; i++){
    	write[i] = i;
    	read[i] = 0;
    }

    XSpi_Transfer(&spi0, write, read, BUFFER_SIZE);
    sleep(1);

    for(i=0; i<BUFFER_SIZE; i++){
		printf("read[%d] = %u\n", i, read[i]);
	}



    return 0;
}
