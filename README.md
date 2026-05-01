# SafeRoute 🚨
### *Adaptive Emergency Exit Intelligence System*

> An Arduino-based smart evacuation guidance system that detects hazards across multiple zones and dynamically routes occupants to the safest available exit in real time.

---

## Overview

SafeRoute is an embedded systems project designed to address a critical gap in conventional fire safety infrastructure: **standard alarms tell you something is wrong, but not where to go.**

The system continuously monitors three independent zones using light-dependent resistors (LDRs) as smoke proxies and a TMP36 temperature sensor for thermal detection. When a hazard is identified, SafeRoute evaluates all available exit paths and guides occupants via a color-coded LED routing array and a 16x2 LCD display, dynamically updating as conditions change.

### Key Features

- **Multi-zone hazard detection** across three independent sensor nodes
- **Adaptive LED path routing** — green for safe, red for blocked, updates in real time
- **Dual-mode detection** — smoke simulation via LDR + thermal alert via TMP36
- **Escalating audio alert** — buzzer severity scales with number of compromised zones
- **False alarm override** — button silences the system for 15 seconds, then auto re-arms
- **Live LCD display** — shows exit instructions, blocked zones, and temperature simultaneously

### Why It Matters

Building fires kill people not only from flames or smoke, but from uninformed, panicked evacuation. In under-resourced environments such as schools, community centers, and infrastructure-limited regions, there are rarely smart exit systems beyond a basic alarm. SafeRoute demonstrates a low-cost, scalable approach to intelligent evacuation guidance using commodity components costing under $15.

---

## Problem Statement

Standard smoke detectors and fire alarm systems operate on a simple binary model: hazard detected, alarm triggered. They provide no spatial awareness of where the hazard is relative to exit paths, no guidance on which direction to move, and no adaptive response as conditions evolve.

High-end smart evacuation systems exist in commercial buildings but require expensive networked infrastructure and professional installation. There is no low-cost, open-source, deployable alternative for resource-constrained environments. SafeRoute addresses that gap.

---

## Solution Overview

SafeRoute maps a building's exit zones to sensor nodes. Each node monitors ambient light as a proxy for smoke density. The Arduino evaluates all three nodes simultaneously, determines which exits are passable, activates the corresponding green LED indicators, and updates the LCD with plain-language exit instructions.

If temperature hits 55°C or above, a thermal override engages — all exits are marked dangerous and the system escalates to maximum alert. A false alarm button lets a human operator silence the system temporarily while the environment is assessed.

### A Note on Smoke Detection

TinkerCAD does not have a dedicated smoke sensor component. Since I was new to TinkerCAD and working within the simulation environment's constraints, I used photoresistors (LDRs) as a creative proxy. A drop in light level (dragging the LDR slider left) represents smoke blocking the sensor. It was a workaround that ended up working perfectly for demonstrating the core routing logic.

---

## System Architecture

```
+-----------------------------------------------------+
|                   INPUT SUBSYSTEM                   |
|  [LDR Zone A] [LDR Zone B] [LDR Zone C]  [TMP36]   |
|       A0            A1           A2          A4      |
+------------------------+----------------------------+
                         |  Analog signals
+------------------------v----------------------------+
|               PROCESSING SUBSYSTEM                  |
|                  Arduino UNO                         |
|  - Reads analog values from all sensors              |
|  - Compares LDR values against threshold (< 200)     |
|  - Evaluates temperature against fire threshold      |
|  - Determines safe/blocked status per zone           |
|  - Generates routing decision                        |
|  - Manages false alarm silence state                 |
+----------+---------------------------+--------------+
           |  Digital output           |  Parallel data
+----------v----------+   +-----------v--------------+
|   OUTPUT SUBSYSTEM  |   |    DISPLAY SUBSYSTEM     |
|  6x LEDs (R/G pairs)|   |    16x2 LCD              |
|  Buzzer             |   |    Exit instructions     |
|  D2-D7, D8          |   |    Live temperature      |
+---------------------+   |    D9-D13, A3            |
                          +--------------------------+
           |
+----------v----------+
|   OVERRIDE INPUT    |
|   Tactile Button    |
|   A5 (Digital 19)   |
+---------------------+
```

---

## Components and Materials

