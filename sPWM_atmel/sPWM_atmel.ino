
/*
 * sPWMv2.c
 *
 * Created: 31/12/2014
 * Author: Kurt Hutten
 
 sPWM on the for atmel chips. Confirmed compatable with ATmega328, ATmega2560 and the arduino Uno, mega2560.
 Likely compatable with other atmel chips / arduino boards.
 Compare outputs A and B output to PB1 and PB2, which are pins 9  and 10 on the Arduino Uno, respectively.
 Compare outputs A and B output to PB5 and PB6, which are pins 11 and 12 on the Arduino mega2560, respectively.
 Also useful to know the led is:
   PB5 on the Uno.
   PB7 on the mega2560.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#define SinDivisions (50)// Sub divisions of sisusoidal wave.

static int microMHz = 16; // Micro clock frequency
static int freq = 50;     // Sinusoidal frequency
static long int period;   // Period of PWM in clock cycles.
static unsigned int lookUp[SinDivisions];
static char theTCCR1A = 0b10000010; //varible for TCCR1A

int main(void){
  double temp; //Double varible for <math.h> functions.
  
  period = microMHz*1e6/freq/SinDivisions;// Period of PWM in clock cycles
  
  for(int i = 0; i < SinDivisions/2; i++){ // Generating the look up table.
    temp = sin((i+0.5)*2*M_PI/SinDivisions)*period;
    lookUp[i] = (int)(temp+0.5);       // Round to integer.    
  }
  // Register initilisation, see datasheet for more detail.
  TCCR1A = theTCCR1A; // 0b10000010;
        /*10 clear on match, set at BOTTOM for compA.
          00 compB disconected initially, toggled later to clear on match, set at BOTTOM.
          00
          10 WGM1 1:0 for waveform 15.
        */
  TCCR1B = 0b00011001;
        /*000
          11 WGM1 3:2 for waveform 15.
          001 no prescale on the counter.
        */
  TIMSK1 = 0b00000001;
        /*0000000
          1 TOV1 Flag interrupt enable.
        */  
  ICR1   = period;   /* Period for 16MHz crystal, for a switching frequency of 100KHz for 200 subdivisions per 50Hz sin wave cycle. */
  sei();             // Enable global interrupts.
  DDRB = 0b00000110; // Set PB1 and PB2 as outputs.
	
  while(1){; /* Do nothing . . . forever. */}
}

ISR(TIMER1_OVF_vect){
    static int num;
    static int delay1;
    
    if(delay1 == 1){/*delay by one period because the high time loaded into OCR1A:B values are buffered but can be disconnected immediately by TCCR1A. */
      theTCCR1A ^= 0b10100000;// Toggle connect and disconnect of compare output A and B.
      TCCR1A = theTCCR1A;
      delay1 = 0;             // Reset delay1
    } else if(num >= SinDivisions/2){
      num = 0;                // Reset num
      delay1++;
    }
    // change )duty-cycle every period.
    OCR1A = OCR1A = lookUp[num];
    num++;      
}

