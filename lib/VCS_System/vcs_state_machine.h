#ifndef VCS_STATE_MACHINE_H
#define VCS_STATE_MACHINE_H

#include <Arduino.h>

enum VcsState {
    INIT_STATE,
    IDLE_STATE,
    MANUAL_STATE,
    AUTONOMOUS_STATE,
    FAULT_STATE,
    ESTOP_STATE
};

extern VcsState currentState;

void initState_Machine();
void updateStateMachine(uint32_t externalFaults);

uint32_t getDMSHoldStartTime();

// Helper functions for other modules
bool isAutonomousMode();
bool isDrivingState();

#endif