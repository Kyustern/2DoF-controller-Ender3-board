// // Random.pde
// // -*- mode: C++ -*-
// //
// // Make a single stepper perform random changes in speed, position and
// // acceleration
// //
// // Copyright (C) 2009 Mike McCauley
// // $Id: Random.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

// #include <AccelStepper.h>

// // Define a stepper and the pins it will use
// #define MOTORS_MAX_SPEED 50.0
// // Home the yaw motor once and set here the measured steps it took to reach the
// // end switch
// #define YAW_MEASURED_STEPS_RANGE 1000
// #define YAW_STEP_PIN 15
// // #define YAW_STEP_PIN 3131
// #define YAW_DIR_PIN 21
// #define YAW_LIMIT_PIN 18
// // #define YAW_LIMIT_PIN PC3

// // Home the pitch motor once and set here the measured steps it took to reach
// // the end switch
// #define PITCH_MEASURED_STEPS_RANGE 1000

// #define PITCH_STEP_PIN 31
// #define PITCH_DIR_PIN 23
// #define PITCH_LIMIT_PIN 19
// // #define PITCH_LIMIT_PIN PC2

// const int ESTEP = 1;
// const int EDIR = 0;
// const int EN = 14;

// // AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2,
// // 3, 4, 5
// AccelStepper stepper(AccelStepper::DRIVER, ESTEP, EDIR);
// void setup()
// {
//     Serial.begin(9600);

//     pinMode(ESTEP, OUTPUT);
//     pinMode(EDIR, OUTPUT);
//     pinMode(EN, OUTPUT);

//     digitalWrite(EDIR, HIGH);
//     digitalWrite(EN, LOW);
// }

// void loop()
// {
//     // digitalWrite(ESTEP, HIGH);
//     // delayMicroseconds(250);
//     // digitalWrite(ESTEP, LOW);
//     // delayMicroseconds(250);
//     if (stepper.distanceToGo() == 0)
//     {
//         // Random change to speed, position and acceleration
//         // Make sure we dont get 0 speed or accelerations
//         delay(100);
//         stepper.moveTo(rand() % 5000);
//         stepper.setMaxSpeed(600);
//         stepper.setAcceleration(600);
//     }
//     stepper.run();
// }