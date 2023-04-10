# TTMotorPIDVelocity
Velocity Control using a TTMotor and photo-interrupt wheel encoder.
![PIDVelocitySetup-small (Small)](https://user-images.githubusercontent.com/31633408/230807965-06dc9ff9-834a-450d-8f99-d02dc746b178.jpg)

Adapts the PID Velocity work of Curio Res (https://github.com/curiores/ArduinoTutorials) for use on a single phase photo-interrupter with slotted wheel insted of an A/B magnetic encoder.

This projects adapts the PI closed loop code and the signal filter to control a TTmotor velocity with a joystick input.

## Hardware
* Joystick from Mouser Electronics:  611-THB001P
* Arduino Uno - generic from Aliexpress
* Adafruit V1 motor shield - generic from Aliexpress
* TTmotor - generic from Aliexpress
* 9v rechargable battery - TalentCell 500mAh from Amazon

## Wiring
### Encoder
* VCC to Arduino 5v
* GND to Arduino Ground
* Output to Arduino Pin 2

### Joystick
* VCC to Arduino 5v
* GND to Arduino Ground
* Y to Arduino Pin A3
* X to Arudino Pin A2 (not used in this example)
* BTN to Arduino Pin A1 (not used in this example)

### TTmotor
* Connected to M2 terminal

## Overview
Every 80 milliseconds the following actions occur:
1)  Read the current count from the encoder
2)  Read the Joystick Y axis input
3)  Convert the Joystick input into a velocity command and direction
4)  Compute the current speed of the motor using the number of encoder counts and elapse time
![image](https://user-images.githubusercontent.com/31633408/230810741-ed3456eb-4c98-4d36-bfc4-327f3b1e5607.png)
5)  Filter the speed using a 25Hz low pass filter (see Curio Res reference)
6)  Compute the control signal using a PI filter
7)  Use the absolute value of the control signal as the PWM input to the motor driver speed input
8)  Use the joystick command direction as input to the motor driver direction input
9)  Reset the counter to zero

Issues
Overall the PI loop controls the motor velocity fairly well.  However there are challenges.
*  The encoder count is very low therefore the speed measurment resolution is very low.  Small variations in count make a large differnce in measured speed. The measured velocity is fixed to the number of counts per sample period.  The sample period was chosed to allow at least one count at the lowest desired velocity of 40 RPM.  At 80ms sample time, the resolution is 18.75 RPM.  2 counts per sample time equates to 37.5 RPM.  If the target velocity is set at 40 RPM, the feedback velocity will settle at between 2 to 3 counts per sample period or 37.5 to 56.25RPM.  So there will be a lot of velocity jitter which makes control difficult.  The filter helps signicantly in averaging the jitter out, but performance at low speed is still jittery.
*  The typical aduino joystick has a high gain and then saturates very quickly.  The readings seem to hit the limits of 0 or 1024 at ~70% of joystick travel in either direction.  The joystick therefore is hard to keep from going full throttle in use.  The Mouser electronics joystick is much more linearly in that the min/max A2D values are not hit until the ends of the joystick travel.  The deadband issue is not resolved, but this still gives a much easier experience in commanding a speed between the limits.  The joystick does not come mounted to a PCB but it is not difficult to design a simple PCB which takes the signals to a connector.

