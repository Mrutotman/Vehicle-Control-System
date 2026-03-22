# 🏎️ VCS: Vehicle Control System (Shell Eco-marathon Edition)

The **Vehicle Control System (VCS)** is a closed-loop, RTOS-driven control architecture designed specifically for the **Shell Eco-marathon (SEM)** competition. It serves as the high-reliability bridge between high-level navigation (Raspberry Pi) and high-power execution (1500W BLDC Motor and Stepper-based Steering).

## 🤖 AI Collaboration
This system was architected and refined in collaboration with **Gemini 3.1 Pro**. The collaboration focused on:
* Transitioning from monolithic ESP32 code to a **Deterministic Mbed OS Thread Architecture** on the Arduino Nano 33 BLE.
* Implementing **Hardware-in-the-Loop (HIL) Simulation** for safe bench testing without 60V power.
* Integrating **SEM-compliant safety protocols** (Hardware Watchdog, DMS, Heartbeat, and Optocoupler Overrides).

---

## 🚀 Version History

### **v1.3 (Current - Nano 33 BLE, 1500W Driver & Mbed OS)**
*The "Reliability" release. Focuses on strict thread timing, standard hardware, and digital twin verification.*
* **Powertrain Upgrade:** Optimized for **1500W prebuilt e-bike BLDC motor drivers**, removing the instability of DIY commutation logic.
* **Mbed OS RTOS:** Split execution into strict 1kHz (Control), 100Hz (Comm), and 20Hz (UI) threads.
* **Hardware Watchdog:** Integrated the nRF52840 hardware watchdog to enforce SEM safety rules.
* **Full Digital Twin Integration:** Physics-based simulation module (`vcs_simulation`) to emulate vehicle dynamics and payload parsing without 60V power.
* **High-Speed Binary Telemetry:** CRC16-validated Uplink/Downlink protocol for real-time Raspberry Pi monitoring.

### **v1.1 & v1.2 (Legacy - ESP32/STM32)**
*Major upgrade focusing on steering precision, power management, and SEM technical compliance.*
* **Stepper Steering:** Replaced legacy DC motors with high-torque microstep drivers.
* **Smart Shaft Locking:** Integrated `ENA` logic to manage holding torque, reducing current draw to **0.4A** during manual idle to conserve battery.
* **Multi-Architecture ADC Mapping:** Auto-scales feedback between 12-bit (STM32/ESP32) and 10-bit hardware.

### **v1.0 (Legacy - Prototype)**
*Initial prototype build focusing on core connectivity.*
* **DC Motor Steering:** Basic PWM/DIR control via H-Bridge.
* **Core State Machine:** First draft of the SIDLAK state transitions.

---

## 🛡️ SIDLAK & SEM Safety Architecture
The system utilizes a strict priority hierarchy. Motor power is strictly prohibited unless the system evaluates all safety conditions as "Clear."

| State | SEM Requirement | Description |
| :--- | :--- | :--- |
| **INIT** | Self-Test | Boot sequence, optocoupler lockout, and internal hardware health check. |
| **IDLE** | Safety Standby | System alive but motor disabled; awaiting RPi heartbeat. |
| **MANUAL** | Human Control | Driver-operated via throttle pedal and steering wheel. |
| **AUTONOMOUS** | Computer Control | Requires **active DMS hold** and verified 10Hz RPi communication. |
| **FAULT** | Fail-Safe | Triggered by comms loss, CRC failures, or sensor spikes. **Instant Motor Kill.** |
| **ESTOP** | Emergency Stop | Physical hardware lockout; requires hard power cycle. |

---

## 🛠️ System Modules
The project follows a modular C++ structure to ensure maintainability and safety isolation.

* **`vcs_throttle` / `vcs_steering`**: QuickPID-driven actuator control mapped to 10-bit limits.
* **`vcs_uart`**: Manages the `0xAA 0x55` binary UART protocol with strict CRC16 validation.
* **`vcs_state_machine`**: The core SIDLAK architecture; manages transitions and Watchdog feeding.
* **`vcs_lowbrake` / `vcs_embutton`**: Zero-latency hardware interrupt overrides for safety.
* **`vcs_display`**: Drives the 128x64 I2C OLED for live telemetry and 3-LED status indicators.
* **`vcs_hallsensor`**: Mbed OS hardware interrupts for parallel RPM tapping.
* **`vcs_simulation`**: Physics engine for testing RPi autonomy without physical motors attached.

---

## 🔌 Hardware Setup (Arduino Nano 33 BLE)

### Control & Actuation (3.3V Logic to 1500W Driver)
* **DMS Button:** Pin D2 (Input Pullup)
* **Brake Switch:** Pin D3 (Input Pullup - Primary Override)
* **E-Stop:** Pin D4 (Input Pullup)
* **Steer PUL/DIR/ENA:** Pins D5, D6, D7
* **Throttle Out:** Pin D9 (10-bit PWM mapped to Optocoupler/RC Filter)

### Telemetry & Sensing
* **Steer Pot:** Pin A0 (Analog Precision Feedback - 10-bit)
* **UART1 (To RPi):** TX (Pin D1) / RX (Pin D0) at 115200 Baud
* **Hall Speed:** Pin D10 (Parallel tap from driver dashboard wire)
* **OLED (I2C):** SDA (Pin A4) / SCL (Pin A5)

---

## 📡 Communication Protocol (v1.3 CRC16)

All packets feature a strict two-byte start header and a two-byte CRC16 validation.

### **Uplink (Nano 33 BLE → Pi) - Telemetry**
**Format:** `[0xAA][0x55][0x02][0x09][RPM(4)][STEER(2)][VOLT(2)][STATE(1)][CRC(2)][0xFF]`
* **RPM**: 32-bit Integer (Scaled).
* **Steer/Volt**: 16-bit Unsigned Integers.
* **State**: Current SIDLAK State index.

### **Downlink (Pi → Nano 33 BLE) - Command**
**Format:** `[0xAA][0x55][0x01][Len][MODE][SPEED(2)][STEER(2)][BRAKE(1)][CRC(2)][0xFF]`
* **Mode**: 1 = Request Autonomous, 0 = Force Manual.
* **Speed/Steer**: 16-bit target values bounded by `vcs_constants.h`.

---

## 💻 Building and Testing (HIL Simulation)
To test the system logic safely without 60V power:
1. Open `vcs_constants.h` and set `#define SIMULATION_MODE 1`.
2. Connect the Nano 33 BLE via USB, build via **PlatformIO**, and open the Serial Monitor.
3. Ground the **DMS Pin (e.g., D2)** to simulate the driver holding the dead-man switch to observe the transition to **AUTONOMOUS**.
4. Ground the **Brake Pin (e.g., D3)** to verify the optocoupler fires and the state machine instantly overrides back to **MANUAL**.

---

## 📚 Libraries Used
* **Mbed OS (`mbed.h`)**: Native ARM threading, sleep timings, and hardware Watchdog.
* **QuickPID**: High-performance, low-jitter PID calculations synced to microsecond threads.
* **Adafruit SSD1306 / GFX**: For onboard OLED telemetry visualization.
* **Arduino Core**: Base framework for Nano 33 BLE compatibility.