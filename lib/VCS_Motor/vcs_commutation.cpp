/* ==============================================================================
 * MODULE:        VCS_Commutation
 * RESPONSIBILITY: 6-Step Trapezoidal BLDC Commutation.
 * DESCRIPTION:   Directly controls the Power Stage MOSFETs. Based on the 3-bit 
 * Hall state, it enables the correct High-Side and Low-Side gates
 * to pull the motor through its rotation. It includes a safety 
 * dead-time function to prevent phase shoot-through and supports
 * a 'Coast' (all phases off) mode for safety.
 * HW RESOURCES:  PA8, PA9, PA10 (High PWM) | PB13, PB14, PB15 (Low GPIO).
 * ============================================================================== */

#include "vcs_commutation.h"

// Actual definition of the verified Hall mapping
const uint8_t hall_to_phase_correct[8] = {
  0,  // 000 - Invalid
  2,  // 001 - Phase 2
  4,  // 010 - Phase 4
  3,  // 011 - Phase 3
  6,  // 100 - Phase 6
  1,  // 101 - Phase 1
  5,  // 110 - Phase 5
  0   // 111 - Invalid
};

void initCommutation() {
  // Initialize MOSFET control pins as outputs
  pinMode(PIN_MOTOR_AH, OUTPUT);
  pinMode(PIN_MOTOR_AL, OUTPUT);
  pinMode(PIN_MOTOR_BH, OUTPUT);
  pinMode(PIN_MOTOR_BL, OUTPUT);
  pinMode(PIN_MOTOR_CH, OUTPUT);
  pinMode(PIN_MOTOR_CL, OUTPUT);
  
  // Start with ALL MOSFETS OFF for safety
  allPhasesOff();
}

void allPhasesOff() {
  digitalWrite(PIN_MOTOR_AH, LOW);
  digitalWrite(PIN_MOTOR_AL, LOW);
  digitalWrite(PIN_MOTOR_BH, LOW);
  digitalWrite(PIN_MOTOR_BL, LOW);
  digitalWrite(PIN_MOTOR_CH, LOW);
  digitalWrite(PIN_MOTOR_CL, LOW);
} // [cite: 352, 353]

void applyDeadtime() {
  allPhasesOff();
  delayMicroseconds(DEADTIME_US);
} // [cite: 352]

// =============== YOUR VERIFIED COMMUTATION PHASES ===============
void commutatePhase(uint8_t phase, uint8_t pwm_duty) {
  switch(phase) {
    case 0: // Phase 1: A_LOW + B_HIGH (Hall 101) 
      digitalWrite(PIN_MOTOR_AL, HIGH);
      analogWrite(PIN_MOTOR_BH, pwm_duty);
      break;
      
    case 1: // Phase 2: A_LOW + C_HIGH (Hall 001) 
      digitalWrite(PIN_MOTOR_AL, HIGH);
      analogWrite(PIN_MOTOR_CH, pwm_duty);
      break;
      
    case 2: // Phase 3: B_LOW + C_HIGH (Hall 011) 
      digitalWrite(PIN_MOTOR_BL, HIGH);
      analogWrite(PIN_MOTOR_CH, pwm_duty);
      break;
      
    case 3: // Phase 4: A_HIGH + B_LOW (Hall 010) 
      analogWrite(PIN_MOTOR_AH, pwm_duty);
      digitalWrite(PIN_MOTOR_BL, HIGH);
      break;
      
    case 4: // Phase 5: A_HIGH + C_LOW (Hall 110) 
      analogWrite(PIN_MOTOR_AH, pwm_duty);
      digitalWrite(PIN_MOTOR_CL, HIGH);
      break;
      
    case 5: // Phase 6: B_HIGH + C_LOW (Hall 100) 
      analogWrite(PIN_MOTOR_BH, pwm_duty);
      digitalWrite(PIN_MOTOR_CL, HIGH);
      break;
  }
}