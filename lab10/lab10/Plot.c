/** Plot.c
 * Plot for data acquisition system.
 * Motor plot and user interface.
 * 
 * Author: Ronald Macmaster
 * Date: 11/07/2016
 */
 
 #include <stdint.h>
 #include <stdbool.h>
 #include <stdio.h>
 #include "ST7735.h"
 #include "Timer.h"
 #include "Switch.h"
 
 // helper functions
 static void PrintFixed(uint16_t n);
 static uint16_t map(uint16_t data);
 
 /**************Plot_Init()***************
 Specify the plot axes and range of LCD plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          min   smallest data value allowed, resolution= 0.01
          max   largest data value allowed, resolution= 0.01
 Outputs: none
*/
static int32_t min = 0, max = 0; 
void Plot_Init(char *title, int32_t minData, int32_t maxData){
	// setup data and timer
	min = map(minData); max = map(maxData);
	Timer1A_Init(TIMER_1Hz, 0x02);
	Keypad_Init();
	
	// clear screen.
  ST7735_FillScreen(0);
  ST7735_SetCursor(0, 0);
	ST7735_PlotClear(min, max); 

  // print plot axes
  printf("%s \n", title);
	Timer1A_Start();
}
 
/** Plot_PrintSpeed()
 * Print the data value in fixed point
 * Special place right above plot for data string
 */
void Plot_PrintSpeed(uint16_t desired, uint16_t actual){
	// map input
	static uint8_t idx = 0;
	desired = map(desired);
	actual = map(actual);
	
	// print desired
	ST7735_SetCursor(0, 1);
	printf("desired: ");
	PrintFixed(desired);
	printf(" rps  ");
	
	// print actual
	ST7735_SetCursor(0, 2);
	printf("actual: ");
	PrintFixed(actual);
	printf(" rps  ");
	idx = idx + 1;
}

/** Plot_PlotSpeed()
 * Plots the next data point in the sequence.
 * plot pointer is incremented one place
 */
void Plot_PlotSpeed(uint16_t speed){
	speed = map(speed);
	ST7735_PlotPoint(speed);
	ST7735_PlotNextErase();
	ST7735_SetCursor(0, 3);
	PrintFixed(max);
	ST7735_SetCursor(0, 15);
	PrintFixed(min);
}

/****************PrintFixed(uint16_t n)***************
 converts fixed point number to LCD
 format unsigned 16-bit with resolution 0.1
 range 0 to +500.0
 Inputs:  unsigned 16-bit integer part of fixed-point number
 Outputs: none
 Parameter LCD display
 12345    "*.**"
  2345    "234.5"
  2100    "210.0"
   120    "12.0"
    31    "3.1"
-12345    "*.*"
 ************************************************/
static void PrintFixed(uint16_t n){
  const uint16_t max = 5000;
  //const uint16_t min = 0000;

  // invalid number
  if(/*n < min || */n > max){
    ST7735_OutString("*.*");
    return;
  }

	// build and print the number
	uint8_t idx = 0;
	char num[6] = {'\0'};
	// n >= 100.0
	if((n / 1000) % 10 > 0){ // hundred's digit
		num[idx++] = ((n / 1000) % 10) + '0';
		if((n / 100) % 10 > 0){ // ten's digit
			num[idx++] = ((n / 100) % 10) + '0';
		}
	} 
	// 10.0 <= n <= 100.0
	else if((n / 100) % 10 > 0){ // ten's digit
			num[idx++] = ((n / 100) % 10) + '0';
	} 
	
	// ones and tenth's digits
	num[idx++] = ((n / 10) % 10) + '0';
	num[idx++] = '.';
	num[idx++] = ((n / 1) % 10) + '0';
	num[idx++] = '\0';
	ST7735_OutString(num);
}


/** map()
 * Map an ADC data input to a temperature value
 * Map's LUT is generated by therm12.xls
 * Temperature vaules are in decimal fixed point with 0.01 resolution
 * return : Temperature fixed point value
 */
static uint16_t map(uint16_t data){
	return data;
}
//extern const uint16_t MAP_SIZE;
//extern uint16_t const ADCdata[];
//extern uint16_t const Tdata[];
//static uint16_t map(uint16_t data){
//	uint8_t idx = MAP_SIZE / 2, jdx = MAP_SIZE;
//	bool found = false;
//	while(!found){
//		if(data >= ADCdata[idx] && data <= ADCdata[idx + 1]){
//			// linear interpolation
//			uint16_t value = Tdata[idx] - Tdata[idx + 1];
//			value = value * (data - ADCdata[idx]);
//			value = value / (ADCdata[idx + 1] - ADCdata[idx]);
//			value = Tdata[idx] - value;
//			return value;
//		} else if(data > ADCdata[idx]){
//			idx = (jdx + idx) / 2;
//		} else if(data < ADCdata[idx]){
//			jdx = idx;
//			idx = (idx) / 2;
//		} 
//	} 
//	// error / search failure
//	return 0x0000;
//}


extern bool redraw;
extern uint16_t desired;
void Timer1A_Handler(){
	Timer1A_Acknowledge();
	// sample and debounce the keypad
	static uint32_t keypad, last = 0x00;
	keypad = Keypad_Read(); 
	redraw = true;
	
	// update motor setting
	const uint16_t ds = 500;
	if(keypad != last){
		// increase desired speed
		if(keypad == 0x01){
			if(desired + ds < max){
				desired = desired + ds;
			} else {
				desired = max;
			}
		} 
		// decrease desired speed
		else if(keypad == 0x02){
			if(desired > min + ds){
				desired = desired - ds;
			} else {
				desired = min;
			}
		}
	}
}

