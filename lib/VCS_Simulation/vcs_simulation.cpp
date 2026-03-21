/* ==============================================================================
 * MODULE:        VCS_Simulation
 * RESPONSIBILITY: Physics Emulation & Signal Testing (Uplink/Downlink)
 * ------------------------------------------------------------------------------
 * SETUP INSTRUCTIONS FOR SIMULATION MODE:
 * * 1. PHYSICAL CONNECTIONS (Logic Only):
 * - ESP32 (TX2/Pin 17)  --->  RPi (RXD/Pin 10)
 * - ESP32 (RX2/Pin 16)  --->  RPi (TXD/Pin 8)
 * - COMMON GROUND: Connect ESP32 GND to RPi GND (Critical for UART stability).
 * * 2. POWER SAFETY:
 * - DISCONNECT the 60V Main Battery. Simulation should run on USB power only.
 * - DISCONNECT the 1000W Motor Phase wires if the controller is powered.
 * * 3. PERIPHERALS NEEDED:
 * - DMS Button: Connect to PIN_DMS_BUTTON and GND (Normally Open).
 * - Brake Switch: Connect to PIN_LOWBRAKE_IN and GND.
 * - Note: In SIMULATION_MODE, the Steering Pot and Hall Sensors are ignored,
 * but the DMS and Brake triggers remain active for safety logic testing.
 * * 4. SOFTWARE ACTIVATION:
 * - Set '#define SIMULATION_MODE true' in 'vcs_simulation.h'.
 * - Ensure Serial2 is initialized at 115200 baud for RPi communication.
 * ============================================================================== */

#include "vcs_simulation.h"
#include "vcs_state_machine.h"

// Global Simulated Variables
float sim_steer_pos = 500.0f; 
float sim_motor_rpm = 0.0f;

/**
 * @brief Updates the "Digital Twin" of the vehicle physics.
 * @param pulse_freq Current frequency sent to the stepper (Hz)
 * @param direction  True for Right, False for Left
 * @param throttle_pwm Current PWM value sent to the motor controller (0-255)
 */
void updateSimulatedPhysics(int pulse_freq, bool direction, float throttle_pwm) {
    #if SIMULATION_MODE
    
    // 1. STEERING PHYSICS SIMULATION
    // Scale: 2000Hz = Max speed. 0.001 is the integration constant for 10ms loops.
    if (pulse_freq > 0) {
        float move = (float)pulse_freq * 0.001f;
        if (direction) sim_steer_pos += move;
        else sim_steer_pos -= move;
    }
    sim_steer_pos = constrain(sim_steer_pos, 0, 1000);

    // 2. MOTOR PHYSICS SIMULATION
    // Simple inertia model: RPM follows throttle with a slight delay
    float target_rpm = map(throttle_pwm, 0, 255, 0, 3000); 
    sim_motor_rpm += (target_rpm - sim_motor_rpm) * 0.05f; // 5% approach per loop

    #endif
}

float getSimulatedRPM() { return sim_motor_rpm; }
float getSimulatedSteering() { return sim_steer_pos; }