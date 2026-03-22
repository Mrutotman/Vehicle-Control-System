/* ==============================================================================
 * MODULE:        VCS_ThreeSpeed
 * RESPONSIBILITY: Handle physical 3-position switch and optocoupler outputs.
 * ============================================================================== */

#include "vcs_threespeed.h"
#include "vcs_pins.h"
#include "vcs_state_machine.h"
#include "vcs_constants.h"
#include "vcs_uart.h" // For RPi command access in Autonomous mode

uint8_t current_drive_mode = 1; // Default to Medium on boot

void initThreeSpeed() {
    pinMode(PIN_SPEED_SW_LOW, INPUT);
    pinMode(PIN_SPEED_SW_HIGH, INPUT);
    
    pinMode(PIN_SPEED_LOW, OUTPUT);
    pinMode(PIN_SPEED_HIGH, OUTPUT);
    
    setDriveMode(DRIVE_MED); // Default on boot
}

void updateThreeSpeed() {
    // Only allow the physical switch to control speed if we are in MANUAL mode
    if (currentState == MANUAL_STATE) {
        bool swLow = digitalRead(PIN_SPEED_SW_LOW);
        bool swHigh = digitalRead(PIN_SPEED_SW_HIGH);

        if (swLow == LOW) { // Assuming Active LOW (Switch pulls pin to GND)
            setDriveMode(DRIVE_LOW);
        } else if (swHigh == LOW) {
            setDriveMode(DRIVE_HIGH);
        } else {
            setDriveMode(DRIVE_MED); // Center position / No pins grounded
        }
    }
    // Note: In AUTONOMOUS_STATE, the Raspberry Pi controls this via setDriveMode()
}

void setDriveMode(DriveMode mode) {
    switch (mode) {
        case DRIVE_LOW:
            digitalWrite(PIN_SPEED_LOW, HIGH);  // Trigger Low-Speed Opto
            digitalWrite(PIN_SPEED_HIGH, LOW);
            break;
        case DRIVE_MED:
            digitalWrite(PIN_SPEED_LOW, LOW);   // Both Open = Controller Default (Med)
            digitalWrite(PIN_SPEED_HIGH, LOW);
            break;
        case DRIVE_HIGH:
            digitalWrite(PIN_SPEED_LOW, LOW);
            digitalWrite(PIN_SPEED_HIGH, HIGH); // Trigger High-Speed Opto
            break;
    }
}