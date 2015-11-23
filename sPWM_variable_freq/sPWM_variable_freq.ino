#include <avr/io.h>
#include <avr/interrupt.h>

#define SinDivisions (200)// Sub divisions of sisusoidal wave.

#define LookupEntries (1000)

static int microMHz = 16; // Micro clock frequency
//static int LookupEntries = 1000;
static int maxfreq = 50;     // Sinusoidal frequency
static long int period;   // Period of PWM in clock cycles.
static unsigned int lookUp[512];
static char theTCCR1A = 0b10000010; //varible for TCCR1A
static unsigned int phaseinc;
static float phaseincMult;

void setup(){
  double temp; //Double varible for <math.h> functions.
  
  period = 1600; // Period of PWM in clock cycles
  
  for(int i = 0; i < LookupEntries/2; i++){ // Generating the look up table.
    temp = sin(i*2*M_PI/LookupEntries)*period;
    lookUp[i] = (int)(temp+0.5);       // Round to integer.    
  }
  phaseincMult = microMHz*1e6*LookupEntries
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
  // Set outputs pins.
  DDRB = 0b00000110; // Set PB1 and PB2 as outputs.
  pinMode(13, OUTPUT);
}

void loop(){
  int sensorValue = analogRead(A0);
  phaseinc = period/microMHz*1e6/maxfreq*LookupEntries*sensorValue/1023;
  }

ISR(TIMER1_OVF_vect){
  static float phase;
  static int phaseRound;
  static char delay1;
  static char trig;
 
  phase += phaseinc;

  if(delay1 == 1){
    theTCCR1A ^= 0b10100000;// Toggle connect and disconnect of compare output A and B.
    TCCR1A = theTCCR1A;
    delay1 = 0;  
  } else if(phase >= LookupEntries/2){
    phase -= LookupEntries/2;
    delay1++;      
    trig ^=0b00000001;
    digitalWrite(13,trig);
  }

  phaseRound = (int)(phase+0.5);
  OCR1A = OCR1B = lookUp[phaseRound];

    /*static int num;
    static int delay1;
    static char trig;
    
    if(delay1 == 1){/*delay by one period because the high time loaded into OCR1A:B values are buffered but can be disconnected immediately by TCCR1A. *
      theTCCR1A ^= 0b10100000;// Toggle connect and disconnect of compare output A and B.
      TCCR1A = theTCCR1A;
      delay1 = 0;             // Reset delay1
    } else if(num >= SinDivisions/2){
      num = 0;                // Reset num
      delay1++;
      trig ^=0b00000001;
      digitalWrite(13,trig);
    }
    // change duty-cycle every period.
    OCR1A = lookUp[num];
    OCR1B = lookUp[num];
    num++;*/
}
