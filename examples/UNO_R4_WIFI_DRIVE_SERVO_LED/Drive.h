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
const int ENA = 5;
const int IN1 = 4;
const int IN2 = 7;

// MOTOR RIGHT PINS
const int ENB = 6;
const int IN3 = 8;
const int IN4 = 9;

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