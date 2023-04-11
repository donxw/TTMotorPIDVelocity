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
1)  Read the current encoder count
2)  Read the Joystick Y axis input
3)  Convert the Joystick input into a velocity command and direction
    ![Joystick to RPM](https://user-images.githubusercontent.com/31633408/231224850-81188bd7-6841-4a69-b4b9-39a1d0346bf6.png)
4)  Compute the current speed of the motor using the number of encoder counts and elapse time
![image](https://user-images.githubusercontent.com/31633408/230810741-ed3456eb-4c98-4d36-bfc4-327f3b1e5607.png)
5)  Filter the speed using a 25Hz low pass filter (see Curio Res reference)
6)  Compute error between desired velocity and actual velocity (filtered)
7)  Compute the control signal from the error signal using a PI filter
8)  Use the absolute value of the control signal as the PWM input to the motor driver speed input
9)  Use the joystick direction as input to the motor driver direction input
10)  Reset the encoder count to zero

## Issues

Overall the PI loop controls the motor velocity fairly well.  However there are challenges.
*  The encoder count is very low therefore the speed measurement resolution is poor.  With a sample time (Ts) of 150ms and 40 counts per encoder rev, the speed can only be measured in 10 RPM increments. The Joystick can command with much finer resolution than the encoder can measure velocity which can create dithering.  For example, if the target velocity is set at 55 RPM by the joystick, the feedback velocity is limited to either 5 or 6 discrete counts per sample period (50 or 60 RPM).  Therefore, the contoller will always either sense an under or over speed error and constantly speed up or slow the motor speed.  The average will drive to 55 RPM through oscillating between 50 and 60 RPM.  The filter helps significantly in averaging the encoder speed jitter out, but performance is still jittery.
*  The encoder has a 20 slot wheel, but the interrupt will return a count of 40 for one revolution whether set to rising, falling or change.  Change was used.  The RPM returned by the counts matches what is measured by an external tachometer so using 40 as the number of encoder counts per revolution seems to work.
*  The typical Arduino joystick has a high gain and then saturates very quickly.  The readings seem to hit the limits of 0 or 1024 at ~70% of joystick travel in either direction.  As a result, it is difficult to prevent the joystick from operating at full throttle.  The Mouser electronics joystick is far more linear in that the min/max A2D values are not hit until the ends of joystick travel.  This gives a much easier experience in commanding a speed between the limits.  The joystick does not come mounted to a PCB but it is not difficult to design a simple PCB which takes the signals to a connector.

## Response

Ts = 150ms, K = 1.0, I = 0.9

Step Response

![step_ts150_k1_i_9](https://user-images.githubusercontent.com/31633408/231054976-0abf7c89-bb57-4379-a2d9-0aad7b55de59.png)

Stairstep Response

![stairstep_Ts150_k1_i_9](https://user-images.githubusercontent.com/31633408/231055100-07fd93ab-f8ef-4a86-92eb-e163b8891487.png)

Joystick following

![Joystick_resp_Ts150_K1_i_9](https://user-images.githubusercontent.com/31633408/231055354-737df772-9227-4e8f-b07e-f86d12582173.png)
