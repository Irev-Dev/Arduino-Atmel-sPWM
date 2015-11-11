# Arduino-Atmel-sPWM

#### Implementation of an sPWM signal on Ardunio and Atmel micros

## Introduction

This document covers key concepts in the generation of a sinusoidal signal using dynamic
pulse width modulation (PWM) as well as it’s implementation on Atmel microcontrollers,
including Arduino boards. The code in this document is mostly C with use of Arduino
code/libraries in some cases. This document assumes the reader has a fundamental under-
standing of C programming.
It is the aim of this document to help the hobbyist or student make rapid progress
in understanding and implementing an sPWM signal. If you find any mistakes in this
document or can think of improvements, perhaps you are a university tutor or lecturer,
make a suggestion in the associated forum. Any significant contributors will be listed as an
author

## Key PWM Concepts
###Basic PWM

Pulse width modulation’s (PWM) main use is to control the power supplied to electric
circuits, it does this by rapidly switching a load on and off. Another way of thinking of it
is to consider it as a method for a digital system to output an analogue signal. Figure 1.1
shows an example of a PWM signal.

![Figure 1-1](/images/PWMsin_1.png?raw=true)

There are two properties to a PWM signal, the frequency which is determined by the
period of the signal and the duty cycle which is determined by the high-time of the signal.
The signal in Figure 1.1 has a period of 250μS which means it switches at 4KHz. The duty-
cycle is the percent high time in each period, in Figure 1.1 the duty-cycle is 60% because
of the high-time and period of 150μS and 250μS respectively. It is the duty-cycle that
determines average output voltage. In Figure 1.1 the duty-cycle of 60% with 5V switching
voltage results in 3V average output as shown by the red line. After filtering the output a
stable analogue output can be achieved. Figure 1.2 shows PWM signals with 80% and 10%
duty-cycles.
By dynamically changing the duty-cycle, signals other than a flat output voltage can
be achieved.

### Typical micro-controller PWM implementation

This section describes how micro-controllers use timers/counters to implement a PWM
signal, this description relies heavily on Figure 1.3. Here the blue line represents a counter
that resets after 16000, this gives the period of the PWM and also the switching frequency
(f s ), if this micro had a clock source of 16MHz, then f s would be
16×10 6
16×10 3
= 1KHz. There are
two other values represented by the green and red lines, which determine the duty-cycle.
The values shown are 11200 and 8000 and these give the high time of the PWM signal, and
the duty-cycle is these values as a fraction of the PWM period. Therefore the red PWM
has a duty-cycle of
11200
16000
= 70% and the red PWM has a duty-cycle of
8000
16000
Figure 1.3: PWM signals with 80% and 10% duty-cycles.
= 50%