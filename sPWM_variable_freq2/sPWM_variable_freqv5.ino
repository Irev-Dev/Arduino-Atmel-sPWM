#include <avr/io.h>
#include <avr/interrupt.h>

#define LookupEntries (512)

static int microMHz = 16;   // clock frequency in MHz
static int freq, amp = 1024;// Sinusoidal frequency
static long int period;     // Period of PWM in clock cycles. 1600 gives 10KHz.
static unsigned int lookUp[LookupEntries];
static char theTCCR1A = 0b10000010; //varible for TCCR1A
static unsigned long int phaseinc, switchFreq;
static double phaseincMult;

int setFreq(int freq);         //set in Hertz
int setSwitchFreq(int sfreq);  //set in Hertz
int setAmp(float _amp);        //set in % (0 - 100)
void makeLookUp(void);
void registerInit(void);

void setup(){ 
  Serial.begin(9600);
  makeLookUp();
  setSwitchFreq(10000);  
  setFreq(50);
  setAmp(100);
  registerInit();
}

void loop(){
/*
The code in the loop reads analog values from pins A1 and A2 so that potentiometers can be connected.
These values are used to vary the amplitude and frequency of the sine wave.
The switching frequency also toggles between 5 and 15Khz.
*/
  static int ampVal, freqVal, anologVal;

  anologVal = analogRead(A0);
  if(anologVal > freqVal*1.01 || anologVal < freqVal*0.99){
    freqVal = anologVal;
    setFreq(map(freqVal, 0, 1023, 5, 300));
    Serial.println("phaseinc");
    Serial.print(phaseinc>>23);
    Serial.print(".");
    Serial.print(phaseinc&0x007FFFFF);
    Serial.print("\n"); 
  }
  
  anologVal = analogRead(A1);
  if(anologVal > ampVal*1.01 || anologVal < ampVal*0.99){
    ampVal = anologVal;
    setAmp(map(ampVal, 0, 1023, 0, 100));
    Serial.println("amplitude");    
    Serial.println(amp);
  }
  
  delay(20);
  static char cnt = 0;
  cnt++;
  if(cnt == 100){
    setSwitchFreq(15000);
    cnt = 0;
  } else if(cnt == 50){
    setSwitchFreq(5000);
  }
}

ISR(TIMER1_OVF_vect){
  static unsigned long int phase, lastphase;
  static char delay1, trig = LOW;

  phase += phaseinc;

  if(delay1 == 1){
    theTCCR1A ^= 0b10100000;// Toggle connect and disconnect of compare output A and B.
    TCCR1A = theTCCR1A;
    delay1 = 0;  
  } 
  else if((phase>>31 != lastphase>>31) && !(phase>>31)){
    delay1++;      
    trig = !trig;
    digitalWrite(13,trig);
  }
  
  lastphase = phase;
  OCR1A = OCR1B = ((lookUp[phase >> 23]*period) >> 12)*amp >> 10;
}

int setFreq(int _freq){
  if(_freq < 0 || _freq > 1000){ // returns -1 if the frequency value is invalid
    return 0;
  } else {
    freq = _freq;
    phaseinc = (unsigned long int) phaseincMult*_freq;
    return 1;
  }
}
    
int setSwitchFreq(int sfreq){
  double temp;
  
  if(sfreq <= 0 || sfreq > 20000){
    return 0;
  } else {
    switchFreq = sfreq;
    period = microMHz*1e6/sfreq;
    //sindevisions*decimalbits/1MHz = 
    //1024*2^23/1e6 = 8,589.934592
    phaseincMult = (double) period*8589.934592/microMHz;
    phaseinc = (unsigned long int) phaseincMult*freq;
    ICR1   = period;
  }
}

int setAmp(float _amp)
{
  if(_amp < 0 || _amp > 100){
    return 0;
  } else {
    amp = map(_amp,0,100,0,1024);
    return 1;
  }  
}

void makeLookUp(void){
  double temp;

    cli(); //disable global interupts while lookup table is made
    TCCR1A = 0b00000010; //disconnect compare A and B while lookup table is generated
    
    for(int i = 0; i < LookupEntries; i++){ // Generating the look up table.
      temp = sin(i*M_PI/LookupEntries)*4096;
      lookUp[i] = (int)(temp+0.5);       // Round to integer.    
    }

    TCCR1A = theTCCR1A; // reconnect compare outputs
    sei(); //re-enable interupts now that table has been made
}

void registerInit(void){
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
  sei();             // Enable global interrupts.
  // Set outputs pins.
  DDRB   = 0b00000110; // Set PB1 and PB2 as outputs.
  pinMode(13, OUTPUT); // Set trigger pin to output 
}
    

