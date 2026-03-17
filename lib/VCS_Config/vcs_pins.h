#ifndef VCS_PINS_H
#define VCS_PINS_H

#include <Arduino.h>

// ==========================================
// ARCHITECTURE SPECIFIC PIN MAPPINGS
// ==========================================

#if defined(ARDUINO_ARCH_STM32)
    // --- STM32F103C8T6 (Blue Pill) ---
    #define PIN_HALL_U        PB6   // BLDC Motor Hall Sensor Phase U input
    #define PIN_HALL_V        PB7   // BLDC Motor Hall Sensor Phase V input
    #define PIN_HALL_W        PB8   // BLDC Motor Hall Sensor Phase W input
    
    #define PIN_MOTOR_AH      PA8   // Main BLDC High-Side Gate Drive Phase A
    #define PIN_MOTOR_BH      PA9   // Main BLDC High-Side Gate Drive Phase B
    #define PIN_MOTOR_CH      PA10  // Main BLDC High-Side Gate Drive Phase C
    #define PIN_MOTOR_AL      PB13  // Main BLDC Low-Side Gate Drive Phase A
    #define PIN_MOTOR_BL      PB14  // Main BLDC Low-Side Gate Drive Phase B
    #define PIN_MOTOR_CL      PB15  // Main BLDC Low-Side Gate Drive Phase C
    
    #define PIN_STEER_POT     PA0   // 10-turn Potentiometer (Steering angle feedback)
    #define PIN_STEER_PUL     PA6   // Stepper Driver PULSE signal (Speed/Steps)
    #define PIN_STEER_ENA     PB1   // Stepper Driver ENABLE signal (Auto-Lock/Manual-Free)
    #define PIN_STEER_DIR     PA5   // Stepper Driver DIRECTION signal (Left/Right)
    
    #define PIN_ESTOP_BRAKE   PA7   // Hardware Emergency Brake Actuator/Relay output
    
    #define PIN_THROTTLE      PA1   // Throttle Pedal/Input analog signal
    #define PIN_BRAKE_SW      PA11  // New: Digital Brake Pedal Switch (HIGH = Pressed, LOW = Released)
    #define PIN_RC_MODE_SW    PA4   // RC Controller Switch (Auto/Manual/Idle states)
    #define PIN_ESTOP_BTN     PB12  // Physical Emergency Stop Button input
    
    #define PIN_CURRENT_SENS  PA2   // Main Power Current Sensor analog input
    #define PIN_VOLTAGE_SENS  PA3   // Main Battery Voltage Divider analog input
    #define PIN_TEMP_SENS     PB0   // Motor/Heatsink Temperature Sensor analog input
    
    #define PIN_UART_RX       PB11  // Serial RX (Communication from Raspberry Pi)
    #define PIN_UART_TX       PB10  // Serial TX (Communication to Raspberry Pi)

#elif defined(ARDUINO_ARCH_NRF52840) || defined(ARDUINO_ARCH_AVR)
    // --- Arduino Nano 33 BLE & Standard 8-bit Nano ---
    #define PIN_HALL_U        2     // BLDC Motor Hall Sensor Phase U input
    #define PIN_HALL_V        3     // BLDC Motor Hall Sensor Phase V input
    #define PIN_HALL_W        4     // BLDC Motor Hall Sensor Phase W input
    
    #define PIN_MOTOR_AH      9     // Main BLDC High-Side Gate Drive Phase A
    #define PIN_MOTOR_BH      10    // Main BLDC High-Side Gate Drive Phase B
    #define PIN_MOTOR_CH      11    // Main BLDC High-Side Gate Drive Phase C
    #define PIN_MOTOR_AL      6     // Main BLDC Low-Side Gate Drive Phase A
    #define PIN_MOTOR_BL      7     // Main BLDC Low-Side Gate Drive Phase B
    #define PIN_MOTOR_CL      8     // Main BLDC Low-Side Gate Drive Phase C
    
    #define PIN_STEER_POT     A3    // 10-turn Potentiometer (Steering angle feedback)
    #define PIN_STEER_DIR     12    // Stepper Driver DIRECTION signal (Left/Right)
    #define PIN_ESTOP_BTN     13    // Physical Emergency Stop Button input
    
    #define PIN_THROTTLE      A2    // Throttle Pedal/Input analog signal
    #define PIN_BRAKE_SW      A7   // New: Digital Brake Pedal Switch (HIGH = Pressed, LOW = Released)
    #define PIN_RC_MODE_SW    A4    // RC Controller Switch (Auto/Manual/Idle states)
    
    #define PIN_CURRENT_SENS  A0    // Main Power Current Sensor analog input
    #define PIN_VOLTAGE_SENS  A1    // Main Battery Voltage Divider analog input
    #define PIN_TEMP_SENS     A5    // Motor/Heatsink Temperature Sensor analog input
    
    #define PIN_STEER_PUL     5     // Stepper Driver PULSE signal (Speed/Steps)
    #define PIN_STEER_ENA     A6    // Stepper Driver ENABLE signal (Auto-Lock/Manual-Free)
    #define PIN_ESTOP_BRAKE   5     // Hardware Emergency Brake Actuator/Relay output
    
    #define PIN_UART_RX       0     // Serial RX (Communication from Raspberry Pi)
    #define PIN_UART_TX       1     // Serial TX (Communication to Raspberry Pi)

#else
    #error "Unsupported Architecture Selected in PlatformIO!"
#endif

#endif // VCS_PINS_H