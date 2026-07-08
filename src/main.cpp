#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <avr/wdt.h>
#include <pins.h>

#define _LED_BUILTIN_ 27

#define MOTORS_MAX_SPEED 500.0
#define HOMING_SPEED 500.0
#define HOMING_ACCELERATION 450.0

// #define YAW_STEP_PIN 15
#define YAW_STEP_PIN 15
#define YAW_DIR_PIN 21
#define YAW_LIMIT_PIN 18

#define PITCH_STEP_PIN 22
#define PITCH_DIR_PIN 23
#define PITCH_LIMIT_PIN 19

#define XYE_ENABLE 14
#define FAN 4

#define BUTTON 20

// 84/18 teeth
const float gear_ratio = 4.666666667;
const float yaw_degs_per_steps = 1.8;
const float pitch_degs_per_steps = 1.8;

//We substract 30 degrees to avoid hitting the limit switch
float SAFETY_RANGE_MULTIPLIER = (360-30) / 360.0f;
float yaw_full_range = ((360 / 1.8) * 16 * gear_ratio);
float yaw_safe_range = yaw_full_range * SAFETY_RANGE_MULTIPLIER;
int yaw_dir = -1;
float pitch_full_range = ((360 / 1.8) * 16 * gear_ratio);
float pitch_safe_range = pitch_full_range * SAFETY_RANGE_MULTIPLIER;
int pitch_dir = 1;

float steps_safety_offset = (1.0f - SAFETY_RANGE_MULTIPLIER) * yaw_full_range;
float middle = yaw_full_range / 2.0f;

AccelStepper YAW_STEPPER(AccelStepper::DRIVER, YAW_STEP_PIN, YAW_DIR_PIN);
AccelStepper PITCH_STEPPER(AccelStepper::DRIVER, PITCH_STEP_PIN, PITCH_DIR_PIN);

MultiStepper STEPPERS;

// Runtime variables
volatile bool runAllowed = false;
enum HomingState
{
    IDLE,
    HOMING_YAW,
    HOMING_PITCH
};
enum MotorsEnableState
{
    MOT_ENABLED,
    MOT_DISABLED
};
enum DIRECTION
{
    CW,
    CCW
};

HomingState homingState = IDLE;
bool yawHomed = false;
bool pitchHomed = false;

int degreesToStep(int degrees) {

}

