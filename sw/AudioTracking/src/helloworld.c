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
#include <stdint.h>
#include "xil_printf.h"
#include "platform.h"
#include "xparameters.h"
#include "xtime_l.h"

char *SWs = (char *)XPAR_SWS_8BITS_BASEADDR;
char *BTNs = (char *)XPAR_BTNS_5BITS_BASEADDR;

#define KILL_SWITCH 7

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

int * SLV_REG(unsigned int x){
	return ((int *)((XPAR_AXI_PMIC_0_BASEADDR) + (x * 4)));
}

int sleep_ms(unsigned int milliseconds)
{
  XTime tEnd, tCur;

  XTime_GetTime(&tCur);
  tEnd  = tCur + (((XTime) milliseconds) * ((1.0/XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ) * 0.001));
  do
  {
    XTime_GetTime(&tCur);
  } while (tCur < tEnd);

  return 0;
}

//781250

int sleep_read_spi()
{
  XTime tEnd, tCur;

  XTime_GetTime(&tCur);

  tEnd  = tCur + 64;
  do
  {
    XTime_GetTime(&tCur);
  } while (tCur < tEnd);


  return 0;
}

uint16_t read_mic(){
	*SLV_REG(1) = 0;
	//XTime_GetTime(&t2);
	//sleep_ms(1);
	sleep_read_spi();
	//XTime_GetTime(&t1);
	*SLV_REG(1) = 1;

	//printf("%llu\n", t1-t2);
	return *SLV_REG(0);
}

int main()
{
    init_platform();
    uint16_t adc_val;
    XTime t1, t2;
    XTime_GetTime(&t1);
    *SLV_REG(1) = 1;

    while(!SW(KILL_SWITCH)) {

    	adc_val = read_mic();

    	xil_printf("adc val = %5d, reg1 = %5d, reg2 = %5d\n", adc_val, *SLV_REG(1), *SLV_REG(2));

//    	if(*SLV_REG(2) > 0) {
//    		xil_printf("done reading from pmic\n");
//    		*SLV_REG(2) = 0;
//    	} else {
//    		*SLV_REG(1) = 1;
//    	}

//    	int i;
//    	for(i=0; i<32; i++){
//    		printf("%2d=%5d, ", i, *SLV_REG(i));
//    	}
//    	printf("\n");

//    	*SLV_REG(1) = 1;
//    	sleep(1);
//    	*SLV_REG(1) = 0;
//    	sleep(1);
    }

    return 0;
}


