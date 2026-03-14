/* ==============================================================================
 * MODULE:        VCS_Safety
 * RESPONSIBILITY: System-wide Fault Detection and E-Stop Handling.
 * DESCRIPTION:   The "Guardian" of the system. Continuously monitors the battery 
 * voltage, motor temperature, and the physical E-Stop button. 
 * It also validates the RPi Heartbeat. If any safety threshold 
 * is crossed, it immediately signals the state machine to 
 * cut motor power and engage the electronic brake.
 * HW RESOURCES:  ADC (Voltage/Temp), GPIO (PB12 - E-Stop), Millis() (Heartbeat).
 * ============================================================================== */

#include "vcs_safety.h"
#include "vcs_brake.h"

// ==========================================
// Hardware Sensor Pin Placeholders
// Add these to vcs_pins.h later
// ==========================================
#ifndef PIN_CURRENT_SENS

#endif

// ==========================================
// Safety Thresholds (Adjust based on your 1000W motor/battery specs)
// ==========================================
const int CURRENT_MAX_ADC = 3000;  // Placeholder threshold for over-current 
const int VOLTAGE_MIN_ADC = 1000;  // Placeholder threshold for under-voltage 
const int TEMP_MAX_ADC    = 3500;  // Placeholder threshold for over-temp 

void initSafety() {
    pinMode(PIN_CURRENT_SENS, INPUT);
    pinMode(PIN_VOLTAGE_SENS, INPUT);
    pinMode(PIN_TEMP_SENS, INPUT);
    
    // E-Stop button with internal pull-up (Active LOW)
    pinMode(PIN_ESTOP_BTN, INPUT_PULLUP); 
    
    // PWM Brake output
    pinMode(PIN_ESTOP_BRAKE, OUTPUT);
    applyBrake(100); // Engage brake fully on boot
}

// ==========================================
// ADDED: Battery Voltage Calculation
// ==========================================
float getBatteryVoltage() {
    // Read raw ADC value from the voltage sensor pin (0 to 1023 on Nano)
    int raw = analogRead(PIN_VOLTAGE_SENS); 
    
    // Convert ADC to Voltage (Assumes 5V reference)
    // Formula: (Raw / 1023.0) * 5.0 * Voltage_Divider_Ratio
    // For a standard divider, a multiplier of 11.0 is common for 48V-60V systems
    float measured_v = (raw / 1023.0) * 5.0 * 11.0; 
    
    return measured_v;
}

bool emergencyStopPressed() {
    // Returns true if physical button is pressed (Assuming Active LOW)
    return (digitalRead(PIN_ESTOP_BTN) == LOW);
}

uint32_t readFaultInputs() {
    uint32_t current_faults = FAULT_NONE;

    // 1. Motor Current 
    if (analogRead(PIN_CURRENT_SENS) > CURRENT_MAX_ADC) {
        current_faults |= FAULT_OVERCURRENT;
    }

    // 2. Battery Voltage 
    if (analogRead(PIN_VOLTAGE_SENS) < VOLTAGE_MIN_ADC) {
        current_faults |= FAULT_UNDERVOLTAGE;
    }

    // 3. Temperature 
    if (analogRead(PIN_TEMP_SENS) > TEMP_MAX_ADC) {
        current_faults |= FAULT_OVERTEMP;
    }

    // 4. Hall Sensor Validity 
    uint8_t hall = getHallState();
    if (hall == 0 || hall == 7) {
        current_faults |= FAULT_HALL_SEQ;
    }

    // 5. Communication Health 
    if (!rpiHeartbeatReceived()) {
        current_faults |= FAULT_COMMS_LOSS;
    }

    return current_faults;
}

bool selfTestPassed() {
    // Run initial checks before transitioning from INIT to IDLE
    delay(100); // Allow sensors to stabilize
    
    uint32_t initial_faults = readFaultInputs();
    
    // Comms loss is expected during boot if RPi is still starting, 
    // so we might mask it out for the self-test depending on boot sequence.
    initial_faults &= ~FAULT_COMMS_LOSS; 
    
    return (initial_faults == FAULT_NONE);
}

void resetHardwareWatchdog() {
    // Level 1: Internal VCU watchdog toggle [cite: 68]
    // STM32 specific Implementation (IWDG - Independent Watchdog)
    // IWDG->KR = 0xAAAA; // Reload the watchdog counter
}

bool isSystemSafe() {
    // TEMPORARY: Bypass heartbeat for bench testing
    bool heartbeat = true; 
    
    // Check physical pins
    bool estop = digitalRead(PIN_ESTOP_BTN) == HIGH; // Adjust based on your wiring
    bool battery = analogRead(PIN_VOLTAGE_SENS) > 200; // Check if voltage pin is tied high
    
    return (heartbeat && estop && battery);
}