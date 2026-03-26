#ifndef VCS_EMBUTTON_H
#define VCS_EMBUTTON_H

#include <Arduino.h>
#include "vcs_pins.h"

// Global Telemetry Variable exposed for UART dashboard
extern bool is_estop_pressed;

void initEmButton();
void updateEmButton();

// Direct hardware query for the State Machine
bool isEmButtonPressed();

#endif // VCS_EMBUTTON_H