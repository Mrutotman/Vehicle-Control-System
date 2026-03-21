#ifndef VCS_CONSTANTS_H
#define VCS_CONSTANTS_H

#include <Arduino.h>

#define ENABLE_MOCK_SYSTEM 1 // 1 = Simulation Mode, 0 = Real Hardware

// ==========================================
// System Frequencies & Timing
// ==========================================
#define FREQ_SPEED_CTRL_HZ    100   // Speed PI control loop (Reduced to 100Hz, e-bike controller response is mechanical)
#define FREQ_STEER_CTRL_HZ    100   // Steering PID control loop at 100 Hz
#define THROTTLE_PWM_FREQ     5000  // 5kHz PWM frequency for the RC filter/OpAmp to smooth into analog DC
#define DEBOUNCE_TIME_MS      50    // 50ms standard debounce for E-Stop and Brake switches

// ==========================================
// Motor Specifications (Gogo Zion 1000W)
// ==========================================
// Kept for RPM calculation from parallel Hall sensor taps
#define MOTOR_POLE_PAIRS      16    // 16 pole pairs for the motor
#define HALL_TRANSITIONS_REV  6     // 6 Hall state transitions per electrical revolution

// ==========================================
// Control Loop Parameters (PI/PID Defaults)
// ==========================================
// Speed PI (Tuned for <5% overshoot, <100ms settling time)
#define SPEED_KP              1.0f  // TODO: Tune this PI proportional gain
#define SPEED_KI              0.1f  // TODO: Tune this PI integral gain
#define MAX_PWM_DUTY          255   // Max 8-bit PWM duty cycle (Maps to ~4.2V out of OpAmp)
#define MIN_PWM_DUTY          0     // Min PWM duty cycle (Maps to ~0.8V out of OpAmp)

// Steering PID (Tuned for <2° steady-state error)
#define STEER_KP              1.0f  // TODO: Tune this PID proportional gain
#define STEER_KI              0.0f  // TODO: Tune this PID integral gain
#define STEER_KD              0.0f  // TODO: Tune this PID derivative gain

// ==========================================
// Communication Protocol Ranges (ANS -> VCS)
// ==========================================
#define COMM_SPEED_MIN       -1000  // Minimum target speed RPM
#define COMM_SPEED_MAX        1000  // Maximum target speed RPM
#define COMM_STEER_LEFT       0     // Maximum left steering position
#define COMM_STEER_CENTER     500   // Center steering position
#define COMM_STEER_RIGHT      1000  // Maximum right steering position
#define COMM_BRAKE_MIN        0     // Brake off (0%)
#define COMM_BRAKE_MAX        100   // Brake full (100%)

// ==========================================
// Throttle Ranges (Mapped for ESP32 12-bit ADC)
// ==========================================
// Note: ESP32 ADC goes from 0 to 4095
#define THROTTLE_DEADBAND     150   // Ignore slight pedal touches
#define THROTTLE_MIN_RUN      400   // ADC value where motor actually starts moving
#define THROTTLE_MAX_RUN      3800  // Max practical ADC value (ESP32 ADC is non-linear at the very top)

#endif // VCS_CONSTANTS_H