# ESP32 RBS Monitoring and Control

## Overview
This repository contains the firmware for an ESP32-based system designed to monitor and control a Remote Base Station (RBS) in a telecommunication network. The system provides real-time monitoring of key parameters, controls alarms, and publishes data to an MQTT server.

## Features
- Real-time monitoring of energy, battery voltage, rectifier status, and more.
- Control of generator and transfer trip alarms based on predefined conditions.
- Data publishing to an MQTT server for remote monitoring and control.
- Event logging on an SD card for historical analysis.
- Time synchronization using NTP and maintained with an RTC module.
- Configurable Wi-Fi and MQTT settings for secure communication.

## Hardware Requirements
- ESP32 microcontroller
- SD card module
- RTC module (DS3231)
- Relay modules for alarm control
- Sensors for monitoring parameters

## Software Requirements
- Arduino IDE or PlatformIO
- ArduinoJson library
- PubSubClient library (for MQTT)
- NTPClient library (for time synchronization)
- WiFi library (for ESP32)

## Installation
1. Clone this repository to your local machine.
2. Open the project in Arduino IDE or PlatformIO.
3. Install the required libraries.
4. Update the Wi-Fi and MQTT settings in the code to match your network.
5. Compile and upload the firmware to your ESP32.

## Configuration
- Update the `RBS_NAME` and other constants in the code to match your RBS specifications.
- Configure the MQTT topics and server details for your setup.

## Usage
Once the firmware is uploaded and the ESP32 is powered on, it will start monitoring the RBS parameters and control the alarms based on the predefined conditions. The monitored data will be published to the MQTT server, and alarm events will be logged on the SD card.

## Contributing
Contributions to this project are welcome. Please submit a pull request or open an issue if you have any suggestions or improvements.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
