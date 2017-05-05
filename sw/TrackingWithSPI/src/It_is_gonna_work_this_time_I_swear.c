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
#include "xscugic.h"
#include "xil_exception.h"
#include "sleep.h"
#include "xgpio.h"

#define SS_0 (0x1)
#define SS_1 (0x2)

#define GPIO_CHANNEL 		1
#define GPIO_ALL_LEDS		0xFFFF
#define GPIO_ALL_BUTTONS	0xFFFF

#define BTN_DEVICE_ID		XPAR_BTNS_5BITS_DEVICE_ID
#define BTN_INTR_VEC_ID 	XPAR_FABRIC_BTNS_5BITS_IP2INTC_IRPT_INTR
#define SPI_DEVICE_ID       XPAR_SPI_0_DEVICE_ID
#define SPI_INTR_VEC_ID     XPAR_FABRIC_AXI_SPI_0_IP2INTC_IRPT_INTR
#define INTR_DEVICE_ID	    XPAR_SCUGIC_SINGLE_DEVICE_ID


#define BUFFER_SIZE 10

XSpi spi0;   // spi instance
XScuGic intr0; // interrupt instance
XGpio gpio0; // gpio instance

volatile int transfer_in_progress = 0;
volatile int ss = 1;
volatile int angle = 0;
volatile int distance = 0;

//see XSpi data type
void print_spi_debug(){
	printf("SPI\n");
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
	printf("send buffer address %d\n", spi0.SendBufferPtr);
	printf("receive buffer address: %d\n", spi0.RecvBufferPtr);
	printf("total bytes to transfer: %u\n", spi0.RequestedBytes);
	printf("bytes left to transfer: %u\n", spi0.RemainingBytes);
	printf("is busy: %d\n", spi0.IsBusy);
	printf("\n");
}

//see XGpio data type
void print_gpio_debug(){
	printf("GPIO\n");
	printf("base address: 0x%x\n", &gpio0.BaseAddress);
	printf("is ready: 0x%x\n", &gpio0.IsReady);
	printf("interrupts present: 0x%x\n", &gpio0.InterruptPresent);
	printf("is dual channel: 0x%x\n", &gpio0.IsDual);
	printf("\n");
}

void spiISR(void* instancePtr)
{
	XSpi *spiPtr = (XSpi *)instancePtr;

	//XScuGic_IntrDisable(spiPtr);

	transfer_in_progress = 0;

	printf("SPI INTERRUPT\n");

}

void spiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount){
	transfer_in_progress = 0;
	xil_printf("SPI HANDLER\n");
}

//TODO check LSB/MSB
//TODO check Control Reg again. All items are default low.
XStatus spi_init(){

	XSpi_Initialize(&spi0, SPI_DEVICE_ID);

	XSpi_IntrGlobalEnable(&spi0); //enable interrupts
	XSpi_IntrEnable(&spi0, XSP_INTR_RX_OVERRUN_MASK | XSP_INTR_RX_FULL_MASK); //enable receive data overrun interrupt and receiver buffer full interrupt
	//XSpi_IntrEnable(&spi0, XSP_INTR_RX_FULL_MASK); //enable receiver buffer full interrupt
	XSpi_SetControlReg(&spi0, XSP_CR_TXFIFO_RESET_MASK | XSP_CR_RXFIFO_RESET_MASK | XSP_CR_MASTER_MODE_MASK | XSP_CR_ENABLE_MASK); // reset FIFOs, spi set to master, system enabled
	XSpi_SetSlaveSelectReg(&spi0, ss); // default ss reg
	XSpi_Enable(&spi0); //enable spi

	XStatus status = XSpi_SelfTest(&spi0);
	if (status != XST_SUCCESS) {
		printf("self test failed. Error code %d.\n look up error code in xstatus.h\n", status);
	}


	status = XSpi_SetOptions(&spi0, XSP_MASTER_OPTION);
	if (status != XST_SUCCESS) {
		printf("set options failed\n");
	}

	return status;
}

XStatus spi_intr_init(XScuGic* intr, u16 deviceId, u32 int_vec_id){

	XScuGic_Config *IntcConfig = XScuGic_LookupConfig(deviceId);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	int result = XScuGic_CfgInitialize(intr, IntcConfig, IntcConfig->CpuBaseAddress);
	if (result != XST_SUCCESS) {
		return XST_FAILURE;
	}


	XScuGic_SetPriorityTriggerType(intr, int_vec_id, 0xA0, 0x3);

	result = XScuGic_Connect(intr, int_vec_id, XSpi_InterruptHandler, &spi0);
	if (result != XST_SUCCESS) {
		printf("interrupt connect failed\n");
	}

	XScuGic_Enable(intr, int_vec_id);


	//XIntc_MasterEnable(intr.BaseAddress);
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, intr);
	Xil_ExceptionEnable();




	return XST_SUCCESS;
}


