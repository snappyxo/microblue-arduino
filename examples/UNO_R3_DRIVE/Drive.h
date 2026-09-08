/*
 * Drive.h
 *
 * Description:
 * Header file defining functions and constants for controlling a dual-motor drive system.
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#ifndef DRIVE_H
#define DRIVE_H

#include "Arduino.h"

// MOTOR LEFT PINS
const int ENA = 6;
const int IN1 = 4;
const int IN2 = 5;

// MOTOR RIGHT PINS
const int ENB = 11;
const int IN3 = 13;
const int IN4 = 12;

// MOTOR DIRECTION
// If a motor spins the wrong way when the robot should move forward, change
// its flag below to true. This swaps the IN1/IN2 (or IN3/IN4) signals in
// software so the motor wires do not have to be physically swapped.
const bool LEFT_MOTOR_REVERSED = false;
const bool RIGHT_MOTOR_REVERSED = false;

// MOTOR PARAMETERS
// Minimum PWM value for analogWrite to spin motor when robot is on the ground.
const int MINIMUM_MOTOR_SPEED = 40;

// Function Prototypes
void setMotorPins();
void drive(int throttle, int steering);
void motorBrake();
void motorSetForward();
void motorSetBackward();

#endif