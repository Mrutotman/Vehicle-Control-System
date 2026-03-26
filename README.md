# 🏎️ VCS: Vehicle Control System (Shell Eco-marathon Edition)

The **Vehicle Control System (VCS)** is a high-reliability, RTOS-driven control architecture designed for the **Shell Eco-marathon (SEM)**. It serves as the deterministic bridge between high-level autonomous navigation (Raspberry Pi) and high-power execution (1500W BLDC Motor & Stepper Steering).

![3D Render of the VCS V1.4 Isolated PCB](assets/image_b42ac8.png)

## 🤖 AI Collaboration
This system was architected and refined in collaboration with **Gemini 3.1 Pro**. Key focus areas included:
* **Hardware Security & PCB Design:** Engineering a custom 100x100mm dual-layer PCB featuring strict galvanic isolation, dual ground planes, and an optocoupler "moat" to protect 3.3V logic from 60V motor noise.
* **Mbed OS Integration:** Transitioning to a multi-threaded architecture on the Arduino Nano 33 BLE.
* **Safety Engineering:** Implementing a Hardware Watchdog, CRC16 checksums, and prioritized interrupt-based overrides.
* **Digital Twin:** A HIL (Hardware-in-the-Loop) simulation module for lab-testing logic without high-voltage power.

---

## 🚀 Version History

### **v1.4 (Current - Custom PCB & Galvanic Isolation)**
*The "Hardware Security" release. Focuses on migrating from loose wire prototypes to a professional, noise-immune physical board.*
* **Custom PCB:** 100x100mm form factor with dedicated terminal blocks for perimeter harness wiring.
* **Galvanic Isolation:** Integrated PC817 Optocoupler creating a physical barrier between the Nano's ground and the 60V vehicle ground.
* **Analog Signal Conditioning:** On-board LM358 Op-Amp and RC filtering (`C1`, `C2`) for clean, hardware-smoothed throttle output.
* **5V Logic Translation:** Dedicated TXS0108E level-shifting IC for reliable communication with the stepper driver and peripherals.

### **v1.3 (Legacy - Mbed OS & Reliability)**
*The "RTOS" release. Focused on deterministic timing.*
* **Mbed OS Threads:** 1kHz Control Loop, 100Hz Comm/Safety Loop, and 20Hz UI/Telemetry Loop.
* **Hardware Watchdog:** Integrated nRF52840 watchdog (2s timeout) for SEM technical compliance.

### **v1.2 & v1.1 (Legacy - ESP32/STM32)**
*Focus on steering precision and initial modular isolation.*
* **Stepper Steering:** High-torque microstepping replaced legacy DC motor steering.

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

## 🔌 Comprehensive Hardware & Wiring Guide (V1.4)

This section serves as the master reference for the V1.4 PCB, detailing the exact signal path from the physical perimeter terminal blocks to the Arduino Nano 33 BLE software pins.

![Close-up of final silkscreen labels for V1.4 perimeter terminal blocks](assets/image_3099c1.png)

### 📥 Inputs (Sensors & Driver Controls)
| Terminal Block | MCU Pin | Software Macro (`VCS_Pins.h`) | Hardware Path / Conditioning | Function |
| :--- | :--- | :--- | :--- | :--- |
| **POWER** | `VIN` | *N/A* | Direct to Nano `VIN` via **C3 (1000µF)** | Main logic power (12V/5V step-down to Nano). Bulk smoothed. |
| **STEERPOT** | `A0` | `PIN_STEER_POT` | Direct trace to ADC | 10-turn Steering Potentiometer. |
| **THRTP** | `A1` | `PIN_THROTTLE_IN` | Direct trace to ADC | Driver Throttle Pedal (0-3.3V). |
| **DMSC** | `D2` | `PIN_DMS_BUTTON` | Direct trace (`INPUT_PULLUP`) | Dead Man's Switch. Active-Low. |
| **BRKS+** | `D8` | `PIN_LOWBRAKE_IN` | Direct trace (`INPUT_PULLUP`) | Physical Brake Switch. Active-Low. |
| **ROTARY / RVRS**| `A2` | `PIN_REVERSE_IN` | Direct trace (`INPUT_PULLUP`) | Driver's Reverse/Gear Selector. Active-Low. |
| **3PS (Low)** | `A3` | `PIN_SPEED_SW_LOW`| Direct trace (`INPUT_PULLUP`) | 3-Position Speed Switch (Low limit). |
| **3PS (High)** | `A7` | `PIN_SPEED_SW_HIGH`| Direct trace (`INPUT_PULLUP`) | 3-Position Speed Switch (High limit). |
| **SPS+** | `D10`| `PIN_HALL_SPEED` | Direct trace (Verify 3.3V max) | Motor Hall-Effect Speed Feedback. |

