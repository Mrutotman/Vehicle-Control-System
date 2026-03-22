#include <Arduino.h>
#include <mbed.h> // Mbed OS for Threads and Watchdog
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
#include "vcs_simulation.h"

using namespace rtos; // Access Mbed Threads
using namespace mbed; // Access Hardware Watchdog

// --- Mbed Thread Allocation ---
// osPriorityRealtime guarantees the 1kHz loop never misses a microsecond
Thread control_thread(osPriorityRealtime); 
Thread comm_thread(osPriorityHigh);        
Thread ui_thread(osPriorityNormal);        

// --- 1. CONTROL TASK (1 kHz / 1 ms) ---
// High-priority control loops for the 1500W driver and Stepper.
void ControlTask() {
    auto lastWakeTime = Kernel::Clock::now();

    for (;;) {
        // 1. Motor PID & Telemetry (Runs at 1000Hz)
        updateHallCalculations();
        updateThrottle(getMeasuredRPM(), getTargetRPM());

        // 2. Steering PID (Runs at 100Hz via divider to save CPU)
        static uint8_t steerDivider = 0;
        if (++steerDivider >= 10) {
            updateSteeringPID(getTargetSteering(), isAutonomousMode());
            steerDivider = 0;
        }

        // 3. SEM Safety: Pet the Hardware Watchdog
        // If this thread crashes or hangs, the Watchdog will reboot the entire car.
        Watchdog::get_instance().kick();
        
        // Exact 1ms precision sleep (Replaces FreeRTOS vTaskDelayUntil)
        ThisThread::sleep_until(lastWakeTime + std::chrono::milliseconds(1));
        lastWakeTime = Kernel::Clock::now();
    }
}

// --- 2. COMM & STATE TASK (100 Hz / 10 ms) ---
// Mid-priority task for polling safety switches and parsing Raspberry Pi packets.
void CommTask() {
    for (;;) {
        // Poll hardware switches
        updateEmButton();   
        updateLowBrake();   
        updateThreeSpeed(); 
        
        // Parse incoming UART packets from the Pi
        updateUART(); 
        
        // Audit system safety and transition states (Pass 0 for external faults for now)
        updateStateMachine(0); 
        
        ThisThread::sleep_for(std::chrono::milliseconds(10));
    }
}

// --- 3. UI & TELEMETRY TASK (20 Hz / 50 ms) ---
// Low-priority task. I2C screen drawing and UART writing are slow, 
// so they live here where they won't block the motor PID.
void UITask() {
    for (;;) {
        // Send the CRC16-protected payload to the Pi
        broadcastVehicleTelemetry();
        
        // Update OLED (using actual current_drive_mode, not the target from the Pi)
        updateDisplay(getMeasuredRPM(), getMeasuredSteering(), current_drive_mode);

        ThisThread::sleep_for(std::chrono::milliseconds(50));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Allow power to stabilize on boot
    Serial.println("--- VCS v1.3 SYSTEM BOOTING (NANO 33 BLE) ---");

    // 1. Hardware Module Initialization
    initState_Machine();
    initUART();
    initThrottle();
    initLowBrake();
    initEmButton();
    initSteering();
    initHallSensors();
    initThreeSpeed();
    initDisplay();

    // 2. Enable Mbed Hardware Watchdog (SEM Requirement)
    // If Watchdog::kick() isn't called for 2 seconds (e.g., if a thread deadlocks),
    // the nRF52840 processor will hardware reset instantly to cut motor power.
    Watchdog::get_instance().start(2000);

    // 3. Start Mbed OS Threads
    control_thread.start(ControlTask);
    comm_thread.start(CommTask);
    ui_thread.start(UITask);
}

void loop() {
    // In Mbed OS, the standard Arduino loop() acts as a background idle thread.
    // We leave it mostly empty, running a slow diagnostic tick.
    ThisThread::sleep_for(std::chrono::milliseconds(1000));
}