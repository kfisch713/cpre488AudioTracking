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
#include <math.h>

char *SWs = (char *)XPAR_SWS_8BITS_BASEADDR;
char *BTNs = (char *)XPAR_BTNS_5BITS_BASEADDR;

#define KILL_SWITCH 7
#define MIC_SPACING 0.010
#define SPEED_OF_SOUND 343.2
#define tau MIC_SPACING / SPEED_OF_SOUND

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

int * SLV_REG0(unsigned int x){
	return ((int *)((XPAR_AXI_PMIC_0_BASEADDR) + (x * 4)));
}

int * SLV_REG1(unsigned int x){
	return ((int *)((XPAR_AXI_PMIC_1_BASEADDR) + (x * 4)));
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

int sleep_read_spi()
{
  XTime tEnd, tCur;

  XTime_GetTime(&tCur);

  tEnd  = tCur + 1;
  do
  {
    XTime_GetTime(&tCur);
  } while (tCur < tEnd);


  return 0;
}

void read_mic(uint16_t* mic0, uint16_t* mic1){
	*SLV_REG0(1) = 0;
	*SLV_REG1(1) = 0;
	//sleep_read_spi();
	*SLV_REG0(1) = 1;
	*SLV_REG1(1) = 1;

	*mic0 = *SLV_REG0(0);
	*mic1 = *SLV_REG1(0);

	return;
}

int main()
{
    init_platform();
    uint16_t adc_val0, adc_val1, old_adc_val0, old_adc_val1;
    volatile XTime t1, t0;
    unsigned long int i = 0;
    uint8_t look_for_peek_0 = 0, look_for_peek_1 = 0;
    double freq0;
    double angle = 0;
    double intermediate = 0;

    XTime_GetTime(&t0);
    XTime_GetTime(&t1);

    *SLV_REG0(1) = 1;
    *SLV_REG1(1) = 1;


    read_mic(&old_adc_val0, &old_adc_val1);

    while(!SW(KILL_SWITCH)) {
    	if(SW(0)) i=0;
    	read_mic(&adc_val0, &adc_val1);


    	//upward trend for mic 0
    	if(old_adc_val0 < adc_val0){
    		look_for_peek_0 = 1;
    	}
    	//downward trend for mic 0
    	if(look_for_peek_0){
    		if(old_adc_val0 > adc_val0){
    			look_for_peek_0 = 0;
    			XTime_GetTime(&t0);
    		}
    	}

    	//upward trend for mic 0
		if(old_adc_val1 < adc_val1){
			look_for_peek_1 = 1;
		}
		//downward trend for mic 0
		if(look_for_peek_1){
			if(old_adc_val1 > adc_val1){
				look_for_peek_1 = 0;
				XTime_GetTime(&t1);
			}
		}

		//only want to calculate after both mic see a peak
		if (look_for_peek_1 == 0 && look_for_peek_0 == 0){
			intermediate = (t1-t0)/COUNTS_PER_SECOND * 343.0 / 0.1;
			angle = asin( intermediate );
		}


    	//xil_printf("adc val = %5d, reg1 = %5d, reg2 = %5d\n", adc_val, *SLV_REG(1), *SLV_REG(2));
    	printf("mic0 = %5d, mic1 = %5d, angle = %6.2lf, diff = %llu, %llu, %llu, intermediate = %lf\n", adc_val0, adc_val1, angle, t1-t0, t1, t0, intermediate);

    	//Use this to print to .csv
    	//xil_printf("%d, %d, %d\r\n", i++, adc_val0, adc_val1);

    	old_adc_val0 = adc_val0;
    	old_adc_val1 = adc_val1;
    }

    return 0;
}


