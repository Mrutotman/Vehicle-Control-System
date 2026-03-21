#ifndef VCS_SIMULATION_H
#define VCS_SIMULATION_H

#include <Arduino.h>

// Toggle this to true to test without the 60V system or motors connected
#define SIMULATION_MODE true 

// Global simulated variables
extern float sim_steer_pos;
extern float sim_motor_rpm;

// Core simulation functions
void updateSimulatedPhysics(int pulse_freq, bool direction, float throttle_pwm);
float getSimulatedRPM();
float getSimulatedSteering(); // Returns a value from 0 to 1000 representing the simulated steering position

#endif