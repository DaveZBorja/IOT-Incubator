# IOT Incubator Dashboard

This project uses an ESP8266 microcontroller to create a web server that monitors and controls an incubator. The server reads data from temperature and humidity sensors, displays this information on a web page, and allows you to set thresholds for controlling relays.

## Features

- **Sensor Data**: Displays real-time temperature and humidity from DHT11 and BMP280 sensors.
- **Thresholds**: Allows setting temperature and humidity thresholds.
- **Relay Control**: Toggles four relays to control various components of the incubator.
- **Web Interface**: Provides a user-friendly dashboard accessible via a web browser.

## Components

- **ESP8266 Microcontroller**: Runs the web server and handles sensor data.
- **DHT11 Sensor**: Measures temperature and humidity.
- **BMP280 Sensor**: Measures additional temperature and atmospheric pressure.
- **Relays**: Control external devices such as heaters or fans.
- **EEPROM**: Stores user-set thresholds.

## Pin Definitions

- **DHT11 Sensor**: GPIO16 (D3)
- **Relay 1**: GPIO14 (D5)
- **Relay 2**: GPIO12 (D6)
- **Relay 3**: GPIO13 (D7)
- **Relay 4**: GPIO15 (D8)

## Libraries Used

- `ESP8266WiFi.h` - For WiFi functionality.
- `ESP8266WebServer.h` - To create and manage the web server.
- `EEPROM.h` - For storing and retrieving threshold values.
- `DHT.h` - For interfacing with the DHT11 sensor.
- `Adafruit_BMP280.h` - For interfacing with the BMP280 sensor.

## Installation

1. **Hardware Setup**:
   - Connect the DHT11 sensor to GPIO16.
   - Connect the BMP280 sensor to I2C pins (default address 0x76).
   - Connect the relays to GPIO14, GPIO12, GPIO13, and GPIO15.

2. **Software Setup**:
   - Install the required libraries in the Arduino IDE.
   - Upload the code to your ESP8266 board using the Arduino IDE.

## Web Interface

- **Root Page (`/`)**: Displays the incubator dashboard with sensor data, thresholds, and relay controls.
- **Data Endpoint (`/data`)**: Provides raw sensor data in CSV format.
- **Set Thresholds Endpoint (`/set`)**: Allows updating temperature and humidity thresholds.
- **Toggle Relay Endpoint (`/toggleRelay`)**: Toggles the state of the specified relay.

## Example Usage

1. Open the serial monitor to see the ESP8266's IP address.
2. Connect to the WiFi network named `IOT Incubator`.
3. Open a web browser and navigate to the IP address to access the dashboard.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Author

Dave Borja

---



