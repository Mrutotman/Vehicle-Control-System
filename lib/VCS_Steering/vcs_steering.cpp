/* ==============================================================================
 * MODULE:        VCS_Steering
 * RESPONSIBILITY: Closed-loop Steering Position Control using QuickPID.
 * DESCRIPTION:    Maintains the front wheel angle by comparing the Target Angle 
 * from the RPi against the feedback from a 10-turn potentiometer.
 * ============================================================================== */

#include "vcs_steering.h"


// PID Variables
float setpoint, input, output;
QuickPID steeringPID(&input, &output, &setpoint);

// Sample time for 100 Hz loop
const float Ts_s = 1.0f / FREQ_STEER_CTRL_HZ;

// ESP32 Hardware Timer Settings for Stepper Pulses (Non-blocking)
const int steerPwmChannel = 1; 
const int steerPwmResolution = 8; 

void initSteering() {
    pinMode(PIN_STEER_POT, INPUT);
    pinMode(PIN_STEER_DIR, OUTPUT);
    pinMode(PIN_STEER_ENA, OUTPUT);
    
    analogReadResolution(12); // ESP32 12-bit ADC

    // Setup hardware timer for non-blocking pulses
    ledcSetup(steerPwmChannel, 1, steerPwmResolution); 
    ledcAttachPin(PIN_STEER_PUL, steerPwmChannel);
    ledcWrite(steerPwmChannel, 127); // 50% duty cycle for the pulse wave

    // Initial state: Disabled/Free
    digitalWrite(PIN_STEER_ENA, LOW); 

    // QuickPID Configuration
    steeringPID.SetTunings(STEER_KP, STEER_KI, STEER_KD);
    steeringPID.SetSampleTimeUs(Ts_s * 1000000);
    steeringPID.SetOutputLimits(-255, 255);      // Negative = Left, Positive = Right
    steeringPID.SetMode(QuickPID::Control::automatic);
}

uint16_t getMeasuredSteering() {
    uint16_t current_pos;

    // --- 1. DATA ACQUISITION ---
#if SIMULATION_MODE
    // In Simulation, we use the "Digital Twin" position
    current_pos = (uint16_t)constrain(sim_steer_pos, COMM_STEER_LEFT, COMM_STEER_RIGHT);
#else
    int raw_adc = analogRead(PIN_STEER_POT);

    // DISCONNECTION CHECK (Hardware Security)
    if (raw_adc < 50 || raw_adc > 4045) {
        // Triggering a fault here is best, but we'll return safe-center for now
        return 500; 
    }

    // MAPPING PHYSICAL ADC TO COMM SCALE (0-1000)
    int mapped_pos = map(raw_adc, 0, 4095, COMM_STEER_LEFT, COMM_STEER_RIGHT);
    current_pos = (uint16_t)constrain(mapped_pos, COMM_STEER_LEFT, COMM_STEER_RIGHT);
#endif

    // --- 2. VELOCITY CHECK (The "Spike Filter") ---
    // This is mandatory for 60V systems to ignore sensor noise
    static uint16_t last_pos = 500;
    
    // Calculate the jump: |current - last|
    if (abs(current_pos - last_pos) > 200) { 
        // If it jumps > 20% of the range in 10ms, ignore the "spike" 
        // and return the last known good position.
        return last_pos; 
    }
    
    last_pos = current_pos;
    return current_pos;
}

void updateSteeringPID(uint16_t target_position, bool is_automatic) {
    input = (float)getMeasuredSteering();
    setpoint = (float)target_position;

    // --- SECURITY OVERRIDE ---
    if (!is_automatic || currentState == FAULT_STATE || currentState == ESTOP_STATE) {
        digitalWrite(PIN_STEER_ENA, LOW); 
        ledcWriteTone(steerPwmChannel, 0); 
        return; 
    }

    steeringPID.Compute();

    // Deadband check
    if (abs(setpoint - input) < 5) {
        ledcWriteTone(steerPwmChannel, 0);
        digitalWrite(PIN_STEER_ENA, HIGH); 
        return;
    }

    // --- HARDWARE ACTUATION ---
    bool dir = (output > 0);
    digitalWrite(PIN_STEER_DIR, dir ? HIGH : LOW);

    float effort = abs(output);
    int step_frequency_hz = map(effort, 0, 255, 50, 2000); 

#if SIMULATION_MODE
    // Instead of just making noise, we "move" our simulated rack
    // We pass the current frequency and direction to the physics engine
    updateSimulatedPhysics(step_frequency_hz, dir, 0); 
#else
    ledcWriteTone(steerPwmChannel, step_frequency_hz);
#endif
}