#include <Arduino.h>
#include <esp_task_wdt.h>
#include "vcs_pins.h"
#include "vcs_constants.h"
#include "vcs_state_machine.h"
#include "vcs_uart.h"
#include "vcs_throttle.h"
#include "vcs_lowbrake.h"
#include "vcs_embutton.h"
#include "vcs_steering.h"
#include "vcs_hallsensor.h"
#include "vcs_threespeed.h"
#include "vcs_display.h"

// Task Handles
TaskHandle_t ControlTaskHandle;
TaskHandle_t CommTaskHandle;
TaskHandle_t UITaskHandle;

// --- 1. CONTROL TASK (1 kHz / 1 ms) ---
// Runs on Core 1 to stay away from WiFi/BT/UART interrupts
void ControlTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1); // 1ms for 1kHz

    for (;;) {
        // Precise timing for Speed PI
        updateHallCalculations();
        updateThrottle(getMeasuredRPM(), getTargetRPM());

        // Steering runs every 10th cycle (100Hz)
        static uint8_t steerDivider = 0;
        if (++steerDivider >= 10) {
            updateSteeringPID(getTargetSteering(), isAutonomousMode());
            steerDivider = 0;
        }

        // Reset the Task Watchdog for this specific task
        esp_task_wdt_reset();
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// --- 2. COMM & STATE TASK (100 Hz / 10 ms) ---
void CommTask(void *pvParameters) {
    for (;;) {
        updateUART(); // Read from Pi
        updateStateMachine(0); // Audit system state
        updateLowBrake(); // Manage safety brake
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// --- 3. UI & TELEMETRY TASK (10 Hz / 100 ms) ---
void UITask(void *pvParameters) {
    for (;;) {
        // Send telemetry to Pi at 20Hz
        broadcastVehicleTelemetry();

        // Update the physical OLED
        updateDisplay(getMeasuredRPM(), getMeasuredSteering(), getRPiCommandMode());

        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Give the serial monitor time to connect
    Serial.println("--- VCS SYSTEM BOOTING ---");

    // 1. Hardware Init (Serial, Pins, I2C)

    // Initializing high-level architecture
    initState_Machine();
    initUART();
    
    // Actuators & Drive Logic
    initThrottle();
    initLowBrake();
    initEmButton();
    initSteering();
    initHallSensors();
    initThreeSpeed();
    initDisplay();

    // 2. Initialize Task Watchdog (WDT)
    esp_task_wdt_init(2, true); // 2 second timeout

    // 3. Create Tasks
    xTaskCreatePinnedToCore(
        ControlTask, "Control", 4096, NULL, 10, &ControlTaskHandle, 1);
    
    xTaskCreatePinnedToCore(
        CommTask, "Comm", 4096, NULL, 5, &CommTaskHandle, 0);

    xTaskCreatePinnedToCore(
        UITask, "UI", 4096, NULL, 1, &UITaskHandle, 0);

    // Add the most critical task to the watchdog
    esp_task_wdt_add(ControlTaskHandle);
}

void loop() {
    // In FreeRTOS, the Arduino loop() is a low-priority task. 
    // We leave it empty or use it for background system monitoring.
    vTaskDelete(NULL); 
}