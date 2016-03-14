#include <avr/io.h>
#include <avr/interrupt.h>

#define LookupEntries (512)

//Varible declaration
static int microMHz = 16; // Micro clock frequency
static int freq;     // Sinusoidal frequency
static long int period;   // Period of PWM in clock cycles. 1600 gives 10KHz.
static unsigned int lookUp[LookupEntries];
static char theTCCR1A = 0b10000010; //varible for TCCR1A
static unsigned long int phaseinc, switchFreq;
static double phaseincMult;

//Function prototypes
int setFreq(int _freq);
int setSwitchFreq(int sfreq);
void registerInit(void);


void setup(){ 
  Serial.begin(9600);
  setSwitchFreq(10000);  
  setFreq(50);
  registerInit();
}

void loop(){
  /*
  // Un-comment inorder to read from A1 to change the switching frequency, note that if you have a noisey potentiometer the
  // it will disrupt the signal as the lookup table needs to be regenerated everytime the switing frequency is changed
  int sensorValue = analogRead(A1);
  static int sensorValue2;
  if(sensorValue > sensorValue2*1.01 || sensorValue < sensorValue2*0.99){
    sensorValue2 = sensorValue;
    setSwitchFreq(map(sensorValue, 0, 1023, 1000, 19000));
    
    Serial.println(switchFreq);
    Serial.print(phaseinc>>23);
    Serial.print(".");
    Serial.print(phaseinc&0x007FFFFF);
    Serial.print("\n");  
  }*/
  
  // Value read in from A0 changes the frequency of the sine wave
  int sensorValue = analogRead(A0);
  static int sensorValue2;
  if(sensorValue > sensorValue2*1.01 || sensorValue < sensorValue2*0.99){
    sensorValue2 = sensorValue;
    setFreq(map(sensorValue, 0, 1023, 5, 300));
    
    Serial.print(phaseinc>>23);
    Serial.print(".");
    Serial.print(phaseinc&0x007FFFFF);
    Serial.print("\n");   
  }
}

ISR(TIMER1_OVF_vect){
  static unsigned long int phase, lastphase;
  static char delay1, trig = LOW;

  phase += phaseinc;

  if(delay1 > 0){ // toggle the output pins one ISR call after phase has overflowed
    theTCCR1A ^= 0b10100000;// Toggle connect and disconnect of compare output A and B.
    TCCR1A = theTCCR1A;
    delay1 = 0;  
  } 
  else if((phase>>31 != lastphase>>31) && !(phase>>31)){ //if phase has overflowed . . .
    delay1++;      
    trig = !trig;
    digitalWrite(13,trig); // pin can be used as triggre on oscilloscope
  }
  lastphase = phase;
  
  OCR1A = OCR1B = lookUp[phase >> 23];
}

int setFreq(int _freq){
  if(_freq < 0 || _freq > 1000){ // returns 0 if the frequency value is invalid
    return 0;
  } else {
    freq = _freq;
    phaseinc = (unsigned long int) phaseincMult*_freq;
    return 1; // returns 1 if freqency set sucessfully
  }
}

int setSwitchFreq(int sfreq){
  double temp;
  
  if(sfreq <= 0 || sfreq > 20000){
    return 0;
  } else {
    switchFreq = sfreq;
    period = microMHz*1e6/sfreq;
    cli(); //disable global interupts while lookup table is made
    TCCR1A = 0b00000010; //disconnect compare A and B while lookup table is generated
    ICR1   = period;
    for(int i = 0; i < LookupEntries; i++){ // Generating the look up table.
      temp = sin(i*M_PI/LookupEntries)*period;
      lookUp[i] = (int)(temp+0.5);       // Round to integer.    
    }
    //sindevisions*decimalbits/1MHz = 
    //1024*2^23/1e6 = 8,589.934592
    phaseincMult = (double) period*8589.934592/microMHz;
    phaseinc = (unsigned long int) phaseincMult*freq;
    TCCR1A = theTCCR1A; // reconnect compare outputs
    sei(); //re-enable interupts now that table has been made
    return 1;
  }
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
    

