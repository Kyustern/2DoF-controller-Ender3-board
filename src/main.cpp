#include <Arduino.h>
#include <AccelStepper.h>
#include <pins.h>

#define _LED_BUILTIN_ 27

#define MOTORS_MAX_SPEED 500.0
#define HOMING_SPEED 300.0
#define HOMING_ACCELERATION 100.0

// #define YAW_STEP_PIN 15
#define YAW_STEP_PIN 15
#define YAW_DIR_PIN 21
#define YAW_LIMIT_PIN 18

#define PITCH_STEP_PIN 22
#define PITCH_DIR_PIN 23
#define PITCH_LIMIT_PIN 19

#define XYE_ENABLE 14
#define FAN 5

// 84/18 teeth
const float gear_ratio = 4.666666667;
const float yaw_degs_per_steps = 1.8;
const float pitch_degs_per_steps = 1.8;

const int SAFETY_STEPS = -50;
const float yaw_measured_steps_range =  ((360 * gear_ratio)/1.8) * 2 + SAFETY_STEPS;
const float pitch_measured_steps_range =  ((360 * gear_ratio)/1.8) + SAFETY_STEPS;


AccelStepper YAW_STEPPER(AccelStepper::DRIVER, YAW_STEP_PIN, YAW_DIR_PIN);
AccelStepper PITCH_STEPPER(AccelStepper::DRIVER, PITCH_STEP_PIN, PITCH_DIR_PIN);

//Runtime variables
volatile bool runAllowed = false;
enum HomingState { IDLE, HOMING_YAW, HOMING_PITCH };
enum MotorsEnableState { MOT_ENABLED, MOT_DISABLED };
enum DIRECTION {CW, CCW};

HomingState homingState = IDLE;
bool yawHomed = false;
bool pitchHomed = false;
//Runtime constants

void setMotorsEn(MotorsEnableState desiredState) {
  switch (desiredState)
  {
  case MOT_ENABLED:
    Serial.println("Motors enabled");
    digitalWrite(XYE_ENABLE, LOW);
    break;
    case MOT_DISABLED:
    Serial.println("Motors disabled");
    digitalWrite(XYE_ENABLE, LOW);
    break;
  
  default:
    break;
  }
  
  // if (desiredState == MOT_ENABLED) {
  //   digitalWrite(XYE_ENABLE, LOW);
  // }
  // if (desiredState == MOT_DISABLED) {
  //   digitalWrite(XYE_ENABLE, HIGH);
  // }
      
}

void homeMotor(AccelStepper& stepper, int limitSwitchPin, const char* motorName, DIRECTION direction) {
  stepper.setMaxSpeed(HOMING_SPEED);
  stepper.setAcceleration(HOMING_ACCELERATION);
  
  Serial.print("Homing ");
  Serial.print(motorName);
  Serial.println("...");
  
  // Move towards the limit switch until it reads HIGH
  stepper.move(10000 * (direction == CW ? -1 : 1));
  stepper.setSpeed(HOMING_SPEED);
  while (digitalRead(limitSwitchPin) != HIGH) {
    stepper.run();
    // stepper.runSpeed();
  }
  // delayMicroseconds(400);
  
  // Limit switch reached, set current position to 0
  stepper.setCurrentPosition(0);
  // stepper.setSpeed();
  stepper.move(500 * (direction == CW ? 1 : -1));
  stepper.runToPosition();

  
  stepper.stop();
  
  Serial.print(motorName);
  Serial.println(" homed");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Controller start");

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(_LED_BUILTIN_, OUTPUT);
  pinMode(YAW_LIMIT_PIN, INPUT_PULLUP);
  pinMode(PITCH_LIMIT_PIN, INPUT_PULLUP);
  pinMode(XYE_ENABLE, OUTPUT);
  pinMode(FAN, OUTPUT);
  
  PITCH_STEPPER.setMaxSpeed(MOTORS_MAX_SPEED);
  PITCH_STEPPER.setSpeed(MOTORS_MAX_SPEED);
  YAW_STEPPER.setMaxSpeed(MOTORS_MAX_SPEED);
  YAW_STEPPER.setSpeed(MOTORS_MAX_SPEED);
  
  setMotorsEn(MOT_ENABLED);
  digitalWrite(FAN, HIGH);

  // Home yaw motor until its limit switch reads HIGH
  homeMotor(YAW_STEPPER, YAW_LIMIT_PIN, "YAW", CCW);
  yawHomed = true;
  Serial.print("YAW_STEPPER.currentPosition() : "); Serial.println(YAW_STEPPER.currentPosition());
  Serial.print("yaw_measured_steps_range : "); Serial.println(yaw_measured_steps_range);
  YAW_STEPPER.moveTo(-yaw_measured_steps_range);
  YAW_STEPPER.runToPosition();
  Serial.print("YAW_STEPPER.currentPosition() : "); Serial.println(YAW_STEPPER.currentPosition());

  
  // Home pitch motor until its limit switch reads HIGH
  // homeMotor(PITCH_STEPPER, PITCH_LIMIT_PIN, "PITCH", CW);
  // pitchHomed = true;
  // Serial.print("PITCH_STEPPER.currentPosition() : "); Serial.println(PITCH_STEPPER.currentPosition());
  // PITCH_STEPPER.moveTo(6000);
  // PITCH_STEPPER.runToPosition();
  // Serial.print("PITCH_STEPPER.currentPosition() : "); Serial.println(PITCH_STEPPER.currentPosition());
  
  Serial.println("All motors homed. Ready.");

  setMotorsEn(MOT_DISABLED);

}

void checkSerial()  //method for receiving the commands
{
  //switch-case would also work, and maybe more elegant

  char receivedCommand = '\0'; // Initialize to null
  if (Serial.available() > 0)  //if something comes
  {
    receivedCommand = Serial.read();
  }

  if (receivedCommand != '\0') {
    switch (receivedCommand) {
      case 'n':
        runAllowed = false;  //disable running

        // PITCH_STEPPER.setCurrentPosition(0);  // reset position
        PITCH_STEPPER.stop();            //stop motor
        PITCH_STEPPER.disableOutputs();  //disable power

        YAW_STEPPER.stop();            //stop motor
        YAW_STEPPER.disableOutputs();  //disable power

        Serial.println("STOPPED ALL STEPPERS");
        break;
      default:
        Serial.println("Received odd command : ");
        Serial.println(receivedCommand);
    }
    //START - MEASURE
    // if (receivedCommand == 's')  //this is the measure part
    // {
    //   //example s 2000 500 - 2000 steps (5 revolution with 400 step/rev microstepping) and 500 steps/s speed
    //   runallowed = true;  //allow running


    //   receivedMMdistance = Serial.parseFloat();  //value for the steps
    //   receivedDelay = Serial.parseFloat();       //value for the speed

    //   Serial.print(receivedMMdistance);  //print the values for checking
    //   Serial.print(receivedDelay);
    //   Serial.println("Measure ");          //print the action
    //   stepper.setMaxSpeed(receivedDelay);  //set speed
    //   stepper.move(receivedMMdistance);    //set distance
    // }
    //after we went through the above tasks, newData becomes false again, so we are ready to receive new commands again.
    // newData = false;
  }

}

  void loop() {
  }
