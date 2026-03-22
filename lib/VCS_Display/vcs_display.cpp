#include "vcs_display.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void initDisplay() {
    pinMode(PIN_LED_STATUS, OUTPUT);
    pinMode(PIN_LED_FAULT, OUTPUT);
    pinMode(PIN_LED_MODE, OUTPUT);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        // Log I2C failure if necessary
    }
    display.clearDisplay();
    // Use the explicit SSD1306_WHITE to prevent macro collisions on ARM
    display.setTextColor(SSD1306_WHITE); 
}

/************* ✨ Windsurf Command ⭐  *************/
/**
 * @brief Updates the display with the current state, speed, and steering angle.
 * @details This function does the following:
 * 1. GLOBAL SYNC TIMER: Create a 2Hz flash (500ms period) to the status LED.
 * 2. INDICATOR LOGIC: Use a strict table compliance to turn on the appropriate LEDs based on the current state.
 * 3. OLED REFRESH: Clear the OLED display and print out the current state, speed, and steering angle.
 * @param rpm The current speed of the vehicle in RPM.
 * @param steer The current steering angle in 0-1000 scale.
 * @param speedMode The current gear of the vehicle (0: Low, 1: Medium, 2: High)
 */
/******* 5e619574-21ef-4ca0-9247-6bdb9313ad0f  *******/
void updateDisplay(float rpm, uint16_t steer, uint8_t speedMode) {
    // --- 1. GLOBAL SYNC TIMER (Mbed OS Native) ---
    // Create a 2Hz flash (500ms period) using standard millis().
    uint32_t currentTick = millis();
    bool flashOn = (currentTick % 500) < 250;

    // --- 2. INDICATOR LOGIC (Strict Table Compliance) ---
    // Reset all pins first for a clean state
    digitalWrite(PIN_LED_FAULT, LOW);  // Red
    digitalWrite(PIN_LED_MODE, LOW);   // Yellow
    digitalWrite(PIN_LED_STATUS, LOW); // Green

    switch(currentState) {
        case INIT_STATE:
            if (flashOn) digitalWrite(PIN_LED_FAULT, HIGH);  // Flashing Red
            break;

        case IDLE_STATE:
            digitalWrite(PIN_LED_MODE, HIGH);               // Solid Yellow
            break;

        case MANUAL_STATE:
            digitalWrite(PIN_LED_STATUS, HIGH);              // Solid Green
            break;

        case AUTONOMOUS_STATE:
            if (flashOn) digitalWrite(PIN_LED_STATUS, HIGH); // Flashing Green
            break;

        case FAULT_STATE:
            if (flashOn) digitalWrite(PIN_LED_MODE, HIGH);   // Flashing Yellow
            break;

        case ESTOP_STATE:
            digitalWrite(PIN_LED_FAULT, HIGH);               // Solid Red
            break;
    }

    // --- 3. OLED REFRESH ---
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);
    
    // Header based on State
    if(currentState == FAULT_STATE || currentState == ESTOP_STATE) {
        display.println("!! CRITICAL !!");
    } else {
        display.println(currentState == AUTONOMOUS_STATE ? "AUTO MODE" : "MANUAL");
    }

    display.setTextSize(1);
    
    // Replaced printf with standard print/println for Mbed compatibility
    display.print("\nRPM:   ");
    display.println(rpm, 1);
    
    display.print("STEER: ");
    display.println(steer);
    
    display.print("GEAR:  ");
    display.println(speedMode == 0 ? "LOW" : (speedMode == 2 ? "HIGH" : "MED"));
    
    if (currentState == MANUAL_STATE && getDMSHoldStartTime() > 0) {
        display.setCursor(0, 45);
        display.print("ENGAGING AUTO..."); 
        
        // Draw a small progress bar based on the 1000ms timer
        int progress = (millis() - getDMSHoldStartTime()) / 10; // 0 to 100
        
        // Integer math used instead of float (* 1.28) to prevent visual artifacts
        display.drawRect(0, 55, 128, 5, SSD1306_WHITE);
        display.fillRect(0, 55, (progress * 128) / 100, 5, SSD1306_WHITE);
    }
    
    // Moved to the bottom so the progress bar actually renders
    display.display(); 
}