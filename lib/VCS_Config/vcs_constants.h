#ifndef VCS_CONSTANTS_H
#define VCS_CONSTANTS_H

#include <Arduino.h>

#define ENABLE_MOCK_SYSTEM 1 // 1 = Simulation Mode, 0 = Real Hardware

// ==========================================
// System Frequencies & Timing
// ==========================================
#define FREQ_SPEED_CTRL_HZ    1000  // Speed PI control loop at 1 kHz
#define FREQ_STEER_CTRL_HZ    100   // Steering PID control loop at 100 Hz
#define FREQ_PWM_HZ           20000 // Trapezoidal BLDC commutation PWM
#define DEADTIME_US           50    // 50µs dead-time to prevent shoot-through

// ==========================================
// Motor Specifications (Gogo Zion 1000W)
// ==========================================
#define MOTOR_POLE_PAIRS      16    // 16 pole pairs for the motor
#define HALL_TRANSITIONS_REV  6     // 6 Hall state transitions per electrical revolution

// ==========================================
// Control Loop Parameters (PI/PID Defaults)
// ==========================================
// Speed PI (Tuned for <5% overshoot, <100ms settling time)
#define SPEED_KP              1.0f  // TODO: Tune this PI proportional gain
#define SPEED_KI              0.1f  // TODO: Tune this PI integral gain
#define MAX_PWM_DUTY          255   // Max 8-bit PWM duty cycle
#define MIN_PWM_DUTY          0     // Min PWM duty cycle

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
// Throttle & Low Speed Test Settings 
// ==========================================
#define THROTTLE_DEADBAND     30    
#define THROTTLE_MIN_RUN      170   
#define THROTTLE_MAX_RUN      512   
#define MIN_STEP_DELAY        2000  
#define MAX_STEP_DELAY        800   
#define FORCED_START_DELAY    2500  
#define FORCED_START_CYCLES   12    

// ==========================================
// External Array Declarations
// ==========================================
// Verified Hall to Phase Mapping
extern const uint8_t hall_to_phase_correct[8];

#endif // VCS_CONSTANTS_H