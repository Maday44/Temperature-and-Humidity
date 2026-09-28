#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

const char* ssid     = "Vodafone28235F";
const char* password = "nzPPmFbZNtyE3nTN";

#define DHTPIN  4        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);

String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    return "--";
  }
  Serial.println(t);
  return String(t);
}

String readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    return "--";
  }
  Serial.println(h);
  return String(h);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP DHT Server</title>
  <style>
    body { font-family: Arial; text-align: center; }
    p { font-size: 2rem; }
  </style>
</head>
<body>
  <h2>ESP DHT Server</h2>
  <p>Temperature: <span id="temperature">%TEMPERATURE%</span> &deg;C</p>
  <p>Humidity: <span id="humidity">%HUMIDITY%</span> &percnt;</p>
<script>
function update(id, path) {
  fetch(path).then(r => r.text()).then(t => document.getElementById(id).innerHTML = t);
}
setInterval(() => { update("temperature", "/temperature"); update("humidity", "/humidity"); }, 10000);
</script>
</body>
</html>
)rawliteral";


String processor(const String& var) {
  if (var == "TEMPERATURE") return readDHTTemperature();
  if (var == "HUMIDITY")    return readDHTHumidity();
  return String();
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", readDHTTemperature());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", readDHTHumidity());
  });

  server.begin();
}

void loop() {

}
