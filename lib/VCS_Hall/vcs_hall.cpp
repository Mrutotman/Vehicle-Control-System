/* ==============================================================================
 * MODULE:        VCS_Hall
 * RESPONSIBILITY: Hall sensor interrupt handling and RPM calculation.
 * DESCRIPTION:   This module converts raw digital pulses from the motor's Hall 
 * sensors into a physical speed value. It uses hardware interrupts
 * on pins PB6, PB7, and PB8 to capture the timing of transitions.
 * RPM is calculated by measuring the 'Delta-T' between pulses and
 * averaging it over a 16-pole pair cycle.
 * HW RESOURCES:  External Interrupts (EXIT), Micros() Timer.
 * ============================================================================== */

#include "vcs_hall.h"

// Volatile variables because they are modified inside an interrupt
volatile uint32_t last_hall_time_us = 0;
volatile uint32_t hall_delta_t_us = 0;
volatile uint8_t current_hall_state = 0;

// Low-pass filter variable for smooth RPM reading
float filtered_rpm = 0.0;
const float LPF_ALPHA = 0.2; // Smoothing factor (0.0 to 1.0)

// The unified Interrupt Service Routine (ISR)
void hallSensorISR() {
    uint32_t current_time_us = micros();
    
    // Calculate time between transitions
    hall_delta_t_us = current_time_us - last_hall_time_us;
    last_hall_time_us = current_time_us;

    // Read the exact pin states (Inverted as per your verified v4 test)
    uint8_t a = !digitalRead(PIN_HALL_U);
    uint8_t b = !digitalRead(PIN_HALL_V);
    uint8_t c = !digitalRead(PIN_HALL_W);
    
    // Construct the 3-bit state
    current_hall_state = (c << 2) | (b << 1) | a;
}

void initHallSensors() {
    // Configure pins with pullups
    pinMode(PIN_HALL_U, INPUT_PULLUP);
    pinMode(PIN_HALL_V, INPUT_PULLUP);
    pinMode(PIN_HALL_W, INPUT_PULLUP);

    // Attach hardware interrupts to trigger on ANY logical change
    attachInterrupt(digitalPinToInterrupt(PIN_HALL_U), hallSensorISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_HALL_V), hallSensorISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_HALL_W), hallSensorISR, CHANGE);

    // Initial read to set the starting state
    uint8_t a = !digitalRead(PIN_HALL_U);
    uint8_t b = !digitalRead(PIN_HALL_V);
    uint8_t c = !digitalRead(PIN_HALL_W);
    current_hall_state = (c << 2) | (b << 1) | a;
    last_hall_time_us = micros();
}

uint8_t getHallState() {
    return current_hall_state;
}

float getMechanicalRPM() {
    // Prevent division by zero if motor is stopped
    if (hall_delta_t_us == 0 || (micros() - last_hall_time_us) > 100000) {
        filtered_rpm = 0.0; // Motor is essentially stopped (no transitions for > 100ms)
        return filtered_rpm;
    }

    // Convert delta T from microseconds to seconds
    float delta_t_sec = hall_delta_t_us / 1000000.0;

    // Calculate electrical RPM: 60 seconds / (6 transitions * delta_t)
    float rpm_elec = 60.0 / (HALL_TRANSITIONS_REV * delta_t_sec);

    // Calculate mechanical RPM
    float rpm_mech = rpm_elec / MOTOR_POLE_PAIRS;

    // Apply Low-pass filter for the Speed PI controller feedback
    filtered_rpm = (LPF_ALPHA * rpm_mech) + ((1.0 - LPF_ALPHA) * filtered_rpm);

    return filtered_rpm;
}