| Component | Qty | Role |
|---|---|---|
| Arduino UNO | 1 | Central microcontroller |
| Light Dependent Resistor (LDR) | 3 | Smoke/darkness proxy, one per zone |
| TMP36 Temperature Sensor | 1 | Detects thermal conditions consistent with fire |
| 16x2 LCD Display | 1 | Real-time exit guidance and status output |
| 10kΩ Potentiometer | 1 | LCD contrast control |
| Green LED | 3 | Safe exit path indicator per zone |
| Red LED | 3 | Blocked/hazardous exit indicator per zone |
| 10kΩ Resistor | 3 | Pull-down resistors for LDR voltage dividers |
| 220Ω Resistor | 7 | Current limiting for 6 LEDs + LCD backlight |
| Buzzer (active) | 1 | Escalating audio alert |
| Tactile Push Button | 1 | False alarm override input |
| Breadboard x2 | 2 | Circuit prototyping |
| Jumper Wires | — | Interconnects |

**Total estimated component cost:** ~$12-15 USD

---

## Circuit Design

### LDR Voltage Dividers

```
5V ---- [LDR] ---- junction ---- Arduino Analog Pin
                       |
                    [10kΩ]
                       |
                      GND
```

In ambient light, the LDR pulls the junction high (toward 1023). When light is blocked, LDR resistance increases, pulling the junction low (toward 0). A reading below 200 triggers a hazard flag.

### Pin Mapping

| Arduino Pin | Component | Notes |
|---|---|---|
| A0 | LDR Zone A | Voltage divider junction |
| A1 | LDR Zone B | Voltage divider junction |
| A2 | LDR Zone C | Voltage divider junction |
| A3 (Digital 17) | LCD DB7 | Analog pin used as digital output |
| A4 | TMP36 Vout | Middle leg |
| A5 (Digital 19) | Button | INPUT_PULLUP, press = LOW |
| D2 | Green LED Zone A | Via 220Ω |
| D3 | Red LED Zone A | Via 220Ω |
| D4 | Green LED Zone B | Via 220Ω |
| D5 | Red LED Zone B | Via 220Ω |
| D6 | Green LED Zone C | Via 220Ω |
| D7 | Red LED Zone C | Via 220Ω |
| D8 | Buzzer (+) | Active buzzer |
| D9 | LCD RS | Register select |
| D10 | LCD E | Enable |
| D11 | LCD DB4 | 4-bit parallel data |
| D12 | LCD DB5 | 4-bit parallel data |
| D13 | LCD DB6 | 4-bit parallel data |
| A3 (17) | LCD DB7 | 4-bit parallel data |

### TMP36 Wiring

Flat face forward: Left leg = 5V, Middle leg = A4, Right leg = GND. No resistor needed.

### LCD Backlight

Connected through a 220Ω resistor to limit current to ~14mA. Direct connection to 5V exceeds the 20mA maximum.

---

## Software Design

### Hazard Detection

Any LDR reading below 200 flags that zone as dangerous. This fixed threshold was chosen over dynamic calibration because TinkerCAD initializes LDR sliders at minimum light by default, making startup baseline calibration unreliable in simulation.

### Main Loop

1. Button check — detects falling edge (HIGH to LOW) for false alarm press
2. Silence handler — if silenced, shows countdown and skips hazard logic
3. Sensor reads — samples A0, A1, A2 (LDRs) and A4 (TMP36)
4. Hazard evaluation — computes danger flag per zone (readX < 200)
5. Temperature check — converts ADC to degrees C, checks against 55°C
6. LED update — sets each zone's LED pair
7. LCD update — displays appropriate system state message
8. Buzzer — severity-based tone pattern

### Key Functions

| Function | Purpose |
|---|---|
| `setLED(green, red, danger)` | Toggles one zone's LED pair |
| `updateLCD(...)` | State machine for all four LCD display modes |
| `readTempC()` | Converts TMP36 ADC reading to degrees Celsius |
| `allGreen()` | Forces all zones to safe state |

---

## Code

The full Arduino source code is available in the repository:

