#include "vcs_steering.h"


// PID Variables
float setpoint, input, output;
QuickPID steeringPID(&input, &output, &setpoint);

// Sample time for 100 Hz loop
const float Ts_s = 1.0f / FREQ_STEER_CTRL_HZ;

void initSteering() {
    pinMode(PIN_STEER_POT, INPUT);
    pinMode(PIN_STEER_DIR, OUTPUT);
    pinMode(PIN_STEER_ENA, OUTPUT);
    pinMode(PIN_STEER_PUL, OUTPUT);
    
    // Using 10-bit resolution to sync with the rest of the Nano 33 BLE system
    analogReadResolution(10); 

    // Initial state: Disabled/Free
    // (Note: On standard TB6600/DM542 stepper drivers, ENA HIGH = Disabled. 
    // Leave this LOW if your specific driver requires LOW to disable).
    digitalWrite(PIN_STEER_ENA, LOW); 
    noTone(PIN_STEER_PUL); // Ensure stepper is not pulsing

    // QuickPID Configuration
    steeringPID.SetTunings(STEER_KP, STEER_KI, STEER_KD);
    steeringPID.SetSampleTimeUs(Ts_s * 1000000);
    steeringPID.SetOutputLimits(-255.0f, 255.0f);      // Negative = Left, Positive = Right
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
    if (raw_adc < 12 || raw_adc > 1010) {
        return COMM_STEER_CENTER; 
    }

    // MAPPING PHYSICAL ADC TO COMM SCALE (0-1000)
    int mapped_pos = map(raw_adc, 0, 1023, COMM_STEER_LEFT, COMM_STEER_RIGHT);
    current_pos = (uint16_t)constrain(mapped_pos, COMM_STEER_LEFT, COMM_STEER_RIGHT);
#endif

    // --- 2. VELOCITY CHECK (The "Slew Rate" Fix) ---
    static uint16_t last_pos = COMM_STEER_CENTER;
    
    // SECURITY FIX: Prevent permanent lock-up by limiting the max change per tick
    if (abs(current_pos - last_pos) > 50) { 
        if (current_pos > last_pos) {
            current_pos = last_pos + 50;
        } else {
            current_pos = last_pos - 50;
        }
    }
    
    last_pos = current_pos;
    return current_pos;
}

void updateSteeringPID(uint16_t target_position, bool is_automatic) {
    input = (float)getMeasuredSteering();
    setpoint = (float)target_position;

    // --- SECURITY OVERRIDE ---
    if (!is_automatic || currentState == FAULT_STATE || currentState == ESTOP_STATE) {
        digitalWrite(PIN_STEER_ENA, LOW); // Disable motor (verify this matches your driver spec!)
        noTone(PIN_STEER_PUL); 
        return; 
    }

    steeringPID.Compute();

    // Deadband check
    if (abs(setpoint - input) < STEER_DEADZONE) {
        noTone(PIN_STEER_PUL);
        digitalWrite(PIN_STEER_ENA, HIGH); // Enable motor holding torque
        return;
    }

    // --- HARDWARE ACTUATION ---
    bool dir = (output > 0);
    digitalWrite(PIN_STEER_DIR, dir ? HIGH : LOW);

    float effort = abs(output);
    int step_frequency_hz = map(effort, 0, 255, 50, 2000); 

#if SIMULATION_MODE
    updateSimulatedPhysics(step_frequency_hz, dir, 0); 
#else
    // Mbed OS handles tone() well enough for basic square waves
    tone(PIN_STEER_PUL, step_frequency_hz);
#endif
}