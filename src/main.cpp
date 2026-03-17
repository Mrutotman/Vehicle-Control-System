#include <Arduino.h>

// Include all VCS modules
#include "vcs_pins.h"
#include "vcs_constants.h"
#include "vcs_hall.h"
#include "vcs_commutation.h"
#include "vcs_speed_controller.h"
#include "vcs_steering.h"
#include "vcs_uart.h"
#include "vcs_state_machine.h"
#include "vcs_safety.h"
#include "vcs_brake.h"
uint8_t current_pwm_duty = 0;
bool debugModeActive = true; // Set to true to see text, false for RPi hex
void printTelemetryAsText(float rpm, int steer, float voltage, uint8_t state);


// ==============================================================================
// 1. FREERTOS IMPLEMENTATION (For Arduino Nano 33 BLE)
// ==============================================================================
#if defined(ARDUINO_ARCH_NRF52840)

// The Nano 33 BLE uses Mbed OS. 
// We use the Mbed Threading and Ticker APIs which fulfill the SIDLAK RTOS requirement.
#include "mbed.h"
#include "rtos.h"
#include "vcs_state_machine.h" // Ensure this is included to use isAutonomousMode()

using namespace std::chrono;
using namespace mbed;
using namespace rtos;

// We use Mbed Threads instead of xTaskCreate
Thread threadSpeedControl;
Thread threadSteeringSafety;
Thread threadUART;

void taskSpeedControl() {
    while (true) {
        if (currentState == AUTONOMOUS_STATE) {
            current_pwm_duty = updateSpeedController(getTargetRPM(), getMechanicalRPM());
        } else if (currentState == MANUAL_STATE) {
            int throttle = analogRead(PIN_THROTTLE);
            current_pwm_duty = (throttle > THROTTLE_MIN_RUN) ? 
                map(throttle, THROTTLE_MIN_RUN, THROTTLE_MAX_RUN, MIN_PWM_DUTY, MAX_PWM_DUTY) : 0;
            current_pwm_duty = constrain(current_pwm_duty, MIN_PWM_DUTY, MAX_PWM_DUTY);
        } else {
            current_pwm_duty = 0;
            initSpeedController();
        }
        ThisThread::sleep_for(1ms); // 1 kHz
    }
}

void taskSteeringAndSafety() {
    while (true) {
        updateStateMachine();
        
        // Grab the auto state to determine if the stepper shaft should be locked or free
        bool auto_mode = isAutonomousMode();

        if (currentState == AUTONOMOUS_STATE) {
            updateSteeringPID(getTargetSteering(), auto_mode);
            applyBrake(getTargetBrake());
        } else if (currentState == MANUAL_STATE) {
            updateSteeringPID(COMM_STEER_CENTER, auto_mode); 
            applyBrake(0);
        } else {
            applyBrake(100); 
            updateSteeringPID(getMeasuredSteering(), auto_mode); 
        }
        resetHardwareWatchdog();
        ThisThread::sleep_for(10ms); // 100 Hz
    }
}

void taskUARTComms() {
    while (true) {
        updateUART();
        ThisThread::sleep_for(50ms); // 20 Hz
    }
}

void setup() {
    initSafety();
    initBrake();
    initCommutation();
    initHallSensors();
    initSpeedController();
    initSteering();
    initUART();
    initState_Machine();

    // Start Mbed Threads
    threadSpeedControl.start(taskSpeedControl);
    threadSteeringSafety.start(taskSteeringAndSafety);
    threadUART.start(taskUARTComms);
}

void loop() {}

// ==============================================================================
// 2. MICROS() SCHEDULER IMPLEMENTATION (For STM32 and 8-bit Nano)
// ==============================================================================
#else

// Timing tracking
uint32_t last_1kHz_us = 0;
uint32_t last_100Hz_us = 0;
uint32_t last_10Hz_us = 0;
uint32_t last_20Hz_us = 0;
uint8_t last_hall_state = 0;

void setup() {
    initSafety();
    initBrake();
    initCommutation();
    initHallSensors();
    initSpeedController();
    initSteering();
    initUART();
    initState_Machine();
}

