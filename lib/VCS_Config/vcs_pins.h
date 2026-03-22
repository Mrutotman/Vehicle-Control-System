#ifndef VCS_PINS_H
#define VCS_PINS_H

#include <Arduino.h>

// ==============================================================================
// MODULE:        VCS_Pins (Arduino Nano 33 BLE)
// DESCRIPTION:   Centralized pin definitions mapped directly to system modules.
// ==============================================================================

// --- VCS_Displays ---
#define PIN_OLED_SDA          A4  // I2C SDA
#define PIN_OLED_SCL          A5  // I2C SCL
#define PIN_LED_STATUS        D11 // External LED 1
#define PIN_LED_FAULT         D12 // External LED 2
#define PIN_LED_MODE          D13 // Onboard LED / External LED 3

// --- VCS_Comm ---
// Note: On Nano 33 BLE, Serial1 is mapped to hardware pins D0 and D1.
#define PIN_UART_RX           0   // D0 (RX)
#define PIN_UART_TX           1   // D1 (TX)

// --- VCS_Actuators ---
#define PIN_THROTTLE_IN       A1  // Input: Physical Throttle Pedal
#define PIN_THROTTLE_OUT      9   // D9 (PWM capable)
#define PIN_LOWBRAKE_IN       3   // D3 (Use INPUT_PULLUP)
#define PIN_LOWBRAKE_OUT      8   // D8 (To Optocoupler)
#define PIN_EMBUTTON          4   // D4 (Use INPUT_PULLUP)
#define PIN_DMS_BUTTON        2   // D2 (Use INPUT_PULLUP)

// --- VCS_Speed (Speed Control) ---
#define PIN_SPEED_SW_LOW      A2  // Input: 3-Pos Switch Low
#define PIN_SPEED_SW_HIGH     A3  // Input: 3-Pos Switch High
#define PIN_SPEED_LOW         A6  // Output: To Optocoupler
#define PIN_SPEED_HIGH        A7  // Output: To Optocoupler

// --- VCS_Steering ---
#define PIN_STEER_PUL         5   // D5
#define PIN_STEER_DIR         6   // D6
#define PIN_STEER_ENA         7   // D7
#define PIN_STEER_POT         A0  // Input: 10-turn Potentiometer

// --- VCS_Hall (Parallel Signals) ---
// NOTE: MUST use voltage dividers to drop 5V/Battery signals to 3.3V!
#define PIN_HALL_SPEED        10  // D10
#define PIN_HALL_U            A1  // *Reassign if using motor phases*
#define PIN_HALL_V            A2  // *Reassign if using motor phases*
#define PIN_HALL_W            A3  // *Reassign if using motor phases*

#endif // VCS_PINS_H