#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>

// Pin definitions

#define DHTPIN 0   // GPIO16 for DHT11 (D3)
#define DHTTYPE DHT11
#define RELAY1 14  // GPIO14 for Relay1 (D5)
#define RELAY2 12  // GPIO12 for Relay2 (D6)
#define RELAY3 13  // GPIO13 for Relay3 (D7)
#define RELAY4 15  // GPIO15 for Relay4 (D8)

// Sensor and server objects
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
ESP8266WebServer server(80);

// Thresholds
float tempThreshold = 30.0;
float humidityThreshold = 70.0;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  digitalWrite(RELAY4, LOW);

  dht.begin();
  bmp.begin(0x76);

  WiFi.softAP("ESP8266");

  // Load thresholds from EEPROM
  loadThresholds();

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/set", handleSetThresholds);
  server.on("/toggleRelay", handleToggleRelay);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = R"=====(
    <html>
    <head>
      <title>Incubator Dashboard</title>
      <style>
        body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; background-color: #f0f0f0; }
        .container { width: 80%; margin: 0 auto; padding: 20px; background-color: #ffffff; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
        .header { background-color: #4CAF50; color: white; padding: 10px; border-radius: 10px 10px 0 0; }
        .section { margin: 20px 0; padding: 10px; border: 1px solid #ddd; border-radius: 5px; background-color: #fafafa; }
        .section h2 { margin: 0; }
        .sensor-data, .relay-control { text-align: left; }
        .sensor-data div, .relay-control div { margin-bottom: 10px; }
        .btn { padding: 10px 20px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }
        .btn-primary { background-color: #4CAF50; color: white; }
        .btn-secondary { background-color: #f44336; color: white; }
        .img-icon { width: 30px; vertical-align: middle; }
      </style>
      <script>
        function fetchData() {
          var xhr = new XMLHttpRequest();
          xhr.onreadystatechange = function() {
            if (xhr.readyState == 4 && xhr.status == 200) {
              var data = xhr.responseText.split(',');
              document.getElementById('temp').innerHTML = 'Temperature: ' + data[0] + ' °C';
              document.getElementById('humidity').innerHTML = 'Humidity: ' + data[1] + ' %';
              document.getElementById('bmpTemp').innerHTML = 'BMP Temp: ' + data[2] + ' °C';
              document.getElementById('bmpPressure').innerHTML = 'Pressure: ' + data[3] + ' hPa';
            }
          };
          xhr.open('GET', '/data', true);
          xhr.send();
        }

        function setThresholds() {
          var temp = document.getElementById('tempThreshold').value;
          var humidity = document.getElementById('humidityThreshold').value;
          var xhr = new XMLHttpRequest();
          xhr.open('GET', '/set?temp=' + temp + '&humidity=' + humidity, true);
          xhr.onreadystatechange = function() {
            if (xhr.readyState == 4 && xhr.status == 200) {
              document.getElementById('tempThreshold').value = temp;
              document.getElementById('humidityThreshold').value = humidity;
            }
          };
          xhr.send();
        }

        function toggleRelay(relay) {
          var xhr = new XMLHttpRequest();
          xhr.open('GET', '/toggleRelay?relay=' + relay, true);
          xhr.send();
        }

        setInterval(fetchData, 2000);
      </script>
    </head>
    <body onload="fetchData()">
      <div class="container">
        <div class="header">
          <h1>Incubator Dashboard</h1>
        </div>
        <div class="section sensor-data">
          <h2>Sensor Data</h2>
          <div id="temp">Loading...</div>
          <div id="humidity">Loading...</div>
          <div id="bmpTemp">Loading...</div>
          <div id="bmpPressure">Loading...</div>
        </div>
        <div class="section">
          <h2>Set Thresholds</h2>
          <input type="number" id="tempThreshold" placeholder="Temp Threshold" value=")=====" + String(tempThreshold) + R"=====(">
          <input type="number" id="humidityThreshold" placeholder="Humidity Threshold" value=")=====" + String(humidityThreshold) + R"=====(">
          <button class="btn btn-primary" onclick="setThresholds()">Set Thresholds</button>
        </div>
        <div class="section relay-control">
          <h2>Relay Control</h2>
          <div>
            <button class="btn btn-primary" onclick="toggleRelay(1)">Toggle Relay 1</button>
          </div>
          <div>
            <button class="btn btn-primary" onclick="toggleRelay(2)">Toggle Relay 2</button>
          </div>
          <div>
            <button class="btn btn-primary" onclick="toggleRelay(3)">Toggle Relay 3</button>
          </div>
          <div>
            <button class="btn btn-primary" onclick="toggleRelay(4)">Toggle Relay 4</button>
          </div>
        </div>
      </div>
    </body>
    </html>
  )=====";
  server.send(200, "text/html", html);
}

void handleData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float bmpTemperature = bmp.readTemperature();
  float bmpPressure = bmp.readPressure() / 100.0F;

  String data = String(temperature) + "," + String(humidity) + "," + String(bmpTemperature) + "," + String(bmpPressure);
  server.send(200, "text/plain", data);

  // Trigger relays based on thresholds
  if (temperature > tempThreshold) digitalWrite(RELAY1, HIGH);
  else digitalWrite(RELAY1, LOW);

  if (humidity > humidityThreshold) digitalWrite(RELAY2, HIGH);
  else digitalWrite(RELAY2, LOW);
}

void handleSetThresholds() {
  if (server.hasArg("temp") && server.hasArg("humidity")) {
    tempThreshold = server.arg("temp").toFloat();
    humidityThreshold = server.arg("humidity").toFloat();
    saveThresholds();
    server.send(200, "text/plain", "Thresholds updated");
  } else {
    server.send(400, "text/plain", "Invalid parameters");
  }
}

void handleToggleRelay() {
  if (server.hasArg("relay")) {
    int relay = server.arg("relay").toInt();
    int relayPin = 14;
    switch (relay) {
      case 1: relayPin = RELAY1; break;
      case 2: relayPin = RELAY2; break;
      case 3: relayPin = RELAY3; break;
      case 4: relayPin = RELAY4; break;
      default: server.send(400, "text/plain", "Invalid relay number"); return;
    }
    int currentState = digitalRead(relayPin);
    digitalWrite(relayPin, !currentState);  // Toggle the state
    server.send(200, "text/plain", "Relay " + String(relay) + " toggled");
  } else {
    server.send(400, "text/plain", "No relay specified");
  }
}

void saveThresholds() {
  EEPROM.put(0, tempThreshold);
  EEPROM.put(4, humidityThreshold);
  EEPROM.commit();
}

void loadThresholds() {
  EEPROM.get(0, tempThreshold);
  EEPROM.get(4, humidityThreshold);
  if (isnan(tempThreshold)) tempThreshold = 30.0;
  if (isnan(humidityThreshold)) humidityThreshold = 70.0;
}
