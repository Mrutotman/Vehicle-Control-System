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
    display.setTextColor(WHITE);
}

void updateDisplay(float rpm, uint16_t steer, uint8_t speedMode) {
    // --- 1. GLOBAL SYNC TIMER (FreeRTOS Native) ---
    // Create a 2Hz flash (500ms period). 
    // pdMS_TO_TICKS ensures this works regardless of your RTOS tick rate.
    TickType_t currentTick = xTaskGetTickCount();
    bool flashOn = (currentTick % pdMS_TO_TICKS(500)) < pdMS_TO_TICKS(250);

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
    display.printf("\nRPM:   %.1f\n", rpm);
    display.printf("STEER: %d\n", steer);
    display.printf("GEAR:  %s\n", (speedMode == 0 ? "LOW" : (speedMode == 2 ? "HIGH" : "MED")));
    
    display.display();

    if (currentState == MANUAL_STATE && getDMSHoldStartTime() > 0) {
    display.setCursor(0, 45);
    display.print("ENGAGING AUTO..."); 
    
    // Draw a small progress bar based on the 1000ms timer
    int progress = (millis() - getDMSHoldStartTime()) / 10; // 0 to 100
    display.drawRect(0, 55, 128, 5, WHITE);
    display.fillRect(0, 55, progress * 1.28, 5, WHITE);
}
}