void loop() {
    uint32_t current_us = micros();

    // 1. Always update the state machine first
    updateStateMachine();
    
    // 2. Get the current target position (from RC or RPi depending on state)
    uint16_t target_steer = getTargetSteering(); // Assuming you have a function like this
    
    // 3. Check the state and update the steering motor
    bool auto_mode = isAutonomousMode(); 
    
    // If auto_mode is true: Stepper is Locked & Active.
    // If auto_mode is false: Stepper is Unlocked & Power Saving.
    updateSteeringPID(target_steer, auto_mode);

    // FASTEST LOOP: Run continuously to catch state changes
    uint8_t hall = getHallState();
    if (hall != last_hall_state) {
        last_hall_state = hall;
        if ((currentState == AUTONOMOUS_STATE || currentState == MANUAL_STATE) && (hall > 0 && hall < 7)) {
            uint8_t phase = hall_to_phase_correct[hall] - 1;
            applyDeadtime();
            commutatePhase(phase, current_pwm_duty);
        } else {
            allPhasesOff();
        }
    }

    // 1 kHz TASK: Speed PI Control
    if (current_us - last_1kHz_us >= 1000) {
        last_1kHz_us = current_us;
        if (currentState == AUTONOMOUS_STATE) {
            current_pwm_duty = updateSpeedController(getTargetRPM(), getMechanicalRPM());
        } else if (currentState == MANUAL_STATE) {
            int throttle = analogRead(PIN_THROTTLE);
            current_pwm_duty = (throttle > THROTTLE_MIN_RUN) ? 
                map(throttle, THROTTLE_MIN_RUN, THROTTLE_MAX_RUN, MIN_PWM_DUTY, MAX_PWM_DUTY) : 0;
            current_pwm_duty = constrain(current_pwm_duty, MIN_PWM_DUTY, MAX_PWM_DUTY);
        } else {
            current_pwm_duty = 0;
            initSpeedController();
        }
    }

    // 10 Hz TASK: Datalogging / Telemetry
    static uint32_t last_10Hz_ms = 0;
    if (millis() - last_10Hz_ms >= 100) {
        last_10Hz_ms = millis();

        float current_v = getBatteryVoltage();

        if (debugModeActive) {
            printTelemetryAsText(getMechanicalRPM(), getMeasuredSteering(), current_v, currentState);
        } else {
            sendTelemetry(getMechanicalRPM(), getMeasuredSteering(), current_v, currentState);
        }
    }

    // 100 Hz TASK: Steering, Safety, & Watchdog
    if (current_us - last_100Hz_us >= 10000) {
        last_100Hz_us = current_us;
        updateStateMachine();
        
        if (currentState == AUTONOMOUS_STATE) {
            updateSteeringPID(getTargetSteering(), isAutonomousMode());;
            applyBrake(getTargetBrake());
        } else if (currentState == MANUAL_STATE) {
            updateSteeringPID(COMM_STEER_CENTER, isAutonomousMode());;
            applyBrake(0);
        } else {
            applyBrake(100);
            updateSteeringPID(getMeasuredSteering(), isAutonomousMode());
        }
        resetHardwareWatchdog();
    }

    // 20 Hz TASK: UART Comms
    if (current_us - last_20Hz_us >= 50000) {
        last_20Hz_us = current_us;
        updateUART();
    }
}

void printTelemetryAsText(float rpm, int steer, float voltage, uint8_t state) {
    Serial.print(F("--- VCS DATA ---"));
    Serial.print(F("\nSTATE:   ")); Serial.print(state);
    
    // Add text labels for the states
    switch(state) {
        case 0: Serial.print(F(" (INIT)")); break;
        case 1: Serial.print(F(" (IDLE)")); break;
        case 4: Serial.print(F(" (ESTOP)")); break;
        case 5: Serial.print(F(" (FAULT)")); break;
        default: Serial.print(F(" (UNKNOWN)")); break;
    }

    Serial.print(F("\nRPM:     ")); Serial.print(rpm);
    Serial.print(F("\nSTEER:   ")); Serial.print(steer);
    Serial.print(F("\nBATTERY: ")); Serial.print(voltage); 
    Serial.println(F("V"));
    Serial.println(F("----------------\n"));
}

#endif