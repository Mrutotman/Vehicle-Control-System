/* ==============================================================================
 * MODULE:        VCS_State_Machine
 * RESPONSIBILITY: State machine management[cite: 37].
 * DESCRIPTION:   Enforces the strict SIDLAK safety hierarchy. Evaluates inputs 
 * from the RC controller, the Raspberry Pi, and hardware fault 
 * sensors to transition the system safely between states: 
 * INIT, IDLE, MANUAL, AUTONOMOUS, FAULT, and ESTOP[cite: 169]. 
 * The motor is strictly prohibited from receiving power unless 
 * the state evaluates to MANUAL or AUTONOMOUS.
 * EXECUTION:     Called continuously in the main fast loop.
 * ============================================================================== */

#include "vcs_state_machine.h"

SystemState currentState = INIT_STATE;
SystemState nextState = INIT_STATE;
uint32_t faultFlags = 0;

void initState_Machine() {
    currentState = INIT_STATE;
    faultFlags = 0;
}

void updateStateMachine() {
    // 1. Always read all fault inputs first (Over-current, Under-voltage, etc.)
    faultFlags = readFaultInputs();

    // 2. Determine next state based on current state and inputs
    switch(currentState) {
        
        case INIT_STATE:
            // Run self-test procedures
            if(selfTestPassed()) {
                nextState = IDLE_STATE;
            } else {
                faultFlags = 1; // FAULT_SELFTEST
                nextState = FAULT_STATE;
            }
            break;

        case IDLE_STATE:
            if(faultFlags != 0) {
                nextState = FAULT_STATE;
            } else if(rcSwitchPosition() == MODE_MANUAL) {
                nextState = MANUAL_STATE;
            } else if(getRPiCommandMode() == MODE_AUTO) {
                nextState = AUTONOMOUS_STATE;
            } else {
                nextState = IDLE_STATE; // Stay in IDLE [cite: 213]
            }
            break;

        case MANUAL_STATE:
            if(faultFlags != 0) {
                nextState = FAULT_STATE;
            } else if(rcSwitchPosition() == MODE_IDLE) {
                nextState = IDLE_STATE;
            } else if(emergencyStopPressed()) {
                nextState = ESTOP_STATE;
            } else {
                nextState = MANUAL_STATE;
            }
            break;

        case AUTONOMOUS_STATE:
            if(faultFlags != 0) {
                nextState = FAULT_STATE;
            } else if(rcSwitchPosition() != MODE_IDLE) { // RC override acts as an interrupt [cite: 229]
                nextState = MANUAL_STATE;
            } else if(emergencyStopPressed()) {
                nextState = ESTOP_STATE;
            } else if(!rpiHeartbeatReceived()) {
                faultFlags = 2; // FAULT_COMMS_LOSS [cite: 234]
                nextState = FAULT_STATE;
            } else {
                nextState = AUTONOMOUS_STATE;
            }
            break;

        case FAULT_STATE:
            // Stay in FAULT until reset, but allow escalation to ESTOP
            if(emergencyStopPressed()) {
                nextState = ESTOP_STATE; 
            } else {
                nextState = FAULT_STATE; 
            }
            break;

        case ESTOP_STATE:
            // Critical fault: Requires physical power cycle or strict reset sequence to clear
            nextState = ESTOP_STATE;
            break;
    }

    // 3. Apply the state transition
    currentState = nextState;
}

uint8_t rcSwitchPosition() { 
    // Basic stub using the analog pin defined in vcs_pins.h
    int val = analogRead(PIN_RC_MODE_SW);
    if (val > 800) return MODE_AUTO;
    if (val > 400) return MODE_MANUAL;
    return MODE_IDLE;
}