void setMotorsEn(MotorsEnableState desiredState)
{
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

float getSafePosition(float target, int direction) {
  float abs_target = abs(target);
  float safe_pos = min((steps_safety_offset / 2.0f), max(abs_target, yaw_full_range - (steps_safety_offset / 2.0f)));
  return safe_pos * direction;
}

void safeMove(AccelStepper& stepper, int direction, float target) {
  int abs_target = abs(target);
  if (abs_target > (steps_safety_offset / 2.0f) && abs_target < (yaw_full_range - (steps_safety_offset / 2.0f))) {
    
  }
  
}

void homeMotor(AccelStepper& stepper, int limitSwitchPin, const char* motorName, int direction)
{
    stepper.setMaxSpeed(HOMING_SPEED);
    stepper.setAcceleration(HOMING_ACCELERATION);

    Serial.print("Homing ");
    Serial.print(motorName);
    Serial.println("...");

    // Move towards the limit switch until it reads HIGH
    stepper.move(yaw_full_range * (direction > 0 ? -1 : 1));
    stepper.setSpeed(HOMING_SPEED);
    while (digitalRead(limitSwitchPin) != HIGH)
    {
        stepper.run();
        // stepper.runSpeed();
    }
    stepper.setCurrentPosition(0);
    // stepper.setSpeed();
    stepper.moveTo((steps_safety_offset / 2) * direction);
    stepper.runToPosition();
    
    stepper.moveTo(middle * direction);
    stepper.runToPosition();

    stepper.stop();

    Serial.print(motorName);
    Serial.println(" homed : ");
    Serial.print("Current position : ");
    Serial.println(stepper.currentPosition());
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Controller start");

    // initialize digital pin LED_BUILTIN as an output.
    pinMode(_LED_BUILTIN_, OUTPUT);
    pinMode(YAW_LIMIT_PIN, INPUT_PULLUP);
    pinMode(PITCH_LIMIT_PIN, INPUT_PULLUP);
    pinMode(XYE_ENABLE, OUTPUT);
    // pinMode(FAN, OUTPUT);
    pinMode(BUTTON, INPUT);

    PITCH_STEPPER.setMaxSpeed(MOTORS_MAX_SPEED);
    PITCH_STEPPER.setSpeed(MOTORS_MAX_SPEED);
    PITCH_STEPPER.setAcceleration(MOTORS_MAX_SPEED);
    YAW_STEPPER.setMaxSpeed(MOTORS_MAX_SPEED);
    YAW_STEPPER.setSpeed(MOTORS_MAX_SPEED);
    YAW_STEPPER.setAcceleration(MOTORS_MAX_SPEED);

    setMotorsEn(MOT_ENABLED);

    // Home yaw motor until its limit switch reads HIGH

    Serial.print("yaw_full_range : "); Serial.println(yaw_full_range);
    Serial.print("yaw_safe_range : "); Serial.println(yaw_safe_range);
    Serial.print("middle : "); Serial.println(middle);

    Serial.print("middle + 1000 : "); Serial.println(middle + 1000);
    homeMotor(YAW_STEPPER, YAW_LIMIT_PIN, "YAW", yaw_dir);
    yawHomed = true;
    // YAW_STEPPER.moveTo(-yaw_safe_range);
    // YAW_STEPPER.runToPosition();
    
    // Home pitch motor until its limit switch reads HIGH
    homeMotor(PITCH_STEPPER, PITCH_LIMIT_PIN, "PITCH", pitch_dir);
    pitchHomed = true;
    
    STEPPERS.addStepper(YAW_STEPPER);
    STEPPERS.addStepper(PITCH_STEPPER);

    Serial.println("All motors homed, entering operation status");

    Serial.print("PITCH_STEPPER.currentPosition() : "); Serial.println(PITCH_STEPPER.currentPosition());
    Serial.print("YAW_STEPPER.currentPosition() : "); Serial.println(YAW_STEPPER.currentPosition());
    Serial.print("steps_safety_offset : "); Serial.println(steps_safety_offset);
    Serial.print("middle : "); Serial.println(middle);

}

void checkSerial() // method for receiving the commands
{
    // switch-case would also work, and maybe more elegant

    char receivedCommand = '\0'; // Initialize to null
    if (Serial.available() > 0)  // if something comes
    {
        receivedCommand = Serial.read();
    }

    if (receivedCommand != '\0')
    {
        switch (receivedCommand)
        {
        case 'n':
            runAllowed = false; // disable running

            // PITCH_STEPPER.setCurrentPosition(0);  // reset position
            PITCH_STEPPER.stop();           // stop motor
            PITCH_STEPPER.disableOutputs(); // disable power

            YAW_STEPPER.stop();           // stop motor
            YAW_STEPPER.disableOutputs(); // disable power

            Serial.println("STOPPED ALL STEPPERS");
            break;
        default:
            Serial.println("Received odd command : ");
            Serial.println(receivedCommand);
        }
        // START - MEASURE
        //  if (receivedCommand == 's')  //this is the measure part
        //  {
        //    //example s 2000 500 - 2000 steps (5 revolution with 400 step/rev
        //    microstepping) and 500 steps/s speed runallowed = true;  //allow
        //    running

        //   receivedMMdistance = Serial.parseFloat();  //value for the steps
        //   receivedDelay = Serial.parseFloat();       //value for the speed

        //   Serial.print(receivedMMdistance);  //print the values for checking
        //   Serial.print(receivedDelay);
        //   Serial.println("Measure ");          //print the action
        //   stepper.setMaxSpeed(receivedDelay);  //set speed
        //   stepper.move(receivedMMdistance);    //set distance
        // }
        // after we went through the above tasks, newData becomes false again,
        // so we are ready to receive new commands again.
        // newData = false;
    }
}

int yaw_target = 0;

void loop()
{
    // digitalWrite(FAN, HIGH);
    if (digitalRead(BUTTON) == HIGH)
    {
        Serial.println("Resetting board...");
        wdt_enable(WDTO_15MS);
        while (1)
        {
        }
    }


    if (YAW_STEPPER.distanceToGo() == 0)
    {
      if (yaw_target < middle) {
        
        int new_target = middle + 1000;
        // Serial.print("getSafePosition(new_target, yaw_dir) : "); Serial.println(getSafePosition(new_target, yaw_dir));
        Serial.print("new_target a : "); Serial.println(new_target);
        // YAW_STEPPER.moveTo(getSafePosition(new_target, yaw_dir));
        YAW_STEPPER.moveTo(new_target * yaw_dir);
        yaw_target = new_target;
      } else if (yaw_target >= middle) {
        int new_target = middle - 1000;
        Serial.print("new_target b : "); Serial.println(new_target);
        // YAW_STEPPER.moveTo(getSafePosition(new_target, yaw_dir));
        YAW_STEPPER.moveTo(new_target * yaw_dir);
        yaw_target = new_target;
      }
    }
    YAW_STEPPER.run();

    // STEPPERS.moveTo();

    
}
