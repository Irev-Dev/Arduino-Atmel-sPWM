# Arduino-Atmel-sPWM 

#### Implementation of an sPWM signal on Ardunio and Atmel micros

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/pulses_1.JPG?raw=true "Figure")

## Introduction

The aim of this repo is to help the hobbyist or student make rapid progress in implementing an sPWM signal on a arduino or atmel micro, while making sure that the theory behind the sPWM and the code itself is understood. 

Please also note that:

 * It's assumed the reader has a basic understanding of C programming
 * If you plan on making an inverter please read the safety section
 * Feel free to colaborate on improving this repo


## Table of Contents
<!-- toc -->
* [Brief Theory](#brief-theory)
    - [Basic PWM](#basic-pwm)
    - [Typical micro implementation](#typical-microcontroller-pwm-implementation)
    - [Sinusoidal PWM](#sinusoidal-pwm)
* [Code & Explanation](#code-and-explanation)
    - [sPWM_Basic](#spwm_basic)
    - [sPWM_Generate_Lookup_Table](#spwm_generate_lookup_table)
* [Testing the Signal](#testing-the-signal)
    - [Viewing Pulse Widths with an Osilloscope](#viewing-pulse-widths-with-an-osilloscope)
    - [Viewing the Filtered Signal with an Oscilloscope](#viewing-the-filtered-signal-with-an-oscilloscope)
    - [Listening to the Signal](#listening-to-the-signal)
* [Compatibility](#compatibility)
* [Safety](#safety)

<!-- tocstop -->

## Brief Theory
###Basic PWM

Pulse width modulation’s (PWM) main use is to control the voltage supplied to electric circuits, it does this by rapidly switching a load on and off. Another way of thinking of it is to consider it as a method for a digital system to output an analogue signal. The figure below shows an example of a PWM signal.

![Figure 1-1](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/basicPWM_3.png?raw=true "Figure 1.1")

There are two properties to a PWM signal, the frequency which is determined by the period of the signal and the duty cycle which is determined by the high-time of the signal. The signal in figure above has a period of 250μS which means it switches at 4KHz. The duty-cycle is the percent high time in each period, in the last figure the duty-cycle is 60% because of the high-time and period of 150μS and 250μS respectively. It is the duty-cycle that determines average output voltage. In figure above the duty-cycle of 60% with 5V switching voltage results in 3V average output as shown by the red line. After filtering the output a stable analogue output can be achieved. the figure below shows PWM signals with 80% and 10% duty-cycles. By dynamically changing the duty-cycle, signals other than a flat output voltage can be achieved.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/basicPWM_4.png?raw=true "Figure")

### Typical microcontroller PWM implementation

This section describes how micro-controllers use timers/counters to implement a PWM signal, this description relies heavily on the figure below. Here the blue line represents a counter that resets after 16000, this gives the period of the PWM and also the switching frequency (fs), if this micro has a clock source of 16MHz, then fs will be 16×10^6/16×10^3 = 1KHz.

The two other values of 11200 and 8000 shown as the green and red line, they represent compare output values of the microcontroller. When the counter reaches this values the microcontroller can change the output of some pins. In this case two pins are set to HIGH when the counter resets and then Set low when the counter reaches each of the output compare values. This determines the high time and therefore duty-cycle of the PWM. It's important to understand how a PWM is typcialy set up in a microcontroller, as the explanation of the code in the next section will not make sense otherwise.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/sawtooth_counter_1.png?raw=true "Figure")

### Sinusoidal PWM

A sinusoidal PWM (sPWM) signal can be constructed by dynamically changing the duty- cycle. The result is short pulses at the zero-crossings and long pulses at the wave peaks. This can be seen in the figure below.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/PWMsin_2.png?raw=true "Figure")

The figure shows negative pulses which is not possible on most micro-controllers. Instead normally this is implemented with two pins, one pulsing the positive half of the sin wave and the second pulsing the negative half, this is how it is implemented in this repo as having the signal split across two pins makes sense if the signal is going to be used to control a H-bridge.

## Code and Explanation

In this chapter we'll step through the code found in the folder sPWM_basic, and then the difference between it and the code found in the folder sPWM_generate_lookup_table will be discussed. Code found in sPWM_atmel is for use on an atmel chip without using the arduino IDE.The code toggles a pin for every period of the sine output in order to make it osilliscope friendy. 

### sPWM_Basic

The following C code implements an sPWM on a Atmel micro-controller. The signal is generated on two pins, one responsible for the positive half of the sine wave and the other pin the negative half. The sPWM is generated by running an (ISR) every period of the PWM in order to dynamically change the duty-cycle. This is done by changing the values in the registers OCR1A and OCR1B from values in a look up table. There are two look up tables for each of the two pins, lookUpTable1 and lookUpTable2 and both have 200 values. The first half of lookUpTable1 has sin values from 0 to π and the second half is all zeroes. The first half of lookUpTable2 is all zeroes and the second half has sin values from 0 to π as shown in figure below.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/lookup_2.png?raw=true "Figure")

The code assumes implementation on a Arduino Uno but is likely compatable with other boards. see Compatability.

Lets walk through the code.

```c
#include <avr/io.h>
#include <avr/interrupt.h>

// Look up tables with 200 entries each, normalised to have max value of 1600 which is the period of the PWM loaded into register ICR1.
int lookUp1[] = {50 ,100 ,151 ,201 ,250 ,300 ,349 ,398 ,446 ,494 ,542 ,589 ,635 ,681 ,726 ,771 ,814 ,857 ,899 ,940 ,981 ,1020 ,1058 ,1095 ,1131 ,1166 ,1200 ,1233 ,1264 ,1294 ,1323 ,1351 ,1377 ,1402 ,1426 ,1448 ,1468 ,1488 ,1505 ,1522 ,1536 ,1550 ,1561 ,1572 ,1580 ,1587 ,1593 ,1597 ,1599 ,1600 ,1599 ,1597 ,1593 ,1587 ,1580 ,1572 ,1561 ,1550 ,1536 ,1522 ,1505 ,1488 ,1468 ,1448 ,1426 ,1402 ,1377 ,1351 ,1323 ,1294 ,1264 ,1233 ,1200 ,1166 ,1131 ,1095 ,1058 ,1020 ,981 ,940 ,899 ,857 ,814 ,771 ,726 ,681 ,635 ,589 ,542 ,494 ,446 ,398 ,349 ,300 ,250 ,201 ,151 ,100 ,50 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0};
int lookUp2[] = {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,50 ,100 ,151 ,201 ,250 ,300 ,349 ,398 ,446 ,494 ,542 ,589 ,635 ,681 ,726 ,771 ,814 ,857 ,899 ,940 ,981 ,1020 ,1058 ,1095 ,1131 ,1166 ,1200 ,1233 ,1264 ,1294 ,1323 ,1351 ,1377 ,1402 ,1426 ,1448 ,1468 ,1488 ,1505 ,1522 ,1536 ,1550 ,1561 ,1572 ,1580 ,1587 ,1593 ,1597 ,1599 ,1600 ,1599 ,1597 ,1593 ,1587 ,1580 ,1572 ,1561 ,1550 ,1536 ,1522 ,1505 ,1488 ,1468 ,1448 ,1426 ,1402 ,1377 ,1351 ,1323 ,1294 ,1264 ,1233 ,1200 ,1166 ,1131 ,1095 ,1058 ,1020 ,981 ,940 ,899 ,857 ,814 ,771 ,726 ,681 ,635 ,589 ,542 ,494 ,446 ,398 ,349 ,300 ,250 ,201 ,151 ,100 ,50 ,0};

```

We are going to be adressing the registers on the atmel chip as well as using interrupts so the <avr/io.h> and <avr/interrupt.h> headers are necessary. From there we have two arrays which have a two half sinusoidal signal entered in

```c
void setup(){
    // Register initilisation, see datasheet for more detail.
    TCCR1A = 0b10100010;
       /*10 clear on match, set at BOTTOM for compA.
         10 clear on match, set at BOTTOM for compB.
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
```
Here the timer register have been initilised. If you are interested in the details you can look up the ATMEGA328p datasheet, but for now what's important is that we have set up a PWM for two pins and it call in interrupt routine for every period of the PWM.

```c
    ICR1   = 1600;     // Period for 16MHz crystal, for a switching frequency of 100KHz for 200 subdevisions per 50Hz sin wave cycle.
    sei();             // Enable global interrupts.
    DDRB = 0b00000110; // Set PB1 and PB2 as outputs.
    pinMode(13,OUTPUT);
}
```
ICR1 is another register that contains the length of the counter before resetting, since we have no prescale on our clock, this defines the period of the PWM to 1600 clock cycles. Then we enable interrupts. Next the two pins are set as outputs, the reason why pinMode() is not used is because the pins might change on different arduinos, they might also change on different Atmel micros, however you are using an arduino with a 328p, then this code will work. Lastly pinMode() is used to set pin 13 as an output, we will use this later as a trigger for the osilliscope, how ever it is not necessary.

```c
void loop(){; /*Do nothing . . . . forever!*/}
```
Nothing is implemented in the loop.

```c
ISR(TIMER1_OVF_vect){
    static int num;
    static char trig;
    // change duty-cycle every period.
    OCR1A = lookUp1[num];
    OCR1B = lookUp2[num];
    
    if(++num >= 200){ // Pre-increment num then check it's below 200.
       num = 0;       // Reset num.
       trig = trig^0b00000001;
       digitalWrite(13,trig);
     }
}
```
This interrupt service routine is call every period of the PWM, and every period the duty-cycle is change. This done by changing the value in the registers OCR1A and OCR1B to the next value of the look up table as this registers hold the compare values that set the output pins low when reached as per figure .

Therefore in each period the registers OCR1x are loaded with the next value of their look up tables by using num to point to the next value in the array, as each period num is incremented and checked that it is below 200, if it is not below 200 in is reset to 0. The other two lines involving trig and digitalWrite are there two toggle a pin as a trigger for an osilloscope and does not impact the sPWM code.

### sPWM_generate_lookup_table

The rest of this section discusses some modifications to this code, namely we can make generate the lookup table at the start of the code, the benifits of this is we change change the switching frequency as well as the sPWM frequency. Code for this can be found in the sPWM_generate_lookup_table folder. The start of the code looks like this:

```c
#include <avr/io.h>
#include <avr/interrupt.h>

#define SinDivisions (200)// Sub divisions of sisusoidal wave.

static int microMHz = 16; // Micro clock frequency
static int freq = 50;     // Sinusoidal frequency
static long int period;   // Period of PWM in clock cycles.
static unsigned int lookUp[SinDivisions];
static char theTCCR1A = 0b10000010; //varible for TCCR1A

void setup(){
  double temp; //Double varible for <math.h> functions.
  
  period = microMHz*1e6/freq/SinDivisions;// Period of PWM in clock cycles
  
  for(int i = 0; i < SinDivisions/2; i++){ // Generating the look up table.
    temp = sin(i*2*M_PI/SinDivisions)*period;
    lookUp[i] = (int)(temp+0.5);       // Round to integer.    
  }
```
Notice that only the first half of the sine wave is generated, because of the way this code implements the sPMW where each of the two pins are responisble for different halves of the signal, only half the sine wave is needed. However it does require a modification to the interrupt service routine.

```c
ISR(TIMER1_OVF_vect){
    static int num;
    static int delay1;
    static char trig;
    
    if(delay1 == 1){/*delay by one period because the high time loaded into OCR1A:B values are buffered but can be disconnected immediately by TCCR1A. */
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
    num++;
}
``` 
Both output compare registers ORC1x have the same values loaded into them each period, however the output compare for each pin is enabled and disable in turns by toggling two bits in the TCCR1A resgister. It is toggle each time the look up table index (num) is reset, however it is delayed by one clock cycle, this is because when values are loaded into OCR1x registered, is is buffered where as changes in TCCR1A is implemented imediately, see the 328p datasheet for more details.

## Testing the Signal

This chapter discusses ways to test and monitor the sPWM signal, the first section discusses using as oscilloscope which is the best method to verify the signal however an alternate and cheaper method is to use a small speaker. The 50hz signal and the underlying switching frequency is easy to hear.

### Viewing Pulse Widths with an Osilloscope

Here we aim to view the individual pulses that make up the sPWM to see if they look how we would expect, that is thin pulses becoming thick and then thin again. If you are using the code sPWM_generate _lookup_table I recommend lowering the switching frequency of the sPWM by changing #define SinDivisions(200) to #define SinDivisions (50). This will make the pulses easier to see as there will be fewer of them. If you are using the Uno pin 13 will be toggled every period of the sine and so can be used as a trigger for the osilloscope. the figure below shows the wiring to the Ardurino Uno.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/PulseOscill_1.png?raw=true "Figure")

Notice we are only looking at half the signal from one of the output pins. The wave form produced should look like the figure below. Notice the change from thin to thick back to thin pulses. 

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/pulses_2.png?raw=true "Figure")

### Viewing the Filtered Signal with an Oscilloscope

Here we aim to view a smooth sinusoidal wave by smoothing the output of the micro with a simple low-pass RC filter as shown in Figure 4.3. 

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/filterOscill_2.png?raw=true "Figure")

The purpose of the filter is to remove the much higher switching frequency (fs) and leave only the 50Hz sine wave. The cut-off frequency (fc) of the filter determined is by fc = 1/2πRC, and it should lie somewhere between fs and 50Hz, closer to 50Hz is more optimal. fs is determined by the line of code #define SinDivisions (a number) in sPWM_generate_lookup_table. The switching frequency is given by fs = SinDivisions×50Hz. Using an arbitrary number for SinDivisions may produce unexpected results, I recommend using 50,200 and 400 to produced fs’s of 2.5, 10 and 20 KHz respectively. The filtered signal is shown in the figure below.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/smoothed_2.png?raw=true "Figure")

Alternatively a potentiometer can be used as a variable resistor as shown in the figure below.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/trimpot_RCfilter_2.png?raw=true "Figure")

 This allows the fc to be varied. The followwing five figures show the result of changing the potentiometer and therefore the amount of filtering.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/smothing_1.png?raw=true "Figure")

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/smothing_2.png?raw=true "Figure")

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/smothing_3.png?raw=true "Figure")

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/smothing_4.png?raw=true "Figure")

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/smothing_5.png?raw=true "Figure")

### Listening to the Signal

If you don’t have an oscilloscope, listening to the signal is a useful way to determine if the sPWM is working since it is easy to hear both the 50Hz hum and the switching frequency. We can use the micro to drive a small pair of head-phones directly, putting them in series with a 1KΩ resister should protect most head-phones, as seen in Figure 4.6.

![Figure what](https://github.com/Terbytes/Arduino-Atmel-sPWM/blob/master/im/speaker_2.png?raw=true "Figure")

It is recommended to change #define SinDivisions (200) to 50, 200 and 400 in order to hear the difference in switching frequencies. Note depending on your age and hearing you might not be able to hear the switching frequency with #define SinDivisions (400), as this produces a switching frequency of 20KHz, which is on the limit of human hearing.

## Compatibility

Please let me know if you got the code to work on a device that's not listed here

Compatability list:

* Arduino Uno
* Arduino Nano
* Arduino mega2560

## Safety

This section is to briefly discuss safety in regards to making an inverter that steps up to  mains voltage whether that be 110 or 230. First of all I don't encourage it, I'd prefer that you didn't and I take no responsibilty for your actions. Remember 30mA can be leathal, mains voltage deserves respect.

If you still choose to do so, take basic precautionary steps like: Invest in some terminals and make sure that any high voltage part of the circuit is not touchable; don't modify it while it's power up; Don't do it alone.
