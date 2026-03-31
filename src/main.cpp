#include <Arduino.h>
#include <mbed.h> // Mbed OS for Threads and Watchdog
#include "vcs_pins.h"
#include "vcs_constants.h"
#include "vcs_state_machine.h"
#include "vcs_uart.h"
#include "vcs_throttle.h"
#include "vcs_lowbrake.h"
// #include "vcs_embutton.h"  <-- [REMOVED] Replaced by dual dead-man switches
#include "vcs_deadman.h"      // [ADDED] Dual Hand Dead-Man Switch Logic
#include "vcs_relays.h"       // [ADDED] 12V Strobe & Organizer State Relay Logic
#include "vcs_steering.h"
#include "vcs_hallsensor.h"
#include "vcs_threespeed.h"
#include "vcs_reverse.h"
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
        Watchdog::get_instance().kick();
        
        // Exact 1ms precision sleep
        ThisThread::sleep_until(lastWakeTime + std::chrono::milliseconds(1));
        lastWakeTime = Kernel::Clock::now();
    }
}

// --- 2. COMM & STATE TASK (100 Hz / 10 ms) ---
void CommTask() {
    for (;;) {
        // Poll hardware switches
        updateDeadman();    // [ADDED] Polls Left/Right grips for active AND gate
        updateLowBrake();   
        updateThreeSpeed(); // [UPDATED] Now applies software throttle limits
        updateReverse();
        
        // Parse incoming UART packets from the Pi
        updateUART(); 
        
        // Audit system safety and transition states
        updateStateMachine(0); 

        // Trigger hardware relays based on current state
        updateRelays(isAutonomousMode());     // [ADDED] Fires the Orange Strobe & Organizer NO/NC Relay
        
        ThisThread::sleep_for(std::chrono::milliseconds(10));
    }
}

// --- 3. UI & TELEMETRY TASK (20 Hz / 50 ms) ---
void UITask() {
    for (;;) {
        // Send the CRC16-protected payload to the Pi
        broadcastVehicleTelemetry();
        
        // Update OLED
        updateDisplay(getMeasuredRPM(), getMeasuredSteering(), current_drive_mode);

        ThisThread::sleep_for(std::chrono::milliseconds(50));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Allow power to stabilize on boot
    Serial.println("--- VCS v1.5 SEM AUTONOMOUS BOOTING (NANO 33 BLE) ---"); // [UPDATED]

    // 1. Hardware Module Initialization
    initState_Machine();
    initUART();
    initThrottle();
    initLowBrake();
    initDeadman();   // [ADDED]
    initRelays();    // [ADDED]
    initSteering();
    initHallSensors();
    initThreeSpeed();
    initReverse(); 
    initDisplay();

    // 2. Enable Mbed Hardware Watchdog (SEM Requirement)
    Watchdog::get_instance().start(2000);

    // 3. Start Mbed OS Threads
    control_thread.start(ControlTask);
    comm_thread.start(CommTask);
    ui_thread.start(UITask);
}

void loop() {
    // In Mbed OS, the standard Arduino loop() acts as a background idle thread.
    ThisThread::sleep_for(std::chrono::milliseconds(1000));
}