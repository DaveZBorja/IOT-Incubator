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
    <meta charset='UTF-8' /><meta name='viewport' content='width=device-width, initial-scale=1.0' />

      <title>Incubator Dashboard</title>
      <style>
        body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; background-color: #f0f0f0; }
        .container { width: 80%; margin: 0 auto; padding: 20px; background-color: #ffffff; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
        //.header { background-color: #4CAF50; color: white; padding: 10px; border-radius: 10px 10px 0 0; }
        //header img { width: 50px; vertical-align: middle; }
        .header { background-color: #4CAF50; color: white; padding: 10px; border-radius: 10px 10px 0 0; display: flex; align-items: center; justify-content: center; }
        .header img { width: 50px; margin-right: 15px; }
        .header h1 { margin: 0; }
        .section { margin: 20px 0; padding: 10px; border: 1px solid #ddd; border-radius: 5px; background-color: #fafafa; }
        .section h2 { margin: 0; }
        .sensor-data, .relay-control { text-align: left; }
        .sensor-data div, .relay-control div { margin-bottom: 10px; }
        .btn { padding: 10px 20px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }
        .btn-primary { background-color: #4CAF50; color: white; }
        .btn-secondary { background-color: #f44336; color: white; }
        .img-icon { width: 30px; vertical-align: left; }
        .footer { margin-top: 20px; padding: 10px; background-color: #333; color: white; font-size: 14px; }
        .footer p { margin: 0; }
         table {
           // width: 100%;
           border-collapse: collapse;
            //margin: 20px 0;
            //font-size: 18px;
            text-align: left;
        }
        th, td {
            //padding: 12px;
            border: 1px solid #ddd;
        }
        th {
            background-color: #4CAF50;
            color: white;
        }
        tr:nth-child(even) {
            background-color: #f2f2f2;
        }
        
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
        <img src= data:image/png;base64,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 alt="Egg Logo">
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
        <h2>Temperature and Humidity Requirements</h2>
    <table>
        <tr>
            <th>Species</th>
            <th>Days</th>
            <th>Temperature</th>
            <th>Humidity</th>
        </tr>
        <tr>
            <td>Chicken</td>
            <td>21</td>
            <td>38°C</td>
            <td>55-70%</td>
        </tr>
        <tr>
            <td>Duck</td>
            <td>28</td>
            <td>37.8°C</td>
            <td>60-75%</td>
        </tr>
        <tr>
            <td>Pigeon</td>
            <td>18</td>
            <td>38.5°C</td>
            <td>55-70%</td>
        </tr>
    </table> 
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
            <p>Main Heater </p>
            <button class="btn btn-primary" onclick="toggleRelay(1)">Push</button> 
          </div>
          <div>
            <p>Main Humidity </p>
            <button class="btn btn-primary" onclick="toggleRelay(2)">Push</button>
          </div>
          <div>
            <p>Relay 3 </p>
            <button class="btn btn-primary" onclick="toggleRelay(3)">Push</button>
          </div>
          <div>
          <p>Relay 4 </p>
            <button class="btn btn-primary" onclick="toggleRelay(4)">Push</button>
          </div>
        </div>
      </div>
      <div class="footer">
        <p>&copy; 2024 Dave Borja. All rights reserved.</p>
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
