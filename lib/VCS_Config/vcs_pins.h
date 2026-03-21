#ifndef VCS_PINS_H
#define VCS_PINS_H

#include <Arduino.h>

// ==============================================================================
// MODULE:        VCS_Pins (ESP32 WROOM 38-Pin Only)
// DESCRIPTION:   Centralized pin definitions mapped directly to system modules.
// ==============================================================================

// --- VCS_Displays ---
#define PIN_OLED_SDA          21  // I2C SDA for OLED Screen
#define PIN_OLED_SCL          22  // I2C SCL for OLED Screen
#define PIN_LED_STATUS        14  // External LED 1: System Status
#define PIN_LED_FAULT         12  // External LED 2: Fault/E-Stop (Boot Strapping Pin - Keep Pulled Low)
#define PIN_LED_MODE          2   // External LED 3: Mode indicator (Boot Strapping Pin - Keep Pulled Low)

// --- VCS_Comm ---
#define PIN_UART_RX           16  // Hardware Serial RX2 (Connected to RPi TX)
#define PIN_UART_TX           17  // Hardware Serial TX2 (Connected to RPi RX)

// --- VCS_Actuators ---
#define PIN_THROTTLE_IN       36  // Input: Physical Throttle Pedal (Analog VP Pin)
#define PIN_THROTTLE_OUT      18  // Output: PWM -> RC Filter/OpAmp -> Controller
#define PIN_LOWBRAKE_IN       4   // Input: Physical Brake Switch (Use INPUT_PULLUP)
#define PIN_LOWBRAKE_OUT      5   // Output: To Optocoupler -> Controller Low Brake wire
#define PIN_EMBUTTON          13  // Input: Emergency Stop Button (Use INPUT_PULLUP)
#define PIN_DMS_BUTTON        25  // Input: Driver Monitoring System Button (Use INPUT_PULLUP)

// --- VCS_Speed (Speed Control) ---
#define PIN_SPEED_SW_LOW      25  // Input: Physical 3-Pos Switch (Low Position)
#define PIN_SPEED_SW_HIGH     0   // Input: Physical 3-Pos Switch (High Position) - Note: Boot Strapping Pin
#define PIN_SPEED_LOW         19  // Output: To Optocoupler -> 3-Speed Low wire
#define PIN_SPEED_HIGH        23  // Output: To Optocoupler -> 3-Speed High wire

// --- VCS_Steering ---
#define PIN_STEER_PUL         26  // Output: Stepper Pulse
#define PIN_STEER_DIR         27  // Output: Stepper Direction
#define PIN_STEER_ENA         32  // Output: Stepper Enable
#define PIN_STEER_POT         34  // Input: 10-turn Potentiometer Analog Feedback (Input Only)

// --- VCS_Hall (Parallel Signals) ---
// NOTE: These are 5V/Battery signals. MUST use voltage dividers to drop to 3.3V!
#define PIN_HALL_SPEED        35  // Input: Tapped in parallel from Green Dashboard wire (Input Only)
#define PIN_HALL_U            39  // Input: Tapped in parallel from Motor Phase U (VN Pin)
#define PIN_HALL_V            33  // Input: Tapped in parallel from Motor Phase V
#define PIN_HALL_W            15  // Input: Tapped in parallel from Motor Phase W

#endif // VCS_PINS_H