/* ==============================================================================
 * MODULE:        VCS_Steering
 * RESPONSIBILITY: Closed-loop Steering Position Control.
 * DESCRIPTION:    Maintains the front wheel angle by comparing the Target Angle 
 * from the RPi against the feedback from a 10-turn potentiometer.
 * Uses a PID algorithm to drive a STEPPER motor.
 * Includes 'Center' calibration logic to ensure 500 = straight.
 * HW RESOURCES:   ADC1 (PA0), PUL (PA6), DIR (PA5), ENA (PA4).
 * ============================================================================== */

#include "vcs_steering.h"

// Internal PID state
static float sum_error = 0.0f;
static float last_error = 0.0f;

// Sample time for 100 Hz loop
const float Ts_s = 1.0f / FREQ_STEER_CTRL_HZ; // 0.01 seconds

void initSteering() {
    pinMode(PIN_STEER_POT, INPUT);
    pinMode(PIN_STEER_PUL, OUTPUT);
    pinMode(PIN_STEER_DIR, OUTPUT);
    pinMode(PIN_STEER_ENA, OUTPUT); // Pin for Power Saving / Shaft Lock
    
    // Explicitly set ADC to 12-bit (0-4095) for 32-bit architectures.
    // AVR boards will ignore this and default to their native 10-bit (0-1023).
    #if defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_NRF52840) || defined(ARDUINO_ARCH_MBED)
        analogReadResolution(12);
    #endif

    // Ensure motor is stopped and driver is DISABLED on boot (Shaft Unlocked, saving power)
    digitalWrite(PIN_STEER_PUL, LOW);
    digitalWrite(PIN_STEER_ENA, LOW); // Note: For Common-Anode wiring, LOW = Disabled/Free
    
    sum_error = 0.0f;
    last_error = 0.0f;
}

uint16_t getMeasuredSteering() {
    // Read the 10-turn pot. 
    int raw_adc = analogRead(PIN_STEER_POT);
    
    // Map ADC resolution to the 0-1000 scale expected by the protocol
    // Automatically adjusts based on PlatformIO target architecture
    #if defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_NRF52840) || defined(ARDUINO_ARCH_MBED)
        // 12-bit ADC for STM32 and Nano 33 BLE
        int mapped_pos = map(raw_adc, 0, 4095, COMM_STEER_LEFT, COMM_STEER_RIGHT);
    #else
        // 10-bit ADC for standard 8-bit Arduino (AVR)
        int mapped_pos = map(raw_adc, 0, 1023, COMM_STEER_LEFT, COMM_STEER_RIGHT);
    #endif
    
    return (uint16_t)constrain(mapped_pos, COMM_STEER_LEFT, COMM_STEER_RIGHT);
}

void updateSteeringPID(uint16_t target_position, bool is_automatic) {
    uint16_t measured_position = getMeasuredSteering();
    
    // 1. Calculate Error
    float e_steer = (float)target_position - (float)measured_position;
    
    // 2. Proportional Term
    float u_p = STEER_KP * e_steer;
    
    // 3. Integral Term
    sum_error += e_steer * Ts_s;
    float u_i = STEER_KI * sum_error;
    
    // 4. Derivative Term 
    float u_d = STEER_KD * ((e_steer - last_error) / Ts_s);
    last_error = e_steer;
    
    // 5. Total Output (Effort)
    float u_steer = u_p + u_i + u_d;
    
    // ---AUTO / MANUAL MODE LOGIC---
    if (!is_automatic) {
        // Manual Mode: Turn OFF driver. Shaft is UNLOCKED and free to turn.
        // Current drops to 0.4A (or zero, depending on driver internal wiring).
        digitalWrite(PIN_STEER_ENA, LOW); 
        return; // Exit the function immediately so no pulses are sent
    } else {
        // Automatic Mode: Turn ON driver. Shaft is LOCKED (Holding torque applied).
        digitalWrite(PIN_STEER_ENA, HIGH); 
    }
    
    // 6. Convert to Stepper Direction and Magnitude
    uint8_t effort = 0;
    
    if (u_steer > 0) {
        // Steer Right
        digitalWrite(PIN_STEER_DIR, HIGH);
        effort = (uint8_t)constrain(u_steer, 0, MAX_PWM_DUTY);
    } else {
        // Steer Left
        digitalWrite(PIN_STEER_DIR, LOW);
        // Invert negative output for effort magnitude
        effort = (uint8_t)constrain(-u_steer, 0, MAX_PWM_DUTY); 
    }
    
    // Apply deadband to prevent micro-oscillations
    if (abs(e_steer) < 5) { 
        // We are at the target position. 
        // Do NOT turn off the driver here, so the shaft remains LOCKED in Automatic mode.
        return; 
    } 
    
    // Map PID effort to pulse delay. Higher effort = smaller delay (faster speed).
    int step_delay = map(effort, 1, MAX_PWM_DUTY, 1000, 35);
    
    // Security constraint: enforce strict 35us limit to prevent vibration/driver failure
    if (step_delay < 35) {
        step_delay = 35; 
    }

    // Map effort to how many steps to take in this 10ms loop cycle
    int steps_to_take = map(effort, 1, MAX_PWM_DUTY, 1, 40);

    // Generate the pulses for the stepper driver
    for (int i = 0; i < steps_to_take; i++) {
        digitalWrite(PIN_STEER_PUL, HIGH);
        delayMicroseconds(step_delay);
        digitalWrite(PIN_STEER_PUL, LOW);
        delayMicroseconds(step_delay);
    }
}