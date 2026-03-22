#include "vcs_embutton.h"

// Global Telemetry Variable
bool is_estop_pressed = false;

void initEmButton() {
    // Standard E-Stops are Normally Closed (NC) to ground for safety.
    // We use INPUT_PULLUP so if the wire breaks, it triggers an E-Stop automatically.
    pinMode(PIN_EMBUTTON, INPUT_PULLUP);
    
    // Initial read
    is_estop_pressed = (digitalRead(PIN_EMBUTTON) == LOW);
}

void updateEmButton() {
    // Update the telemetry variable for the UART broadcast
    is_estop_pressed = (digitalRead(PIN_EMBUTTON) == LOW);
}

bool isEmButtonPressed() {
    // When the State Machine asks, do a direct, instant hardware read
    // to bypass any potential loop delays in the CommTask.
    is_estop_pressed = (digitalRead(PIN_EMBUTTON) == LOW);
    return is_estop_pressed;
}