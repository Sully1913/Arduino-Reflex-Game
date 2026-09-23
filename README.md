# Arduino Reflex Game ⏱️

## Project Scope
This system is a hardware-software integration designed to measure and analyze human reaction times. It encompasses the full lifecycle of an embedded device, from raw electronic component soldering to the implementation of the microcontroller's control logic.

## Core Capabilities
* **Reaction Measurement:** Millisecond-precision timing loop for user input.
* **Hardware Debouncing:** Interrupt-driven button handling preventing signal noise.
* **Physical Feedback:** Direct integration with an LCD/LED display for immediate visual output.
* **Standalone Operation:** Fully self-contained device logic executing independently on the microcontroller.

## Architecture Overview
* **Hardware Layer:** Custom-designed and manually soldered circuit board connecting physical switches and displays to the microcontroller pins.
* **Software Layer:** Embedded C++ logic handling I/O initialization, state machine progression, and real-time hardware interrupts.

## Technology Stack
* **Language:** C++
* **Platform:** Arduino Uno (AVR microcontrollers)
* **Hardware:** Custom circuitry, LEDs/LCD, physical tactile switches
* **Environment:** Arduino IDE

## Screenshots & Demo
*Gameplay demonstration:*
<img width="800" height="463" alt="ab740343-18bb-4a16-be27-e2f17505c98f-ezgif com-video-to-gif-converter" src="https://github.com/user-attachments/assets/574a8cfb-1416-4cd3-a9c0-2a6fb1f3948d" />

*Assembled hardware:*
<img width="1536" height="2048" alt="78c0de6c-bbd5-4f72-b595-55973ee510eb" src="https://github.com/user-attachments/assets/24492b3a-5ca6-4fec-8ec0-9c9ecb3bb8be" />
