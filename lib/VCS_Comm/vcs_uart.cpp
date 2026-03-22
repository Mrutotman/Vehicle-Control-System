#include "vcs_uart.h"
#include "vcs_hallsensor.h"
#include "vcs_steering.h"
#include "vcs_simulation.h"
#include "vcs_state_machine.h" // Required to read currentState

// --- HARDWARE FIX: Nano 33 BLE uses Serial1 for the RX/TX pins ---
// HardwareSerial ANS_SERIAL(2); // REMOVED: ESP32 Specific
#define ANS_SERIAL Serial1 

enum UartState { 
    WAIT_START1, WAIT_START2, WAIT_TYPE, WAIT_LEN, WAIT_SEQ, 
    WAIT_PAYLOAD, WAIT_CRC1, WAIT_CRC2, WAIT_END 
};

static UartState rxState = WAIT_START1;
static uint8_t rxBuffer[64];
static uint8_t rxIndex = 0;
static uint8_t expectedLength = 0;

static uint8_t target_mode = 0;
static float target_rpm = 0.0f;
static uint16_t target_steering = 500;
static uint8_t target_brake = 0;
static uint32_t last_valid_packet_time = 0;

// Function Prototypes
uint16_t calculateCRC16(uint8_t *data, uint8_t length);
void processCommand(uint8_t msgType, uint8_t *payload, uint8_t length);

void initUART() {
    // HARDWARE FIX: Mbed OS maps Serial1 directly to the board's TX/RX pins automatically
    ANS_SERIAL.begin(115200); 
    last_valid_packet_time = millis();
}

void updateUART() {
    while (ANS_SERIAL.available() > 0) {
        uint8_t byte = ANS_SERIAL.read();

        switch (rxState) {
            case WAIT_START1:
                if (byte == 0xAA) rxState = WAIT_START2;
                break;
                
            case WAIT_START2:
                if (byte == 0x55) rxState = WAIT_TYPE;
                else rxState = WAIT_START1;
                break;
                
            case WAIT_TYPE:
                rxBuffer[0] = byte;
                rxState = WAIT_LEN;
                break;
                
            case WAIT_LEN:
                rxBuffer[1] = byte;
                expectedLength = byte;
                rxState = WAIT_SEQ; // Transition to SEQ state
                break;

            case WAIT_SEQ:
                // We currently don't use the sequence byte for logic, but we must consume it
                rxState = (expectedLength > 0) ? WAIT_PAYLOAD : WAIT_CRC1;
                rxIndex = 2; // Start storing payload after Type and Length
                break;
                
            case WAIT_PAYLOAD:
                rxBuffer[rxIndex++] = byte;
                if (rxIndex >= expectedLength + 2) rxState = WAIT_CRC1;
                break;
                
            case WAIT_CRC1:
                rxBuffer[expectedLength + 2] = byte; // CRC MSB
                rxState = WAIT_CRC2;
                break;
                
            case WAIT_CRC2:
                rxBuffer[expectedLength + 3] = byte; // CRC LSB
                rxState = WAIT_END;
                break;
                
            case WAIT_END:
                if (byte == 0xFF) {
                    uint16_t receivedCRC = (rxBuffer[expectedLength + 2] << 8) | rxBuffer[expectedLength + 3];
                    uint16_t calculatedCRC = calculateCRC16(rxBuffer, expectedLength + 2);
                    
                    if (receivedCRC == calculatedCRC) {
                        processCommand(rxBuffer[0], &rxBuffer[2], expectedLength);
                        last_valid_packet_time = millis();
                    }
                }
                rxState = WAIT_START1;
                break;
        }
    }
}

