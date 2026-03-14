#ifndef VCS_SAFETY_H
#define VCS_SAFETY_H

#include <Arduino.h>
#include "../VCS_Config/vcs_pins.h"
#include "../VCS_Config/vcs_constants.h"
#include "../VCS_Hall/vcs_hall.h"
#include "../VCS_Comm/vcs_uart.h"

// Define Fault Flags (Bitmask for multiple simultaneous faults)
#define FAULT_NONE            0x00
#define FAULT_OVERCURRENT     0x01
#define FAULT_UNDERVOLTAGE    0x02
#define FAULT_OVERTEMP        0x04
#define FAULT_HALL_SEQ        0x08
#define FAULT_COMMS_LOSS      0x10
#define FAULT_SELFTEST        0x20

// Hardware initialization
void initSafety();

// State Machine Stubs & Implementations
uint32_t readFaultInputs();
bool selfTestPassed();
bool emergencyStopPressed();

// Battery Reading Functions
float getBatteryVoltage();

// Watchdog management
void resetHardwareWatchdog();

// Combines all safety checks into a single function for easy use in the main loop
bool isSystemSafe();

#endif // VCS_SAFETY_H