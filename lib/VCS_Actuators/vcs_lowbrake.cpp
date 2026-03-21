/* ==============================================================================
 * MODULE:        VCS_LowBrake
 * RESPONSIBILITY: Electronic Brake Actuation & Monitoring.
 * DESCRIPTION:   Reads the physical brake lever switch and controls the optocoupler
 * connected to the e-bike controller's Low Level Brake wire.
 * On system boot or during an E-Stop, this module defaults to 
 * ENGAGED for hardware-level safety.
 * ============================================================================== */

#include "vcs_lowbrake.h"

bool physicalBrakeState = false;

void initLowBrake() {
    // Configure inputs and outputs
    pinMode(PIN_LOWBRAKE_IN, INPUT_PULLUP); // Assuming the switch pulls to ground
    pinMode(PIN_LOWBRAKE_OUT, OUTPUT);
    
    // Engage brake fully on boot for safety (Migrated from original code)
    forceBrakeEngagement(true); 
}

void updateLowBrake() {
    // 1. Read the physical pedal switch with standard debounce
    static uint32_t lastDebounceTime = 0;
    static bool lastReading = HIGH;
    
    bool currentReading = digitalRead(PIN_LOWBRAKE_IN);
    
    if (currentReading != lastReading) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_TIME_MS) {
        // Active LOW: If the pin is grounded, the brake is pressed
        physicalBrakeState = (currentReading == LOW);
    }
    lastReading = currentReading;
    
    // 2. Hardware-level Safety Fallback: 
    // If the human presses the physical brake, fire the optocoupler immediately,
    // bypassing the state machine for instant response.
    if (physicalBrakeState) {
        digitalWrite(PIN_LOWBRAKE_OUT, HIGH); // HIGH triggers the optocoupler
    }
}

void forceBrakeEngagement(bool engage) {
    if (engage) {
        digitalWrite(PIN_LOWBRAKE_OUT, HIGH); // Force trigger optocoupler
    } else {
        // SECURITY CHECK: Only allow the state machine to release the software brake 
        // if the human is NOT currently holding down the physical brake pedal.
        if (!physicalBrakeState) {
            digitalWrite(PIN_LOWBRAKE_OUT, LOW);
        }
    }
}

bool isPhysicalBrakePressed() {
    return physicalBrakeState;
}