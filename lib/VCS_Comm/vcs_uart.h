#ifndef VCS_UART_H
#define VCS_UART_H

#include <Arduino.h>
#include "vcs_pins.h"
#include "vcs_constants.h"

// Core UART functions
void initUART();
void updateUART();
void sendTelemetry(float rpm, uint16_t steer, float volt, uint8_t state);
void broadcastVehicleTelemetry();

// Data Retrieval Functions
uint8_t getRPiCommandMode(); 
float getTargetRPM(); 
uint16_t getTargetSteering(); 
uint8_t getTargetBrake(); 
bool getRPiReverseCommand();

// Security checks
bool rpiHeartbeatReceived();

#endif // VCS_UART_H