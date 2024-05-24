# MQTT and SENSORS

## Description

This project utilizes a ESP32 and a DHT11 temperature and humidity sensor along with a water sensor to collect data and transmit it via MQTT (Message Queuing Telemetry Transport).

## Installation

To install the project, follow these steps:

1. Clone the repository from GitHub: `git clone https://github.com/jorgerpg/Mqtt_Sensors.git`
2. Install any dependencies required by the project.
3. Configure the MQTT broker settings in the code.

## Usage

To use the project:

1. Connect the DHT11 sensor and water sensor to the appropriate GPIO pins on your microcontroller.
2. Compile and upload the code to your microcontroller.
3. Ensure that the MQTT broker is running and accessible.
4. The device will start publishing sensor data to the specified MQTT topics.

## Configuration

You can configure the project by modifying the following settings:

- MQTT broker address and port
- MQTT topics for publishing sensor data
- GPIO pins used by the DHT11 sensor and water sensor

## Contributing

Contributions to the project are welcome! Please follow these guidelines:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your changes and test them thoroughly.
4. Submit a pull request detailing your changes.