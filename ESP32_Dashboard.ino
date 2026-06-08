#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_now.h>

// =======================
// WiFi Credentials
// =======================
const char* ssid = "Dubeyji5G";
const char* password = "Anujdubey@1506";

// =======================
// WEB SERVER
// =======================
AsyncWebServer server(80);

// =======================
// SENSOR PINS
// =======================
#define FLAME_DO_PIN     27
#define MQ_AO_PIN        34
#define MQ_DO_PIN        32
#define PIR_PIN          33
#define IR_LEFT_PIN      25
#define IR_RIGHT_PIN     26
#define TRIG_PIN         5
#define ECHO_PIN         18

// =======================
// SENSOR VARIABLES
// =======================
int flameValue = 0;
int smokeValue = 0;
int smokeDigital = 0;
bool pirValue = false;
bool irLeft = false;
bool irRight = false;
long distanceCM = 0;
long lastValidDistance = 0;

// =======================
// TIMERS
// =======================
unsigned long lastSensorUpdate = 0;
unsigned long lastSerialPrint = 0;

// =======================
// ESP-NOW SETUP
// Slave MAC: 30:76:F5:93:8E:54
// =======================
uint8_t slaveMac[] = {0x30, 0x76, 0xF5, 0x93, 0x8E, 0x54};
esp_now_peer_info_t peerInfo;

// ESP32 board v3.x callback signature
void onDataSent(const wifi_tx_info_t *txInfo, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "📤 ESP-NOW: Delivered" : "📤 ESP-NOW: Failed");
}

void sendToSlave(String cmd) {
  cmd.trim();
  uint8_t data[32];
  int len = min((int)cmd.length(), 31);
  memcpy(data, cmd.c_str(), len);
  data[len] = '\0';
  esp_now_send(slaveMac, data, len + 1);
  Serial.println("📤 SENT → SLAVE: " + cmd);
}

// =======================
// READ ULTRASONIC
// =======================
long readUltrasonic() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  long distance = duration * 0.034 / 2;

  if (distance > 0 && distance <= 400) {
    lastValidDistance = distance;
  }

  return lastValidDistance;
}

// =======================
// UPDATE ALL SENSORS
// =======================
void updateSensors() {

  flameValue = digitalRead(FLAME_DO_PIN);
  smokeValue = analogRead(MQ_AO_PIN);
  smokeDigital = digitalRead(MQ_DO_PIN);
  pirValue = digitalRead(PIR_PIN);
  irLeft = digitalRead(IR_LEFT_PIN);
  irRight = digitalRead(IR_RIGHT_PIN);
  distanceCM = readUltrasonic();
}

// =======================
// SENSOR API
// =======================
void handleSensors(AsyncWebServerRequest *request) {

  StaticJsonDocument<256> doc;

  doc["flame"] = flameValue;
  doc["smoke"] = smokeValue;
  doc["smokeDigital"] = smokeDigital;
  doc["pir"] = pirValue;
  doc["irLeft"] = irLeft;
  doc["irRight"] = irRight;
  doc["ultrasonic"] = distanceCM;

  String json;
  serializeJson(doc, json);

  request->send(200, "application/json", json);
}

// =======================
// WIFI CONNECT
// =======================
void connectWiFi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Connected to WiFi");
  Serial.println(WiFi.localIP());
}

// =======================
// SETUP
// =======================
void setup() {

  Serial.begin(115200);
  Serial.println("\n🚀 Booting ESP32...");

  // =======================
  // SENSOR PIN MODES
  // =======================
  pinMode(FLAME_DO_PIN, INPUT);
  pinMode(MQ_DO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(IR_LEFT_PIN, INPUT);
  pinMode(IR_RIGHT_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // =======================
  // WiFi Connection
  // =======================
  connectWiFi();

  // =======================
  // ESP-NOW INIT
  // =======================
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW Init Failed!");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, slaveMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Failed to add peer!");
    return;
  }

  Serial.println("✅ ESP-NOW Ready!");

  // =======================
  // LittleFS
  // =======================
  if (!LittleFS.begin(false)) {
    Serial.println("❌ LittleFS Mount Failed!");
    return;
  }

  Serial.println("✅ LittleFS Mounted Successfully");

  File root = LittleFS.open("/");
  File file = root.openNextFile();

  Serial.println("\n📂 Files Found:");
  while (file) {
    Serial.print("FOUND FILE: ");
    Serial.print(file.name());
    Serial.print(" (");
    Serial.print(file.size());
    Serial.println(" bytes)");
    file = root.openNextFile();
  }

  // =======================
  // SERVER STATIC
  // =======================
  server.serveStatic("/", LittleFS, "/")
        .setDefaultFile("index.html");

  server.on("/api/test", HTTP_GET,
    [](AsyncWebServerRequest *request) {
      request->send(200, "application/json",
        "{\"status\":\"ESP32 dashboard working\"}");
    }
  );

  server.on("/sensors", HTTP_GET, handleSensors);

  // =======================
  // COMMAND ROUTES
  // =======================
  server.on("/command", HTTP_GET, [](AsyncWebServerRequest *request) {

    if (request->hasParam("dir")) {
      String dir = request->getParam("dir")->value();

      if (dir == "forward") sendToSlave("F");
      else if (dir == "backward") sendToSlave("B");
      else if (dir == "left") sendToSlave("L");
      else if (dir == "right") sendToSlave("R");
      else if (dir == "stop") sendToSlave("S");
    }

    request->send(200, "text/plain", "OK");
  });

  server.on("/pump", HTTP_GET, [](AsyncWebServerRequest *request) {

    if (request->hasParam("state")) {
      String state = request->getParam("state")->value();

      if (state == "on") sendToSlave("P1");
      else sendToSlave("P0");
    }

    request->send(200, "text/plain", "OK");
  });

  server.on("/voice", HTTP_GET, [](AsyncWebServerRequest *request) {

    if (request->hasParam("cmd")) {
      String cmd = request->getParam("cmd")->value();
      sendToSlave("V:" + cmd);
    }

    request->send(200, "text/plain", "OK");
  });

  server.on("/human_follow", HTTP_GET, [](AsyncWebServerRequest *request) {

    if (request->hasParam("state")) {
      String state = request->getParam("state")->value();

      if (state == "on") sendToSlave("HF1");
      else sendToSlave("HF0");
    }

    request->send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("\n🚀 Dashboard server started");
}

// =======================
// LOOP
// =======================
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi disconnected!");
    WiFi.disconnect();
    connectWiFi();
  }

  if (millis() - lastSensorUpdate >= 100) {
    lastSensorUpdate = millis();
    updateSensors();
  }

  if (millis() - lastSerialPrint >= 1000) {
    lastSerialPrint = millis();

    Serial.printf(
      "Flame:%d | Smoke:%d | SmokeDO:%d | PIR:%d | IRL:%d | IRR:%d | Dist:%ld cm\n",
      flameValue,
      smokeValue,
      smokeDigital,
      pirValue,
      irLeft,
      irRight,
      distanceCM
    );
  }
}