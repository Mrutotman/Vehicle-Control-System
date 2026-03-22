#ifndef VCS_STATE_MACHINE_H
#define VCS_STATE_MACHINE_H

#include <Arduino.h>

// SIDLAK Safety Hierarchy Enums
enum VcsState {
    INIT_STATE,        // Power-on self-test & stabilization
    IDLE_STATE,        // Standby; waiting for RPi heartbeat
    MANUAL_STATE,      // Human-in-the-loop control
    AUTONOMOUS_STATE,  // RPi-controlled driving (DMS Active)
    FAULT_STATE,       // Software/Comms fail-safe triggered
    ESTOP_STATE        // Critical hardware lockout (Hard reset required)
};

// Global State Variable
extern VcsState currentState;

// Initialization and Main Logic
void initState_Machine();
void updateStateMachine(uint32_t externalFaults);

// Telemetry & Display Helpers
uint32_t getDMSHoldStartTime();
const char* getStateName(VcsState state);

// Logic Helpers for Actuators/UART
bool isAutonomousMode();
bool isDrivingState();

#endif // VCS_STATE_MACHINE_H