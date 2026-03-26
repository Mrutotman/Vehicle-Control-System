#ifndef VCS_PINS_H
#define VCS_PINS_H

#include <Arduino.h>

// ==============================================================================
// MODULE:       VCS_Pins (Arduino Nano 33 BLE)
// DESCRIPTION:  Centralized pin definitions mapped directly to manual PCB V1.4.
// ==============================================================================

// --- VCS_Displays ---
#define PIN_OLED_SDA          A4  // I2C SDA
#define PIN_OLED_SCL          A5  // I2C SCL
#define PIN_LED_FAULT         A6  // Single External Dashboard LED (RED)

// --- VCS_Comm ---
#define PIN_UART_RX           0   // D0 (RX from RPi TX)
#define PIN_UART_TX           1   // D1 (TX to RPi RX)

// --- VCS_Actuators (Primary Drive) ---
#define PIN_THROTTLE_IN       A1  // Input: Physical Throttle Pedal (0-3.3V)
#define PIN_THROTTLE_OUT      9   // D9 Output: PWM to RC Filter & LM358 Op-Amp
#define PIN_LOWBRAKE_IN       8   // D8 Input: Physical Pedal Switch (INPUT_PULLUP)
#define PIN_LOWBRAKE_OUT      3   // D3 Output: To PC817 Optocoupler (Galvanic Isolation)

// --- VCS_Safety_Switches ---
#define PIN_EMBUTTON          4   // D4 Input: E-Stop Switch (INPUT_PULLUP)
#define PIN_DMS_BUTTON        2   // D2 Input: Dead Man's Switch (INPUT_PULLUP)

// --- VCS_Transmission (Gears & Reverse) ---
#define PIN_SPEED_SW_LOW      A3  // Input: Physical 3-Pos Switch (Low)
#define PIN_SPEED_SW_HIGH     A7  // Input: Physical 3-Pos Switch (High)
#define PIN_SPEED_LOW         12  // D12 Output: To Shifter -> Controller Brown Wire
#define PIN_SPEED_HIGH        11  // D11 Output: To Shifter -> Controller Blue Wire
#define PIN_REVERSE_IN        A2  // Input: Driver's Reverse Switch (INPUT_PULLUP)
#define PIN_REVERSE_OUT       13  // Output: To Shifter -> Controller Yellow Wire

// --- VCS_Steering ---
#define PIN_STEER_PUL         5   // D5 Output: To Shifter -> Stepper PUL+
#define PIN_STEER_DIR         6   // D6 Output: To Shifter -> Stepper DIR+
#define PIN_STEER_ENA         7   // D7 Output: To Shifter -> Stepper ENA+
#define PIN_STEER_POT         A0  // Input: 10-turn Steering Potentiometer

// --- VCS_Sensors ---
#define PIN_HALL_SPEED        10  // D10 Input: From Shifter <- Controller Yellow Wire

#endif // VCS_PINS_H