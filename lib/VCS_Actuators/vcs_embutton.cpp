#include "vcs_embutton.h"
#include "vcs_pins.h"
#include "vcs_constants.h" // Ensure DEBOUNCE_TIME_MS is defined here

void initEmButton() {
    pinMode(PIN_EMBUTTON, INPUT_PULLUP); 
}

// Rename this to match what the State Machine is calling!
bool isEmButtonPressed() {
    static uint32_t lastDebounce = 0;
    static bool estopState = false;
    
    // Check if button is pushed (Active LOW)
    if (digitalRead(PIN_EMBUTTON) == LOW) { 
        if (millis() - lastDebounce > DEBOUNCE_TIME_MS) {
            estopState = true;
        }
    } else {
        estopState = false;
        lastDebounce = millis();
    }
    
    return estopState;
}