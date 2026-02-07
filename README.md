# Traffic Light Program (using ESP32)

This is code base for traffic light controller embedded to ESP32.

## Module Parts

- ESP32 Development Board
- LED RGY Module Board
- Breadboard (optional)

## Pins

- GND
- GPIO 25 (Red Light)
- GPIO 26 (Yellow Light)
- GPIO 27 (Green Light)
- GPIO 33 (Button Input) using jumper connected to GND

## Hardware Setup

- Connect ESP32 GND to TL GND
- Connect ESP32 GPIO 25 to TL Red
- Connect ESP32 GPIO 26 to TL Yellow
- Connect ESP32 GPIO 27 to TL Green
- Case 1 (w/o button):

1. Connect ESP32 GND to Breadboard
2. Connect ESP32 GPIO 33 to GND on step 1 to change Mode

## Features

1. Normal Mode (RGY light sequence)
2. Night Mode (Yellow light blinking)

## TODO

- Add other modes
- Add real button compatibility
- Improve code base
