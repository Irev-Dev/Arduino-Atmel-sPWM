#include <avr/io.h>
#include <avr/interrupt.h>

#define LookupEntries (512)

static int microMHz = 16; // Micro clock frequency
static int maxfreq = 60;     // Sinusoidal frequency
static long int period = 1600;   // Period of PWM in clock cycles. 1600 gives 10KHz.
static unsigned int lookUp[LookupEntries];
static char theTCCR1A = 0b10000010; //varible for TCCR1A
static unsigned long int phaseinc;
static double phaseincMult;

void setup(){
  double temp; //Double varible for <math.h> functions.

  for(int i = 0; i < LookupEntries; i++){ // Generating the look up table.
    temp = sin(i*M_PI/LookupEntries)*period;
    lookUp[i] = (int)(temp+0.5);       // Round to integer.    
  }
  //1024/1023*2^23/1e6 = 8.39680800782014
  phaseincMult = (period*maxfreq*8.39680800782014/microMHz);
  Serial.begin(9600);
  Serial.println(phaseincMult);
  
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
  pinMode(13, OUTPUT); // Set trigger pin to output
}

void loop(){
  int sensorValue = analogRead(A0);
  static int sensorValue2;
  if(sensorValue > sensorValue2*1.01 || sensorValue < sensorValue2*0.99){
    sensorValue2 = sensorValue;
    phaseinc = (unsigned long int)(phaseincMult*sensorValue2);

    Serial.print(phaseinc>>23);
    Serial.print(".");
    Serial.print(phaseinc&0x007FFFFF);
    //Serial.print(phaseinc<<9);
    Serial.print("\n");   
  }
}

ISR(TIMER1_OVF_vect){
  static unsigned long int phase, lastphase;
  static char delay1, trig;

  phase += phaseinc;

  if(delay1 == 1){
    theTCCR1A ^= 0b10100000;// Toggle connect and disconnect of compare output A and B.
    TCCR1A = theTCCR1A;
    delay1 = 0;  
  } 
  else if((phase>>31 != lastphase>>31) && !(phase>>31)){
    delay1++;      
    trig ^=0b00000001;
    digitalWrite(13,trig);
  }
  
  lastphase = phase;
  OCR1A = OCR1B = lookUp[phase >> 23];
}

