#include "vcs_throttle.h"

// Global Telemetry Variables
uint16_t current_throttle_adc = 0;
uint16_t current_pwm_duty = 0;

// PID Variables
float measured_rpm = 0.0f;
float target_rpm = 0.0f;
float throttle_pwm_out = 0.0f;

// Initialize QuickPID
QuickPID speedPID(&measured_rpm, &throttle_pwm_out, &target_rpm);

void initThrottle() {
    pinMode(PIN_THROTTLE_OUT, OUTPUT);
    pinMode(PIN_THROTTLE_IN, INPUT);

    // Sync Nano 33 BLE hardware to the 10-bit standard (0-1023)
    analogReadResolution(10);
    analogWriteResolution(10); 

    // Apply constants from vcs_constants.h
    speedPID.SetTunings(SPEED_KP, SPEED_KI, 0.0f);
    speedPID.SetOutputLimits(MIN_PWM_OUT, MAX_PWM_OUT);
    
    // CRITICAL: Sync QuickPID to our 1kHz Mbed ControlTask thread
    speedPID.SetSampleTimeUs(1000); 
    
    // Start in manual to prevent accidental windup on boot
    speedPID.SetMode(QuickPID::Control::manual);
}


void updateThrottle(float current_rpm_in, float target_rpm_in) {
    // Always read the pedal ADC for telemetry, regardless of state
    current_throttle_adc = analogRead(PIN_THROTTLE_IN);

    // --- 1. HARDWARE SAFETY LOCKOUT ---
    // If the state machine says we shouldn't be driving (e.g., FAULT, ESTOP, IDLE),
    // we hard-kill the PWM and reset the PID to prevent integral windup.
    if (currentState != AUTONOMOUS_STATE && currentState != MANUAL_STATE) {
        current_pwm_duty = MIN_PWM_OUT;
        analogWrite(PIN_THROTTLE_OUT, current_pwm_duty);
        
        speedPID.SetMode(QuickPID::Control::manual);
        throttle_pwm_out = MIN_PWM_OUT;
        return;
    }

    // --- 2. AUTONOMOUS CONTROL (PID) ---
    if (currentState == AUTONOMOUS_STATE) {
        // Re-engage PID if we just transitioned from Manual
        if (speedPID.GetMode() == (uint8_t)QuickPID::Control::manual) {
            speedPID.SetMode(QuickPID::Control::automatic);
        }
        
        measured_rpm = current_rpm_in;
        target_rpm = target_rpm_in;
        
        // Compute() evaluates true if 1000us has passed
        if (speedPID.Compute()) {
            current_pwm_duty = (uint16_t)throttle_pwm_out;
            analogWrite(PIN_THROTTLE_OUT, current_pwm_duty);
        }
    } 
    // --- 3. MANUAL CONTROL (Pass-Through) ---
    else if (currentState == MANUAL_STATE) {
        int mapped_pwm = MIN_PWM_OUT;
        
        // Deadband logic: Ignore slight touches or sensor noise
        if (current_throttle_adc > THROTTLE_MIN_INPUT) {
            mapped_pwm = map(current_throttle_adc, THROTTLE_MIN_INPUT, THROTTLE_MAX_INPUT, MIN_PWM_OUT, MAX_PWM_OUT);
            mapped_pwm = constrain(mapped_pwm, MIN_PWM_OUT, MAX_PWM_OUT);
        }
        
        current_pwm_duty = mapped_pwm;
        analogWrite(PIN_THROTTLE_OUT, current_pwm_duty);
        
        // BUMPLESS TRANSFER: 
        // Feed the manual PWM value back into the PID while in manual mode. 
        // If the driver hits the DMS to engage Auto, the car won't jerk forward.
        throttle_pwm_out = mapped_pwm;
        speedPID.SetMode(QuickPID::Control::manual);
    }
}