### 📤 Outputs (Actuators & Motor Control)
| Terminal Block | MCU Pin | Software Macro (`VCS_Pins.h`) | Hardware Path / Security Barrier | Function |
| :--- | :--- | :--- | :--- | :--- |
| **BRKP** | `D3` | `PIN_LOWBRAKE_OUT`| **PC817 Optocoupler** | Galvanically isolated brake trigger. 60V safe. |
| **MISTECON** (PUL)| `D5` | `PIN_STEER_PUL` | **TXS0108E Level Shifter** | Steps 3.3V up to clean 5V Stepper Pulse. |
| **MISTECON** (DIR)| `D6` | `PIN_STEER_DIR` | **TXS0108E Level Shifter** | Steps 3.3V up to clean 5V Stepper Direction. |
| **MISTECON** (ENA)| `D7` | `PIN_STEER_ENA` | **TXS0108E Level Shifter** | Steps 3.3V up to clean 5V Stepper Enable. |
| **ESC (S+)** | `D9` | `PIN_THROTTLE_OUT`| **RC Filter (C1, C2) + LM358** | Converts PWM to pure analog DC, buffered. |
| *(Internal)* | `A6` | `PIN_LED_FAULT` | Direct to onboard **LED1** via 220Ω | Dashboard Error / Fault indicator. |

### 📡 Communications (Telemetry)
| Terminal Block | MCU Pin | Software Macro (`VCS_Pins.h`) | Hardware Path / Conditioning | Function |
| :--- | :--- | :--- | :--- | :--- |
| **DISPLAY** | `A4, A5`| `PIN_OLED_SDA/SCL`| Direct I2C | Local OLED Dashboard (SSD1306). |
| **UARTRPI** | `D0, D1`| `PIN_UART_RX/TX` | Direct UART | Telemetry & Control Uplink to Raspberry Pi. |

---

## 🔒 Hardware Security Architecture & Assembly Rules

![V1.4 Dual Ground Planes demonstrating strict galvanic isolation (the central moat)](assets/image_b41b29.png)

1. **The 60V Isolation Moat:** The `BRKP` terminal connects directly to the high-voltage vehicle motor controller. The `D3` signal triggers an infrared LED inside the **PC817 Optocoupler**. **Wiring Rule:** The ground pin on the `BRKP` terminal must *only* connect to the vehicle ground, never the Nano ground.
2. **The 5V Translation Bridge:** The **TXS0108E** high-speed level shifter guarantees that 5V stepper noise cannot back-feed into the 3.3V logic pins (`D5`, `D6`, `D7`).
3. **The Analog Clean Room:** The V1.4 board routes the `D9` PWM through an RC low-pass filter (`C1`, `C2`, `R1`, `R2`) to flatten it into true DC, then pushes it through an **LM358 Op-Amp** to buffer the current, ensuring the throttle signal never sags under load.
4. **Active-Low Fail-Safes:** All critical digital inputs (`DMSC`, `ROTARY`, `BRKS+`) are wired active-low with internal pull-ups. If a wire is severed during the race, the system safely defaults to a deactivated state.

---

## 📦 Bill of Materials (BOM) - V1.4 PCB

### Integrated Circuits & Core Modules
| Designator | Component | Function |
| :--- | :--- | :--- |
| **NANO1** | Arduino Nano 33 BLE | Primary MCU (3.3V Logic, RTOS, BLE capabilities). |
| **TXS0108E**| 8-Channel Level Shifter| Safely translates 3.3V Nano signals to 5V logic for stepper driver. |
| **LM-358** | Dual Op-Amp | Buffers the hardware-filtered PWM signal to the `ESC` throttle. |
| **PC817** | Optocoupler | Creates physical light-gap barrier to trigger the 60V `BRKP` line. |

### Passive Components (Resistors & Capacitors)
| Designator | Value | Purpose |
| :--- | :--- | :--- |
| **C3** | `1000µF` | Bulk electrolytic capacitor. Absorbs voltage spikes on `POWER`. |
| **C2** | `10µF` | Smoothing capacitor (part of the Op-Amp / Throttle RC filter). |
| **C1** | `0.1µF` | High-frequency decoupling/filter capacitor. |
| **R1, R3** | `10kΩ` | Standard pull-up/pull-down or filter resistors. |
| **R2** | `3.3kΩ` | Filter network resistor. |
| **R4, R5** | `220Ω` | Current-limiting resistors for LEDs. |

---

## 📡 Communication Protocol (v1.4 CRC16)

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
To safely test vehicle logic before plugging into the physical V1.4 board:
1.  Set `#define SIMULATION_MODE 1` in `vcs_constants.h`.
2.  Open **PlatformIO**, select the `env:nano33ble` environment, and click **Upload**.
3.  Monitor the OLED and Serial Monitor (**115200 baud**) for physics emulation.
4.  Ground the **DMSC** terminal to watch the system transition to **AUTO** via the digital twin.