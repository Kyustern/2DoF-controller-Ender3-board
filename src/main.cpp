#include <Arduino.h>
#include <AccelStepper.h>
#include <pins.h>

#define _LED_BUILTIN_ 27

#define MOTORS_MAX_SPEED 50.0
//Home the yaw motor once and set here the measured steps it took to reach the end switch
#define YAW_MEASURED_RANGE 1000
#define YAW_STEP_PIN 15
#define YAW_DIR_PIN 21
#define YAW_LIMIT_PIN 18
// #define YAW_LIMIT_PIN PC3

//Home the pitch motor once and set here the measured steps it took to reach the end switch
#define PITCH_MEASURED_RANGE 1000

#define PITCH_STEP_PIN 22
#define PITCH_DIR_PIN 23
#define PITCH_LIMIT_PIN 19
// #define PITCH_LIMIT_PIN PC2

AccelStepper YAW_STEPPER(AccelStepper::DRIVER, YAW_STEP_PIN, YAW_DIR_PIN);
AccelStepper PITCH_STEPPER(AccelStepper::DRIVER, PITCH_STEP_PIN, PITCH_DIR_PIN);

//Runtime variables
volatile bool runAllowed = false;
//Runtime constants
int YAW_SAFE_RANGE = YAW_MEASURED_RANGE - 50;
int PITCH_SAFE_RANGE = PITCH_MEASURED_RANGE - 50;

void stop_yaw() {
  // runAllowed = false;

  // YAW_STEPPER.setCurrentPosition(0);
  // YAW_STEPPER.stop();
  // YAW_STEPPER.disableOutputs();

  Serial.println("YAW MOTOR STOPPED");
}

void stop_pitch() {
  runAllowed = false;

  PITCH_STEPPER.setCurrentPosition(0);
  PITCH_STEPPER.stop();
  PITCH_STEPPER.disableOutputs();

  Serial.println("PITCH MOTOR STOPPED");
}

void home_yaw_axis() {
    runAllowed = true;  //allow running

    Serial.println("HOMING");  //print action

    YAW_STEPPER.setAcceleration(50);
    YAW_STEPPER.setMaxSpeed(50);

    // PITCH_STEPPER.setAcceleration(100);
    // PITCH_STEPPER.setMaxSpeed(100);

    //distance should be larger than the length of the whole path, hence the additional 50 steps
    // YAW_STEPPER.move(-1 * (YAW_MEASURED_RANGE + 50));
    
    Serial.println("Yaw motor moving");
    int count = 0;
    while (count <= YAW_MEASURED_RANGE)
    {
      YAW_STEPPER.move(1);
      count ++;
    }
    Serial.print("count : "); Serial.println(count);
    runAllowed = false;
    
}

void setup() {
  Serial.begin(9600);
  Serial.println("Controller start");

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(_LED_BUILTIN_, OUTPUT);
  pinMode(YAW_LIMIT_PIN, INPUT_PULLUP);
  pinMode(PITCH_LIMIT_PIN, INPUT_PULLUP);

  PITCH_STEPPER.setMaxSpeed(MOTORS_MAX_SPEED);
  PITCH_STEPPER.setSpeed(MOTORS_MAX_SPEED);

  YAW_STEPPER.setMaxSpeed(MOTORS_MAX_SPEED);
  YAW_STEPPER.setSpeed(MOTORS_MAX_SPEED);

  attachInterrupt(YAW_LIMIT_PIN, stop_yaw, HIGH);
  attachInterrupt(PITCH_LIMIT_PIN, stop_pitch, RISING);


  delay(100);
  home_yaw_axis();
}

void checkSerial()  //method for receiving the commands
{
  //switch-case would also work, and maybe more elegant

  char receivedCommand;
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

  // the loop function runs over and over again forever
  void loop() {
    delay(100);
    Serial.print("YAW_LIMIT_PIN : "); Serial.println(digitalRead(YAW_LIMIT_PIN));
    // Serial.print("PITCH LIM"); Serial.println(digitalRead(PITCH_LIMIT_PIN));
    // if (runAllowed == false) return;
    // YAW_STEPPER.moveTo(YAW_SAFE_RANGE);
    // YAW_STEPPER.moveTo(0);
  }
