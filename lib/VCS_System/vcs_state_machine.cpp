#include "vcs_state_machine.h"
#include "vcs_uart.h"
#include "vcs_pins.h"
#include "vcs_embutton.h"
#include "vcs_lowbrake.h"
#include "vcs_reverse.h"


VcsState currentState = INIT_STATE;
uint32_t dmsStartTime = 0;

void initState_Machine() {
    currentState = INIT_STATE;
    dmsStartTime = 0;
}

void updateStateMachine(uint32_t faults) {
    static VcsState lastState = INIT_STATE;

    // --- 1. PRIORITY SAFETY OVERRIDES (SIDLAK Hierarchy) ---
    
    // Physical E-Stop is the highest priority
    if (isEmButtonPressed()) {
        currentState = ESTOP_STATE;
    } 
    // External faults (Over-current/Voltage) or Heartbeat Loss
    else if (faults > 0 || (!rpiHeartbeatReceived() && currentState == AUTONOMOUS_STATE)) {
        currentState = FAULT_STATE;
    }

    // --- 2. STATE TRANSITION LOGIC ---
    switch (currentState) {
        case INIT_STATE:
            // Boot delay for sensor stabilization
            if (millis() > 2000) currentState = IDLE_STATE;
            break;

        case IDLE_STATE:
            // Wait for Pi to be alive before allowing Manual mode
            if (rpiHeartbeatReceived()) currentState = MANUAL_STATE;
            break;

        case MANUAL_STATE:
            // Transition to Auto: Requires physical DMS hold (1.0s), 
            // the RPi requesting Auto Mode (1), AND the car MUST NOT be in Reverse.
            if (digitalRead(PIN_DMS_BUTTON) == LOW && getRPiCommandMode() == 1 && !isReverseEngaged()) {
                if (dmsStartTime == 0) dmsStartTime = millis();
                if (millis() - dmsStartTime > 1000) currentState = AUTONOMOUS_STATE;
            } else {
                dmsStartTime = 0;
            }
            break;

        case AUTONOMOUS_STATE:
            // EXIT TO MANUAL IF:
            // 1. Physical Brake is pressed (Hard override - using debounced function!)
            // 2. RPi sends a Manual request (Soft override)
            // 3. DMS button is released (Safety requirement)
            if (isPhysicalBrakePressed() || 
                getRPiCommandMode() == 0 || 
                digitalRead(PIN_DMS_BUTTON) == HIGH) {
                currentState = MANUAL_STATE;
            }
            break;
            
        case FAULT_STATE:
            // In v1.4, reset only happens if Pi is re-linked and DMS is cleared
            if (rpiHeartbeatReceived() && digitalRead(PIN_DMS_BUTTON) == HIGH) {
                currentState = IDLE_STATE;
            }
            break;

        case ESTOP_STATE:
            // ESTOP is a hard-lock. Requires physical power cycle or hard reset.
            break;
    }

    // --- 3. STATE DEBUGGING ---
    if (currentState != lastState) {
        Serial.print("VCS_STATE_MACHINE: Transitioned to ");
        Serial.println(getStateName(currentState));
        lastState = currentState;
    }
}

// Helper to get string names for the Serial Monitor
const char* getStateName(VcsState state) {
    switch (state) {
        case INIT_STATE:       return "INIT";
        case IDLE_STATE:       return "IDLE";
        case MANUAL_STATE:     return "MANUAL";
        case AUTONOMOUS_STATE: return "AUTONOMOUS";
        case FAULT_STATE:      return "FAULT";
        case ESTOP_STATE:      return "ESTOP";
        default:               return "UNKNOWN";
    }
}

// Getters for other modules
bool isAutonomousMode() { return currentState == AUTONOMOUS_STATE; }
uint32_t getDMSHoldStartTime() { return dmsStartTime; }