void processCommand(uint8_t msgType, uint8_t *payload, uint8_t length) {
    if (msgType == 0x01 && length >= 6) { 
        target_mode = payload[0];
        
        int16_t raw_speed = (int16_t)((payload[1] << 8) | payload[2]);
        target_rpm = (float)constrain(raw_speed, COMM_SPEED_MIN, COMM_SPEED_MAX);
        
        uint16_t raw_steer = (uint16_t)((payload[3] << 8) | payload[4]);
        target_steering = constrain(raw_steer, COMM_STEER_LEFT, COMM_STEER_RIGHT);
        
        target_brake = constrain(payload[5], COMM_BRAKE_MIN, COMM_BRAKE_MAX);
    }
}

uint16_t calculateCRC16(uint8_t *data, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

uint8_t getRPiCommandMode() { return target_mode; }
float getTargetRPM() { return target_rpm; }
uint16_t getTargetSteering() { return target_steering; }
uint8_t getTargetBrake() { return target_brake; }

bool rpiHeartbeatReceived() {
    return (millis() - last_valid_packet_time) <= 200;
}

void sendTelemetry(float rpm, uint16_t steer, float volt, uint8_t state) {
    // Structure: [0xAA, 0x55, Type(0x02), Len(9), RPM(4), Steer(2), Volt(2), State(1), CRC(2), 0xFF]
    uint8_t telBuffer[12];
    
    // We'll scale voltage by 100 to send as an integer (e.g., 12.55V -> 1255)
    uint16_t voltScaled = (uint16_t)(volt * 100);
    int32_t rpmInt = (int32_t)rpm; // Cast float to int for simpler packetizing

    telBuffer[0] = 0x02; // Message Type: Telemetry
    telBuffer[1] = 9;    // Payload Length
    
    // Pack Payload
    telBuffer[2] = (rpmInt >> 24) & 0xFF;
    telBuffer[3] = (rpmInt >> 16) & 0xFF;
    telBuffer[4] = (rpmInt >> 8) & 0xFF;
    telBuffer[5] = rpmInt & 0xFF;
    
    telBuffer[6] = (steer >> 8) & 0xFF;
    telBuffer[7] = steer & 0xFF;
    
    telBuffer[8] = (voltScaled >> 8) & 0xFF;
    telBuffer[9] = voltScaled & 0xFF;
    
    telBuffer[10] = state;

    uint16_t crc = calculateCRC16(telBuffer, 11);

    ANS_SERIAL.write(0xAA);
    ANS_SERIAL.write(0x55);
    ANS_SERIAL.write(telBuffer, 11);
    ANS_SERIAL.write((crc >> 8) & 0xFF);
    ANS_SERIAL.write(crc & 0xFF);
    ANS_SERIAL.write(0xFF);
}

void broadcastVehicleTelemetry() {
    // LOGIC FIX: Route binary data to the Pi (ANS_SERIAL), keep debug text on USB (Serial)
    ANS_SERIAL.write(0xAA); 
    ANS_SERIAL.write(0x05); 

    float rpm;
    uint16_t steer;
    uint8_t state = (uint8_t)currentState;

    #if SIMULATION_MODE
        // --- DATA COLLECTION (SIM) ---
        rpm = getSimulatedRPM();
        steer = getSimulatedSteering();
        
        // --- HUMAN READABLE DEBUG TO USB ---
        Serial.print("SIM DATA -> RPM: ");
        Serial.print(rpm);
        Serial.print(" | Steer: ");
        Serial.println(steer);
    #else
        // --- DATA COLLECTION (HARDWARE) ---
        rpm = getMeasuredRPM();
        steer = getMeasuredSteering();
    #endif

    // Pack and send binary data to Raspberry Pi (Unified for both Sim and Hardware)
    ANS_SERIAL.write(((int)rpm >> 8) & 0xFF);
    ANS_SERIAL.write((int)rpm & 0xFF);
    ANS_SERIAL.write((steer >> 8) & 0xFF);
    ANS_SERIAL.write(steer & 0xFF);
    ANS_SERIAL.write(state);
    ANS_SERIAL.write(0xFF); // End Byte
}