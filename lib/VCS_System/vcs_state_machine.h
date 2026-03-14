#ifndef VCS_STATE_MACHINE_H
#define VCS_STATE_MACHINE_H

#include <Arduino.h>
#include "../VCS_Config/vcs_pins.h"
#include "../VCS_Config/vcs_constants.h"

// Define the core SIDLAK system states [cite: 169]
enum SystemState { 
    INIT_STATE, 
    IDLE_STATE, 
    MANUAL_STATE, 
    AUTONOMOUS_STATE, 
    FAULT_STATE, 
    ESTOP_STATE 
};

// Expose the current state globally (Read-only ideally, but extern for simplicity)
extern SystemState currentState;
extern uint32_t faultFlags;

// Core state machine functions
void initState_Machine();
void updateStateMachine();

// --- Hardware/Logic Stubs (To be implemented in VCS_Safety / VCS_Comm / VCS_Steering) ---
uint32_t readFaultInputs();
bool selfTestPassed();
uint8_t rcSwitchPosition();
bool emergencyStopPressed();
bool rpiHeartbeatReceived();
uint8_t getRPiCommandMode(); // 0=IDLE, 1=MANUAL, 2=AUTO, 3=FAULT, 4=ESTOP [cite: 84]

// Mode constants for RC/RPi logic [cite: 84]
#define MODE_IDLE   0
#define MODE_MANUAL 1
#define MODE_AUTO   2
#define MODE_FAULT  3
#define MODE_ESTOP  4

#endif // VCS_STATE_MACHINE_H