[SafeRoute_FINAL.ino](./SafeRoute_FINAL.ino)
```

---

## Demo / Usage Instructions

### IMPORTANT: LDR Behavior in TinkerCAD

> TinkerCAD initializes all LDR sliders at the leftmost position (minimum light) by default. This means all three zones start in a hazard state when simulation begins.
>
> **To see normal ALL CLEAR operation:** drag all three LDR sliders fully to the RIGHT after starting simulation. Then drag individual sliders left to simulate smoke per zone.

### Running the Simulation

1. Open the TinkerCAD project: `[insert share link here]`
2. Click **Simulate**
3. Wait for LCD to show "SYSTEM READY"
4. Adjust the potentiometer slider until LCD text is clearly visible
5. Drag all LDR sliders to the right for ALL CLEAR baseline

### Test Scenarios

| Test | Action | Expected Result |
|---|---|---|
| All clear | All LDR sliders fully right | All LEDs green, LCD: ALL CLEAR |
| Smoke Zone A | Drag LDR A fully left | Zone A red, LCD: GO TO EXIT B C |
| Smoke Zones A+B | Drag two LDRs left | One green path left, buzzer double-pulse |
| Fire alert | Click TMP36, drag above 55°C | All LEDs red, LCD: FIRE ALERT |
| False alarm | Press button during active alert | Silences 15s, LCD countdown |
| Re-arm | Wait 15 seconds | LCD: SYSTEM ARMED, monitoring resumes |

---

## Results and Performance

### What Worked

- LED routing responds immediately and correctly to any LDR combination
- LCD clearly communicates exit instructions across all four system states
- False alarm button reliably silences and auto re-arms
- Buzzer escalation is distinct between 1, 2, and 3 zone alerts
- Temperature display is accurate and updates in real time

### Known Limitations

- LDR sliders initialize at danger position in TinkerCAD — manual adjustment needed at simulation start
- TMP36 reads abnormally high by default in TinkerCAD simulation
- lcd.clear() on every loop causes minor flicker
- No persistent event logging

---

## Challenges and Engineering Decisions

**Fixed threshold vs. dynamic calibration** — Dynamic calibration was prototyped but dropped because TinkerCAD initializes LDRs at minimum light, making startup baseline calibration unreliable. Fixed threshold of readX < 200 was robust and testable.

**Pin budget** — Using analog pins A3 and A5 as digital I/O was necessary to fit all components without shift registers or I2C expanders. Fully supported by the Arduino framework.

**LCD backlight overcurrent** — Initial wiring drew ~23mA, above the 20mA LED limit. A 220Ω resistor brought it to ~14mA.

**Buzzer blocking** — tone() with delay() blocks the loop during alerts. Non-blocking millis() approach is planned for the physical build.

---

## Future Improvements

### Next Semester — Physical Hardware Build

- Replace LDRs with **MQ-2 gas/smoke sensors** for real smoke detection
- Implement proper **startup calibration** averaging ambient conditions over 3 seconds
- **Non-blocking buzzer** using millis() for responsive button input during alerts
- **EEPROM event logging** for timestamped hazard history
- Cleaner **wire management** — the spaghetti wiring on the breadboard gets the job done but the physical build will be much neater

### Longer Term

- **NRF24L01 radio modules** to network multiple nodes across rooms
- **Bluetooth companion app** (HC-05) for remote zone monitoring
- **Control systems level integration** — coordinated multi-room evacuation routing across an entire building via MQTT over WiFi (ESP32)

---

## Applications

| Sector | Use Case |
|---|---|
| Education | School and university evacuation guidance |
| Healthcare | Hospital corridor routing during emergencies |
| Residential | Multi-unit housing in developing regions |
| Industrial | Factory floor hazard zone routing |
| Community | Low-cost deployment in public spaces |

---

## Conclusion

SafeRoute demonstrates that intelligent, adaptive emergency guidance does not require expensive infrastructure. Using an Arduino UNO and under $15 in components, the system achieves real-time multi-zone hazard detection, dynamic exit routing, thermal override, and human override — behaviors typically reserved for high-end commercial safety systems.

Designed and simulated in Autodesk TinkerCAD for the *Press, Power, Play!* Hardware Hackathon at Grambling State University, where it won **Overall Best Circuit Design and Project**. Physical prototype coming next semester. The foundation is there. The work continues.

---

## Recognition

**Overall Best Circuit Design and Project**
Autodesk "Press, Power, Play!" Hardware Hackathon
Grambling State University — April 2026

---

## Author

**Samuel Agyapong**
B.S. Electronics Engineering Technology — Grambling State University
GitHub: [github.com/Sammuel-bot](https://github.com/Sammuel-bot)

---

*Simulated in Autodesk TinkerCAD | Autodesk "Press, Power, Play!" Hackathon 2026*
