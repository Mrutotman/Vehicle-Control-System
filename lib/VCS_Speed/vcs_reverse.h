#ifndef VCS_REVERSE_H
#define VCS_REVERSE_H

#include <Arduino.h>

// Initializes the reverse switch and output pins
void initReverse();

// Evaluates the driver's request against current vehicle speed
void updateReverse();

// Returns true if reverse is actually active (useful for your OLED UI)
bool isReverseEngaged();

#endif // VCS_REVERSE_H