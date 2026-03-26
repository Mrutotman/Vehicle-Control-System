#include "vcs_reverse.h"
#include "vcs_pins.h"
#include "vcs_hallsensor.h" // Required to fetch getMeasuredRPM()

// Track the actual state for telemetry and UI
static bool reverseEngaged = false;

void initReverse() {
    // Input: Driver's physical switch (LOW means they flipped it)
    pinMode(PIN_REVERSE_IN, INPUT_PULLUP);
    
    // Output: To Level Shifter -> Motor Controller Yellow Wire
    pinMode(PIN_REVERSE_OUT, OUTPUT);
    
    // Start in forward mode (HIGH = inactive/forward for the controller)
    digitalWrite(PIN_REVERSE_OUT, HIGH); 
}

void updateReverse() {
    // 1. Read what the driver wants
    bool reverseRequested = (digitalRead(PIN_REVERSE_IN) == LOW);
    
    // 2. Read what the car is actually doing
    // Assuming getMeasuredRPM() returns the absolute speed value
    int currentRPM = getMeasuredRPM(); 

    // 3. The Security Gate
    // Only allow the controller to reverse if the car is basically stopped.
    // (Threshold of 5 RPM accounts for minor sensor noise/vibration).
    if (reverseRequested && currentRPM < 5) {
        digitalWrite(PIN_REVERSE_OUT, LOW); // Pull Yellow wire to Ground
        reverseEngaged = true;
    } else {
        digitalWrite(PIN_REVERSE_OUT, HIGH); // Release Yellow wire (Forward)
        reverseEngaged = false;
    }
}

bool isReverseEngaged() {
    return reverseEngaged;
}