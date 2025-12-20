# Greenhouse - MSP432 Greenhouse Automation System

This project implements a complete control system for an automated greenhouse based on the **TI MSP432P401R** microcontroller. The system monitors temperature and humidity, automatically managing actuators (fans, pumps, heating resistors, humidifiers) to maintain ideal environmental conditions, or allowing for direct manual control.

## 🌟 Key Features

* **Environmental Monitoring**: Precise reading of Temperature and Humidity via **DHT22** sensor.
* **Finite State Machine (FSM)**: Logic management through states (INIT, MANUAL, AUTOMATIC).
* **Automatic Mode**:
    * Temperature regulation (Fan for cooling, Resistor for heating).
    * Humidity regulation (Humidifier).
    * Automatic watering cycle managed by an internal timer.
* **Manual Mode**: Direct control of each individual actuator via physical buttons.
* **User Interface (UI)**: Data visualization and configuration menus on Crystalfontz 128x128 LCD display.
* **Low-Level Hardware Management**: Custom drivers for GPIO, Timers, Interrupts, and One-Wire protocol (DHT22).

## 🛠 Hardware Requirements

* **Microcontroller**: TI MSP-EXP432P401R LaunchPad.
* **Display**: Educational BoosterPack MKII.
* **Sensors**: DHT22 (Temperature and Humidity).
* **Actuators**:
    * DC Fan.
    * Water Pump.
    * Heating Resistor.
    * Humidifier Module.
* **Input**: Physical Buttons and Switch/Lever.

## 🔌 Pin Configuration (Pinout)

Pins are defined in `source/hardware.c`. Ensure peripherals are connected as follows:

| Peripheral | MSP432 Port/Pin | Notes |
| :--- | :--- | :--- |
| **DHT22 Sensor** | `P2.5` | Requires pull-up resistor |
| **Fan** | `P1.0` | Output Active High |
| **Pump** | `P2.1` | Output Active High |
| **Resistor** | `P3.2` | Output Active High |
| **Humidifier** | `P4.3` | Pulse control |
| **Button S1** | `P5.1` | Navigation / Settings |
| **Button S2** | `P3.5` | Up / Increment |
| **Button S3** | `P1.5` | Down / Decrement (Joystick) |
| **Man/Auto Switch**| `P3.1` | Mode selection lever |

## 📂 Project Structure
```
.
├── .ccsproject
├── .cproject
├── .gitignore
├── .launches
│   └── Greenhouse.launch
├── .project
├── .settings
│   ├── org.eclipse.cdt.codan.core.prefs
│   ├── org.eclipse.cdt.debug.core.prefs
│   └── org.eclipse.core.resources.prefs
├── LcdDriver
│   ├── Crystalfontz128x128_ST7735.c
│   ├── Crystalfontz128x128_ST7735.h
│   ├── HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.c
│   └── HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h
├── NOTES HOW TO RUN TEST_MAIN.c.txt
├── README.md
├── include
│   ├── dht22.h
│   ├── hardware.h
│   ├── states.h
│   └── ui.h
├── msp432p401r.cmd
├── source
│   ├── dht22.c
│   ├── hardware.c
│   ├── main.c
│   ├── states.c
│   ├── test_main.c
│   └── ui.c
├── startup_msp432p401r_ccs.c
├── system_msp432p401r.c
├── targetConfigs
│   ├── MSP432P401R.ccxml
│   └── readme.txt
└── test_suite
```
## 🚀 How to Use

### 1. Manual Mode
Activated via the physical switch.
* Use **S1** to cycle through components (Fan -> Humidifier -> Resistor -> Pump).
* Use **S2** (Up) to turn **ON** the selected component.
* Use **S3** (Down) to turn **OFF** the selected component.

### 2. Automatic Mode
The system reads sensors every 3 seconds.
* If `Temp > Target`: Turns on the **Fan**.
* If `Temp < Target`: Turns off the Fan / Turns on **Heating**.
* If `Hum < Target`: Activates the **Humidifier**.
* Watering occurs cyclically based on the configured timer.

### 3. Settings
Allows configuration of target values for Temperature, Humidity, and Water quantity..

## 💻 Compilation and Testing

### Software Requirements
* **IDE**: Code Composer Studio (CCS).
* **SDK**: SimpleLink MSP432 SDK (DriverLib).

### Running the Test Suite (PC)
The project includes a test mode to verify logic without hardware (`test_main.c`).
To compile on Linux/gcc for logic tests:

```bash
gcc -DTEST_MODE source/test_main.c source/states.c source/hardware.c source/dht22.c -o greenhouse_test
./greenhouse_test