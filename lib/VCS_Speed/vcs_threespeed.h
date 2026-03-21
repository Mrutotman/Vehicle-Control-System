#ifndef VCS_THREESPEED_H
#define VCS_THREESPEED_H

#include <Arduino.h>
#include "vcs_pins.h"

enum DriveMode {
    DRIVE_LOW,
    DRIVE_MED,
    DRIVE_HIGH
};

void initThreeSpeed();
void setDriveMode(DriveMode mode);

#endif