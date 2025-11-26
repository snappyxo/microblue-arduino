/*
 * Drive.cpp
 *
 * Description:
 * Source file implementing functions for controlling a dual-motor drive system.

 * Developed by Mechanismic Inc.
 * Free for use.
 */

#include "Drive.h"

// Initialize motor pins as outputs
void setMotorPins() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

// Converts throttle and steering inputs into PWM signals for motors
void drive(int throttle, int steering) {
  // Print Debug Info
  Serial.print("throttle: ");
  Serial.print(throttle);
  Serial.print("\tsteering: ");
  Serial.print(steering);

  // Brake if throttle is zero
  if (throttle == 0) {
      motorBrake();
      return;
  }

  // Set motor direction based on throttle value
  if (throttle > 0) {
    // Forward
    motorSetForward();
  } else {
    // Backward
    motorSetBackward();
  }

  // Map throttle to PWM range.
  int mappedSpeed = map(abs(throttle), 0, 512, MINIMUM_MOTOR_SPEED, 255);
  // Map steering to PWM range.
  int reducedSpeed = map(abs(steering), 0, 512, mappedSpeed, MINIMUM_MOTOR_SPEED);

  int leftMotorSpeed, rightMotorSpeed;
  if (steering > 0) {
    // Turn Right: reduce right motor speed
    leftMotorSpeed = mappedSpeed;
    rightMotorSpeed = reducedSpeed;
  } else {
    // Turn Left: reduce left motor speed
    leftMotorSpeed = reducedSpeed;
    rightMotorSpeed = mappedSpeed;
  }

  // Set motor speeds
  analogWrite(ENA, leftMotorSpeed);
  analogWrite(ENB, rightMotorSpeed);

  // Print Debug Info
  Serial.print("\tmappedSpeed: ");
  Serial.print(mappedSpeed);
  Serial.print("\treducedSpeed: ");
  Serial.print(reducedSpeed);
  Serial.print("\tleftMotorSpeed: ");
  Serial.print(leftMotorSpeed);
  Serial.print("\trightMotorSpeed: ");
  Serial.println(rightMotorSpeed);
}

// Configures the motor controller to stop the motors.
void motorBrake() {
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  // Do not write the motor speeds on this function. It simply configures the motor controller.
}

// Configures the motor controller to have the robot move forward.
void motorSetForward() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // Do not write the motor speeds on this function. It simply configures the motor controller.
}

// Configures the motor controller to have the robot move backwards.
void motorSetBackward() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Do not write the motor speeds on this function. It simply configures the motor controller.
}