/* ==============================================================================
 * MODULE:        VCS_State_Machine
 * RESPONSIBILITY: Global System Logic and Security Enforcement.
 * DESCRIPTION:   Strictly implements the state table for Motor, Brake, and 
 * Steering. Handles transitions and self-test logic.
 * ============================================================================== */


#include "vcs_pins.h"
#include "vcs_state_machine.h"
#include "vcs_uart.h"
#include "vcs_steering.h"
#include "vcs_embutton.h"

VcsState currentState = INIT_STATE;
static uint32_t initStartTime = 0;
const uint32_t DMS_HANDOVER_DELAY = 1000; // 1 second hold required
static uint32_t dmsHoldStartTime = 0;     // Tracks how long DMS has been held

void initState_Machine() {
    currentState = INIT_STATE;
    initStartTime = millis();
}

void updateStateMachine(uint32_t externalFaults) {

    // 1. GLOBAL OVERRIDES (Physical Security)
    if (isEmButtonPressed()) {
        currentState = ESTOP_STATE;
        return;
    }

    // 2. READ INPUTS
    bool dmsPressed = (digitalRead(PIN_DMS_BUTTON) == LOW); // Active LOW
    bool brakePressed = (digitalRead(PIN_LOWBRAKE_IN) == LOW);
    

    // 3. STATE TRANSITION LOGIC
    switch (currentState) {
        
        case INIT_STATE:
            // Standard 2s self-test
            if (millis() - initStartTime > 2000) {
                if (getMeasuredSteering() > 10 && rpiHeartbeatReceived()) {
                    currentState = IDLE_STATE;
                }
            }
            break;

        case IDLE_STATE:
            // From IDLE, we default to MANUAL if the driver is ready
            if (rpiHeartbeatReceived()) {
                currentState = MANUAL_STATE;
            }
            break;

        case MANUAL_STATE:
            // --- RULE: MANUAL TO AUTO via DMS + DELAY ---
            if (dmsPressed && rpiHeartbeatReceived()) {
                if (dmsHoldStartTime == 0) {
                    dmsHoldStartTime = millis(); // Start the handover timer
                }

                // Check if the 1-second "Intent Window" has passed
                if (millis() - dmsHoldStartTime >= DMS_HANDOVER_DELAY) {
                    currentState = AUTONOMOUS_STATE;
                    dmsHoldStartTime = 0; // Reset for next time
                }
            } else {
                dmsHoldStartTime = 0; // Driver let go or Pi died; reset timer
            }
            break;

        case AUTONOMOUS_STATE:
            // --- RULE: AUTO TO MANUAL via BRAKE ---
            // If the brake is tapped, the driver takes control back instantly
            if (brakePressed) {
                currentState = MANUAL_STATE;
            }

            // --- RULE: ALWAYS DMS for AUTO ---
            // If the driver lets go of the button, the car must stop for safety
            if (!dmsPressed) {
                currentState = IDLE_STATE; // Drop to safe neutral
            }

            // Security: Loss of Pi heartbeat drops to FAULT
            if (!rpiHeartbeatReceived()) {
                currentState = FAULT_STATE;
            }
            break;

        case FAULT_STATE:
            // To clear, ensure the brake is pressed (as a safety park) and DMS is released
            if (brakePressed && !dmsPressed && rpiHeartbeatReceived()) {
                currentState = IDLE_STATE;
            }
            break;

        case ESTOP_STATE:
            // Locked until power cycle
            break;
    }

    // For debugging: Print state changes to Serial Monitor
    static VcsState lastState = INIT_STATE;;

    // Print to Serial Monitor ONLY when the state changes
    if (currentState != lastState) {
        Serial.print("VCS STATE CHANGE: ");
        if (currentState == IDLE_STATE) Serial.println("IDLE");
        if (currentState == MANUAL_STATE) Serial.println("MANUAL");
        if (currentState == AUTONOMOUS_STATE) Serial.println("AUTONOMOUS (DMS ACTIVE)");
        if (currentState == FAULT_STATE) Serial.println("!!! FAULT !!!");
        lastState = currentState;
    }
}

// Helpers for Actuator Lockouts
bool isAutonomousMode() {
    return (currentState == AUTONOMOUS_STATE);
}

bool isDrivingState() {
    return (currentState == MANUAL_STATE || currentState == AUTONOMOUS_STATE);
}

uint32_t getDMSHoldStartTime() {
    return dmsHoldStartTime;
}