void btnISR(void* instancePtr){
	XGpio *GpioPtr = (XGpio *)instancePtr;
	u32 buttons;

	XGpio_InterruptDisable(GpioPtr, GPIO_CHANNEL); //buttons are channel 1

	/*
	printf("\nBUTTON INTERRUPT\n");
	printf("ss = %ld\n", XSpi_GetSlaveSelectReg(&spi0));

	if(ss==0){
		ss=1;
	}
	else{
		ss=0;
	}

	XSpi_SetSlaveSelectReg(&spi0, ss);




	//transfer_in_progress = 0;

	//Buttons = XGpio_DiscreteRead(GpioPtr, GPIO_CHANNEL);

	//printf("button data: %ld\n", Buttons);
	print_spi_debug();
	 */

	buttons = XGpio_DiscreteRead(&gpio0, GPIO_CHANNEL);
	//center button controls angle
	if(buttons & 0x1){
		angle = angle + 5;
		if(angle > 90) angle = -90;
	}

	if(buttons & 0x2){
		distance++;
		if(distance > 10) distance = 0;
	}

	(void)XGpio_InterruptClear(GpioPtr, XGPIO_IR_CH1_MASK);
	XGpio_InterruptEnable(GpioPtr, XGPIO_IR_CH1_MASK);

}

void btn_init(){
	XStatus Status = XGpio_Initialize(&gpio0, BTN_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		printf("btn init failure\n");
	}
	Status = XGpio_SelfTest(&gpio0);
	if (Status != XST_SUCCESS) {
		printf("gpio self test failure\n");
	}
	XGpio_SetDataDirection(&gpio0, GPIO_CHANNEL, GPIO_ALL_BUTTONS);
}

XStatus btn_intr_init(XScuGic* intr, u16 deviceId, u32 int_vec_id){
	XScuGic_Config *IntcConfig = XScuGic_LookupConfig(deviceId);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	int result = XScuGic_CfgInitialize(intr, IntcConfig, IntcConfig->CpuBaseAddress);
	if (result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(intr, int_vec_id, 0xA0, 0x3);

	result = XScuGic_Connect(intr, int_vec_id, btnISR, &gpio0);
	if (result != XST_SUCCESS) {
		printf("interrupt connect failed\n");
	}

	XScuGic_Enable(intr, int_vec_id);

	XGpio_InterruptEnable(&gpio0, XGPIO_IR_MASK);
	XGpio_InterruptGlobalEnable(&gpio0);
	//XIntc_MasterEnable(intr.BaseAddress);
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, intr);
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}



int main()
{
	int i;
	u32 buttons, old_buttons=0;
	u8 write[BUFFER_SIZE], read[BUFFER_SIZE];


    init_platform();
    spi_init();
    //spi_intr_init(&intr0, SPI_DEVICE_ID, SPI_INTR_VEC_ID);
    XSpi_IntrGlobalDisable(&spi0);
    XSpi_SetStatusHandler(&spi0, &spi0, spiHandler);
    //XSpi_Start(&spi0);
    btn_init();
    btn_intr_init(&intr0, BTN_DEVICE_ID, BTN_INTR_VEC_ID);
    //print_gpio_debug();
    //print_spi_debug();


    for(i=0; i<BUFFER_SIZE; i++){
    	write[i] = i;
    	read[i] = 0;
    }

    transfer_in_progress = 1;
    XSpi_Transfer(&spi0, write, read, BUFFER_SIZE);

    //printf("entering waiting loop\n");
    //print_spi_debug();
    while(transfer_in_progress){
    	//XSpi_Transfer(&spi0, write, read, BUFFER_SIZE);
    	//printf("loop ");
//    	buttons = XGpio_DiscreteRead(&gpio0, GPIO_CHANNEL);
//    	if(buttons != old_buttons){
//    		printf("%ld, 0x%x\n", buttons, &gpio0.InterruptPresent);
//    		old_buttons = buttons;
//    	}
    	printf("%d, %d, %d, %d\n", angle, distance, 0, 0);
    	fflush(stdout);
    	sleep(1);

    }

    for(i=0; i<BUFFER_SIZE; i++){
		printf("read[%d] = %u\n", i, read[i]);
	}



    return 0;
}
