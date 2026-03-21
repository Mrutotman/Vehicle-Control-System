#ifndef VCS_EMBUTTON_H
#define VCS_EMBUTTON_H

#include <Arduino.h>
#include "vcs_pins.h"
#include "vcs_constants.h"

void initEmButton();
bool isEmButtonPressed();

#endif // VCS_EMBUTTON_H