/* ==============================================================================
 * MODULE:        VCS_Throttle
 * RESPONSIBILITY: Secure Speed Control via Direct Mapping or QuickPID.
 * DESCRIPTION:   In MANUAL mode, maps the physical pedal to the controller.
 * In AUTO mode, uses QuickPID to maintain target RPM.
 * ============================================================================== */

#include "vcs_throttle.h"
#include "vcs_state_machine.h"

// PID Variables for Autonomous Speed Control
float speed_setpoint, speed_input, speed_output;
QuickPID speedPID(&speed_input, &speed_output, &speed_setpoint);

uint16_t current_throttle_adc = 0;
uint8_t current_pwm_duty = 0;

// ESP32 Hardware Timer Settings
const int throttlePwmChannel = 0;
const int throttlePwmResolution = 8; 

void initThrottle() {
    // 1. Hardware Setup
    ledcSetup(throttlePwmChannel, THROTTLE_PWM_FREQ, throttlePwmResolution);
    ledcAttachPin(PIN_THROTTLE_OUT, throttlePwmChannel);
    ledcWrite(throttlePwmChannel, 0); // Secure start at 0V

    // 2. QuickPID Configuration for Speed
    speedPID.SetTunings(SPEED_KP, SPEED_KI, 0.0f); // Usually PI is enough for speed
    speedPID.SetSampleTimeUs((1.0f / FREQ_SPEED_CTRL_HZ) * 1000000);
    speedPID.SetOutputLimits(MIN_PWM_DUTY, MAX_PWM_DUTY); 
    speedPID.SetMode(QuickPID::Control::automatic);
}

void updateThrottle(float measured_rpm, float target_rpm) {
    // Always read the physical pedal for safety overrides
    current_throttle_adc = analogRead(PIN_THROTTLE_IN);

    // --- SECURITY STATE CHECK ---
    if (currentState == MANUAL_STATE) {
        // DIRECT MAPPING: Use the physical pedal
        if (current_throttle_adc < THROTTLE_MIN_RUN) {
            current_pwm_duty = 0;
        } else {
            long mapped = map(current_throttle_adc, THROTTLE_MIN_RUN, THROTTLE_MAX_RUN, MIN_PWM_DUTY, MAX_PWM_DUTY);
            current_pwm_duty = (uint8_t)constrain(mapped, MIN_PWM_DUTY, MAX_PWM_DUTY);
        }
    } 
    else if (currentState == AUTONOMOUS_STATE) {
        // PID CONTROL: Use QuickPID to hit the target RPM
        speed_input = measured_rpm;
        speed_setpoint = target_rpm;
        
        speedPID.Compute();
        current_pwm_duty = (uint8_t)speed_output;

        // Security Clamp: Ensure PID doesn't do anything crazy
        current_pwm_duty = constrain(current_pwm_duty, MIN_PWM_DUTY, MAX_PWM_DUTY);
    } 
    else {
        // HARD LOCKOUT: Force 0 if in IDLE, FAULT, or ESTOP
        current_pwm_duty = 0;
    }

    // 3. Hardware Execution
    ledcWrite(throttlePwmChannel, current_pwm_duty);
}