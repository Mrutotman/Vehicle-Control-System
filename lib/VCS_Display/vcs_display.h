#ifndef VCS_DISPLAY_H
#define VCS_DISPLAY_H

#include <Arduino.h>
#include "vcs_pins.h"
#include "vcs_state_machine.h"
#include "vcs_reverse.h"

void initDisplay();
void updateDisplay(float rpm, uint16_t steer, uint8_t speedMode);

#endif // VCS_DISPLAY_H