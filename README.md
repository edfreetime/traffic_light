# Traffic Light Program (using ESP32)

This is code base for traffic light controller embedded to ESP32.

## Module Parts

- ESP32 Development Board
- LED RGY Module Board
- Switch Button or Push Button
- Breadboard (optional)

## Pins

- GND
- GPIO 25 (Red Light)
- GPIO 26 (Yellow Light)
- GPIO 27 (Green Light)
- GPIO 33 (Button Input) connected to GND

## Hardware Setup

- Connect ESP32 GND to TL GND
- Connect ESP32 GPIO 25 to TL Red
- Connect ESP32 GPIO 26 to TL Yellow
- Connect ESP32 GPIO 27 to TL Green
- Connect ESP32 GPIO 33 to Button pin
- Connect ESP32 GND to Button pin

## Features

Switch between mode mechanism:

Push or click button to switch mode. Mode sequence is: Normal Mode (default),
Night Mode (first button click) and Maintenance Mode (second button click).

1. Normal Mode (RGY light sequence)
2. Night Mode (Yellow light blinking)
3. Maintenance Mode (All lights blinking)

## TODO

- [x] Add other modes
- [x] Add real button compatibility
- [ ] Improve code base
