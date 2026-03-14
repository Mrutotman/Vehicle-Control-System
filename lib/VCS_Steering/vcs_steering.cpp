/* ==============================================================================
 * MODULE:        VCS_Steering
 * RESPONSIBILITY: Closed-loop Steering Position Control.
 * DESCRIPTION:   Maintains the front wheel angle by comparing the Target Angle 
 * from the RPi against the feedback from a 10-turn potentiometer.
 * Uses a PID algorithm to drive the steering actuator motor.
 * Includes 'Center' calibration logic to ensure 500 = straight.
 * HW RESOURCES:  ADC1 (PA0), PWM (PA6), DIR (PA5).
 * ============================================================================== */

#include "vcs_steering.h"

// Internal PID state
static float sum_error = 0.0f;
static float last_error = 0.0f;

// Sample time for 100 Hz loop
const float Ts_s = 1.0f / FREQ_STEER_CTRL_HZ; // 0.01 seconds

void initSteering() {
    pinMode(PIN_STEER_POT, INPUT);
    pinMode(PIN_STEER_PWM, OUTPUT);
    pinMode(PIN_STEER_DIR, OUTPUT);
    
    // Ensure motor is stopped on boot
    analogWrite(PIN_STEER_PWM, 0);
    
    sum_error = 0.0f;
    last_error = 0.0f;
}

uint16_t getMeasuredSteering() {
    // Read the 10-turn pot. 
    // STM32 ADC is 12-bit (0-4095). We map it to the ANS standard (0-1000).
    int raw_adc = analogRead(PIN_STEER_POT);
    
    // Map ADC resolution to the 0-1000 scale expected by the protocol
    // Adjust the 4095 below to 1023 if testing on an 8-bit Arduino Nano
    int mapped_pos = map(raw_adc, 0, 4095, COMM_STEER_LEFT, COMM_STEER_RIGHT);
    
    return (uint16_t)constrain(mapped_pos, COMM_STEER_LEFT, COMM_STEER_RIGHT);
}

void updateSteeringPID(uint16_t target_position) {
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
    
    // 5. Total Output
    float u_steer = u_p + u_i + u_d;
    
    // 6. Convert to PWM and Direction
    uint8_t pwm_out = 0;
    
    if (u_steer > 0) {
        // Steer Right
        digitalWrite(PIN_STEER_DIR, HIGH);
        pwm_out = (uint8_t)constrain(u_steer, 0, MAX_PWM_DUTY);
    } else {
        // Steer Left
        digitalWrite(PIN_STEER_DIR, LOW);
        // Invert negative output for PWM magnitude
        pwm_out = (uint8_t)constrain(-u_steer, 0, MAX_PWM_DUTY); 
    }
    
    // Apply deadband to prevent micro-oscillations near steady-state
    if (abs(e_steer) < 5) { // Tuning parameter for < 2 degree error
        pwm_out = 0;
    }
    
    // Actuate motor
    analogWrite(PIN_STEER_PWM, pwm_out);
}