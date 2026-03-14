#ifndef VCS_PINS_H
#define VCS_PINS_H

#include <Arduino.h>

// ==========================================
// ARCHITECTURE SPECIFIC PIN MAPPINGS
// ==========================================

#if defined(ARDUINO_ARCH_STM32)
    // --- STM32F103C8T6 (Blue Pill) ---
    #define PIN_HALL_U        PB6  
    #define PIN_HALL_V        PB7  
    #define PIN_HALL_W        PB8  
    #define PIN_MOTOR_AH      PA8  
    #define PIN_MOTOR_BH      PA9  
    #define PIN_MOTOR_CH      PA10 
    #define PIN_MOTOR_AL      PB13 
    #define PIN_MOTOR_BL      PB14 
    #define PIN_MOTOR_CL      PB15 
    
    #define PIN_STEER_POT     PA0
    #define PIN_STEER_PWM     PA6
    #define PIN_STEER_DIR     PA5
    #define PIN_ESTOP_BRAKE   PA7
    
    #define PIN_THROTTLE      PA1
    #define PIN_RC_MODE_SW    PA4
    #define PIN_ESTOP_BTN     PB12
    
    #define PIN_CURRENT_SENS  PA2
    #define PIN_VOLTAGE_SENS  PA3
    #define PIN_TEMP_SENS     PB0 // Moved to PB0 to avoid conflict with STEER_DIR
    
    #define PIN_UART_RX       PB11
    #define PIN_UART_TX       PB10

#elif defined(ARDUINO_ARCH_NRF52840) || defined(ARDUINO_ARCH_AVR)
    // --- Arduino Nano 33 BLE & Standard 8-bit Nano ---
    // Note: The Nano 33 BLE uses the exact same form factor and integer pinouts.
    #define PIN_HALL_U        2  
    #define PIN_HALL_V        3  
    #define PIN_HALL_W        4  
    #define PIN_MOTOR_AH      9  
    #define PIN_MOTOR_BH      10 
    #define PIN_MOTOR_CH      11 
    #define PIN_MOTOR_AL      6  
    #define PIN_MOTOR_BL      7  
    #define PIN_MOTOR_CL      8  
    
    #define PIN_STEER_POT     A3 
    #define PIN_STEER_DIR     12 
    #define PIN_ESTOP_BTN     13 
    
    #define PIN_THROTTLE      A2 
    #define PIN_RC_MODE_SW    A4 
    
    #define PIN_CURRENT_SENS  A0 
    #define PIN_VOLTAGE_SENS  A1 
    #define PIN_TEMP_SENS     A5 
    
    // WARNING: On the standard Nano, there are limited PWM pins. 
    // You will need to physically verify your PCB routing for these two:
    #define PIN_STEER_PWM     5  
    #define PIN_ESTOP_BRAKE   5  // Conflict for standard Nano testing. BLE 33 has more PWM flexibility.
    
    #define PIN_UART_RX       0
    #define PIN_UART_TX       1

#else
    #error "Unsupported Architecture Selected in PlatformIO!"
#endif

#endif // VCS_PINS_H