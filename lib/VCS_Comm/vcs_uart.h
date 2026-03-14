#ifndef VCS_UART_H
#define VCS_UART_H

#include <Arduino.h>
#include "../VCS_Config/vcs_constants.h"

// Sends telemetry data back to the RPi in the same binary format (Type=0x02)
void sendTelemetry(float rpm, uint16_t steer, float volt, uint8_t state); 

// Initializes the UART hardware
void initUART();

// Parses incoming bytes using the state machine. Call this at 20 Hz or faster.
void updateUART();

// Getters for the parsed ANS commands
uint8_t getRPiCommandMode();
float getTargetRPM();
uint16_t getTargetSteering();
uint8_t getTargetBrake();

// Safety check for the state machine (True if a valid packet arrived recently)
bool rpiHeartbeatReceived();

#endif // VCS_UART_H