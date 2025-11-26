/*
 * Drive.h
 *
 * Description:
 * Header file defining functions and constants for controlling a dual-motor drive system.
 *
 * Developed by Mechanismic Inc.
 * Free for use.
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