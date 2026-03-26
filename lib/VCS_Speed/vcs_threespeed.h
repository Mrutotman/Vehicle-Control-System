#ifndef VCS_THREESPEED_H
#define VCS_THREESPEED_H

#include <Arduino.h>


// Explicitly assigning values so they safely cast to uint8_t for telemetry/display
enum DriveMode {
    DRIVE_LOW = 0,
    DRIVE_MED = 1,
    DRIVE_HIGH = 2
};

// Global Telemetry Variable
extern uint8_t current_drive_mode;

void initThreeSpeed();
void updateThreeSpeed();

// Public setter so the Raspberry Pi can shift gears in Autonomous mode
void setDriveMode(DriveMode mode);

#endif // VCS_THREESPEED_H