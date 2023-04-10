// Velocity PID Control using TTMotor with Photoelectric Single Phase Encoder
// board:  adafruit motor shield V1 with Uno
// please see Curio Res for PID and 25 HZ Filter https://github.com/curiores/ArduinoTutorials

#include <util/atomic.h>
#include <AFMotor.h>

// Pins
#define ENCA 2
// Pins
#define BTN A1
#define VRY A3

// select motors
AF_DCMotor motor_L(2);

// globals
long prevT = 0;
int posPrev = 0;
unsigned long prevTime = 0;  // for step function generator
const int valY_center = 530;  // measure the nominal reading of the joystick in the neutral position
int deltaY = valY_center;  // joystick value
int dirMotorL = FORWARD;
int pos1 = 0;

// Use the "volatile" directive for variables used in an interrupt
volatile int pos_a = 0;

// filter variables
float v1Filt = 0;    //filtered velocity value
float v1Prev = 0;  //prev filtered velocity value

//PID integrator variable
float eintegral = 0;

// Motor stall pwm
#define STALL 30

void setup() {
  Serial.begin(115200);

  pinMode(ENCA, INPUT);
  pinMode(VRY, INPUT);
  pinMode(BTN, INPUT_PULLUP);

  // attach photointerrupter - note that using Rising, Falling or Change makes no difference in counts
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoderA, CHANGE);

  // turn on motor
  motor_L.setSpeed((int)(0));
  motor_L.run(RELEASE);

  // declare function prototypes (optional)
  void runMotorL(double , int );
  void readEncoderA();

}

void loop() {

  const int full_range = 1024;  // Uno analog input resolution
  const int deadBand = 30;  // measure how noisy the joystick in the neutral position, then set this large enough to ingore.
  const int Tsample = 80;    //ms  set this as small as possible to achieve a decent number of counts to get a velocity reading.
  const int maxSpeed = 200;  // max motor driver pwm input (motor saturates in rpm at about 190)

  // read the position in an atomic block
  // to avoid potential misreads
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pos1 = pos_a;
  }

  long currT = millis();
  unsigned long elapsedTime = currT - prevT;

  if (elapsedTime >= Tsample) {

    // read Joystick Y position - uncomment to use joystick instead of step function
    deltaY = abs(analogRead(VRY));

    // overide joystick to test a step function - comment out to use joystick values
    //        unsigned long elapsedStepTime = currT - prevTime;
    //        if (elapsedStepTime > 5000) {
    //          deltaY = deltaY == valY_center ? 800 : valY_center;
    //          prevTime = currT;
    //        }

    // motor speed from Y value
    int motorSpeedL = 0, motorSpeedR = 0;
    if (deltaY < (valY_center - deadBand)) {
      motorSpeedL = map(deltaY, 0, valY_center, -maxSpeed, -STALL);
    } else if (deltaY > (valY_center + deadBand)) {
      motorSpeedL = map(deltaY, valY_center, full_range, STALL, maxSpeed);
    }

    dirMotorL = motorSpeedL < 0 ? BACKWARD : FORWARD;  //LEFT MOTOR_CH_A CCW IS FORWARD

    // limit max PWM command
    motorSpeedL = abs(motorSpeedL);
    if (motorSpeedL > maxSpeed) motorSpeedL = maxSpeed;

    // Compute motor velocity from single phase encoder
    // speed measured from the encoder was verified with a tach

    int PPR = 40;  // encoder changes per rev.  Encoder wheel has only 20 slots, but creates 40 counts per revolution.
    float deltaT = ((float)(currT - prevT)) / 1e3;  // ms to sec
    prevT = currT;

    // Convert count/s to RPM
    /*                                [ counts / (counts / rev) ]
           Speed (rpm) =  ------------------------------------------  = rev / m
                                       [ T.s(ms)*1m/60000ms ]                               */

    float v1 = (pos1 * 60000) / (PPR * Tsample);

    // Low-pass filter (25 Hz cutoff)
    v1Filt = 0.854 * v1Filt + 0.0728 * v1 + 0.0728 * v1Prev;
    v1Prev = v1;

    // Compute the control signal u
    float kp = 1.4;
    float ki = 1.0;
    float e = motorSpeedL - v1Filt;
    eintegral = eintegral + e * deltaT;

    float u = kp * e + ki * eintegral;

    double pwr = (double)fabs(u);
    if (pwr > 255) {
      pwr = 255;
    }

    int PWM_L = pwr;  //new variable is not needed, just done to help me understand what is going on.

    // if pmw command is lower than stall, just set to zero
    if (motorSpeedL > -STALL && motorSpeedL < STALL) PWM_L = 0;

    runMotorL(PWM_L, dirMotorL);  // Left Motor

    // Output to plotter
    Serial.print(250);
    Serial.print(" ");
    Serial.print(0);
    Serial.print(" ");
    Serial.print("TGT: ");
    Serial.print(motorSpeedL);   //target speed
    Serial.print(" ");
    Serial.print("SPD: ");
    Serial.print(v1Filt);  //measured speed
    //    Serial.print(" ");
    //    Serial.print("VRY: ");  // use to measure joystick nominal and noise readings in zero position
    //    Serial.print(deltaY);
    //    Serial.print(" ");
    //    Serial.print("CNT: ");
    //    Serial.print(pos1);  //encoder counts ranges from 2 at min speed t 10 at max speed
    //    Serial.print(" ");
    //    Serial.print("ERR: ");
    //    Serial.print(e);
    //    Serial.print(" ");
    //    Serial.print("PWM: ");
    //    Serial.print(PWM_L);
    Serial.println();

    // Reset encoder count
    pos_a = 0;
  }
}

void runMotorL(double output, int direction) {
  motor_L.setSpeed(output);
  motor_L.run(direction);
}

void readEncoderA() {
  pos_a++;
}
