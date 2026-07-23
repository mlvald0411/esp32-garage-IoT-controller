# esp32-garage-IoT-controller

## Overview
A simple, web-based IoT garage management system built upon the ESP32 microcontroller. This locally-hosted system provides simple garage door control, parking proximity warning, and an informational dashboard for monitoring indoor ambient temperature and humidity.


---

## Installation
### Dependencies

- Adafruit_Sensor.h
- [DHT.h](https://github.com/adafruit/DHT-sensor-library) 
- Wire.h
- LiquidCrystal_I2C.h

### Equipment

- ESP32-C6-WROOM-1
- DHT11 Sensor
- I2C 1602 LCD Display Module
- Passive Buzzer (1x)

## Acknowledgments
* Portions of the time synchronization logic are based on the [espressif/arduino-esp32](https://github.com/espressif/arduino-esp32) SimpleTime example.