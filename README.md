# 🏎️ VCS: Vehicle Control System (Shell Eco-marathon Edition)

The **Vehicle Control System (VCS)** is a high-reliability, RTOS-driven control architecture designed for the **Shell Eco-marathon (SEM)**. It serves as the deterministic bridge between high-level autonomous navigation (Raspberry Pi) and high-power execution (1500W BLDC Motor & Stepper Steering).

## 🤖 AI Collaboration
This system was architected and refined in collaboration with **Gemini 3.1 Pro**. Key focus areas included:
* **Mbed OS Integration:** Transitioning to a multi-threaded architecture on the Arduino Nano 33 BLE.
* **Safety Engineering:** Implementing a Hardware Watchdog, CRC16 checksums, and prioritized interrupt-based overrides.
* **Digital Twin:** A HIL (Hardware-in-the-Loop) simulation module for lab-testing logic without 60V power.

---

## 🚀 Version History

### **v1.3 (Current - Nano 33 BLE & 1500W Driver)**
*The "Reliability" release. Focuses on deterministic timing and standard e-bike driver integration.*
* **Mbed OS Threads:** 1kHz Control Loop, 100Hz Comm/Safety Loop, and 20Hz UI/Telemetry Loop.
* **Hardware Watchdog:** Integrated nRF52840 watchdog (2s timeout) for SEM technical compliance.
* **CRC16 Protocol:** All UART packets are validated via CRC16-CCITT to prevent EMI-induced ghost commands.
* **10-bit Standardization:** Native ADC/PWM scaling (0-1023) across all modules for linear control.

### **v1.2 & v1.1 (Legacy - ESP32/STM32)**
*Focus on steering precision and initial modular isolation.*
* **Stepper Steering:** High-torque microstepping replaced legacy DC motor steering.
* **Smart Shaft Locking:** Integrated `ENA` logic to manage holding torque and reduce idle current draw.

---

## 🛡️ SIDLAK Safety Architecture
The VCS enforces a strict priority hierarchy. Motor power is physically impossible unless the system evaluates all safety conditions as "Clear."

| State | SEM Requirement | Description |
| :--- | :--- | :--- |
| **INIT** | Self-Test | Boot sequence, optocoupler lockout, and hardware health check. |
| **IDLE** | Safety Standby | System alive; motor disabled; awaiting RPi heartbeat/handshake. |
| **MANUAL** | Human Control | Primary mode. Driver-operated via throttle pedal and steering. |
| **AUTONOMOUS**| Computer Control | Requires **active DMS hold** and verified 10Hz RPi heartbeat. |
| **FAULT** | Fail-Safe | Triggered by comms loss or sensor spikes. **Instant Motor Kill.** |
| **ESTOP** | Emergency Stop | Physical hardware lockout; requires manual hard power cycle. |

---

## 🔌 Hardware Setup (Arduino Nano 33 BLE)

### Control & Actuation (3.3V Logic)
* **DMS Button:** Pin **D2** (Input Pullup). Must be held to authorize `AUTONOMOUS` mode.
* **Brake Switch:** Pin **D3** (Input Pullup). Physical override that triggers the e-bike "Low Brake" wire.
* **E-Stop:** Pin **D4** (Input Pullup). NC configuration recommended for wire-break safety.
* **Steer PUL/DIR/ENA:** Pins **D5, D6, D7**. Controls the TB6600 (or equivalent) stepper driver.
* **Throttle Out:** Pin **D9**. 10-bit PWM. *Note: Requires RC Filter + OpAmp to reach 4.2V for max speed.*

### Telemetry & Sensing
* **Steer Pot:** Pin **A0**. 10-turn potentiometer for closed-loop PID feedback.
* **UART1 (To RPi):** **TX (D1) / RX (D0)**. 115200 Baud (3.3V Logic).
* **Hall Speed:** Pin **D10**. Parallel tap from motor Hall sensors. *Note: Use Voltage Divider for 5V -> 3.3V.*
* **OLED (I2C):** **SDA (A4) / SCL (A5)**. SSD1306 128x64 Telemetry Display.

---

## 📡 Communication Protocol (v1.3 CRC16)

### **Uplink (Nano → Pi) - Telemetry**
**Format:** `[0xAA][0x55][0x02][0x09][RPM(4)][STEER(2)][VOLT(2)][STATE(1)][CRC(2)][0xFF]`
* **RPM:** 32-bit Big-Endian (Scaled).
* **Steer:** 16-bit (0-1000 scale).
* **State:** SIDLAK State Index (0=INIT, 1=IDLE, 2=MANUAL, 3=AUTO, 4=FAULT, 5=ESTOP).

### **Downlink (Pi → Nano) - Command**
**Format:** `[0xAA][0x55][0x01][Len][MODE][SPEED(2)][STEER(2)][BRAKE(1)][CRC(2)][0xFF]`
* **Mode:** 1 = Request Autonomous, 0 = Manual/IDLE.
* **Speed:** Target RPM (0 to 3000).
* **Brake:** 1 = Force Engagement, 0 = Release.

---

## 💻 Building and Testing (HIL Simulation)
To test the vehicle logic without 60V power:
1.  Set `#define SIMULATION_MODE 1` in `vcs_constants.h`.
2.  Open **PlatformIO**, select the `nano33ble` environment, and click **Upload**.
3.  Monitor the OLED and Serial Monitor (**115200 baud**) for physics emulation.
4.  Ground **D2 (DMS)** to watch the system transition to **AUTO** via the digital twin.
