#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <esp_now.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

const char* ssid     = "Dubeyji5G";
const char* password = "Anujdubey@1506";

#define L_IN1  16
#define L_IN2  17
#define L_IN3  18
#define L_IN4  19
#define L_ENA  32
#define L_ENB  33

#define R_IN1  21
#define R_IN2  22
#define R_IN3  23
#define R_IN4  25
#define R_ENA  26
#define R_ENB  27

#define SERVO_PIN 2
Servo headServo;

// ✅ Safe scanning angles (45° to 135°) to prevent servo stalling
const int servoAngles[] = {90, 135, 90, 45}; 
int   servoAngleIndex = 0; 
int   servoAngle      = 90;
unsigned long lastServoMove = 0;
bool  servoSweepActive = true;

volatile bool newCommand = false;
char          cmdBuffer[32];

// ================================================================
// SPEED CONTROL — change motorSpeed to adjust (0=stop, 255=full)
// ================================================================
int motorSpeed = 220; // 85% speed — strong, smooth and controlled

void enableMotors() {
  analogWrite(L_ENA, motorSpeed);
  analogWrite(L_ENB, motorSpeed);
  analogWrite(R_ENA, motorSpeed);
  analogWrite(R_ENB, motorSpeed);
}

void stopMotors() {
  analogWrite(L_ENA, 0);
  analogWrite(L_ENB, 0);
  analogWrite(R_ENA, 0);
  analogWrite(R_ENB, 0);
  digitalWrite(L_IN1, LOW);  digitalWrite(L_IN2, LOW);
  digitalWrite(L_IN3, LOW);  digitalWrite(L_IN4, LOW);
  digitalWrite(R_IN1, LOW);  digitalWrite(R_IN2, LOW);
  digitalWrite(R_IN3, LOW);  digitalWrite(R_IN4, LOW);
  Serial.println("[MOTOR] ⏹ STOP");
}

void forward() {
  enableMotors();
  // Left Side
  digitalWrite(L_IN1, HIGH); digitalWrite(L_IN2, LOW);
  digitalWrite(L_IN3, HIGH); digitalWrite(L_IN4, LOW);
  // Right Side
  digitalWrite(R_IN1, HIGH); digitalWrite(R_IN2, LOW);
  digitalWrite(R_IN3, HIGH); digitalWrite(R_IN4, LOW);
  Serial.println("[MOTOR] ⬆ FORWARD | Speed: " + String(motorSpeed));
}

void backward() {
  enableMotors();
  // Left Side
  digitalWrite(L_IN1, LOW);  digitalWrite(L_IN2, HIGH);
  digitalWrite(L_IN3, LOW);  digitalWrite(L_IN4, HIGH);
  // Right Side
  digitalWrite(R_IN1, LOW);  digitalWrite(R_IN2, HIGH);
  digitalWrite(R_IN3, LOW);  digitalWrite(R_IN4, HIGH);
  Serial.println("[MOTOR] ⬇ BACKWARD | Speed: " + String(motorSpeed));
}

void turnLeft() {
  enableMotors();
  // Left Side (Spin Backward)
  digitalWrite(L_IN1, LOW);  digitalWrite(L_IN2, HIGH);
  digitalWrite(L_IN3, LOW);  digitalWrite(L_IN4, HIGH);
  // Right Side (Spin Forward)
  digitalWrite(R_IN1, HIGH); digitalWrite(R_IN2, LOW);
  digitalWrite(R_IN3, HIGH); digitalWrite(R_IN4, LOW);
  Serial.println("[MOTOR] ⬅ LEFT | Speed: " + String(motorSpeed));
}

void turnRight() {
  enableMotors();
  // Left Side (Spin Forward)
  digitalWrite(L_IN1, HIGH); digitalWrite(L_IN2, LOW);
  digitalWrite(L_IN3, HIGH); digitalWrite(L_IN4, LOW);
  // Right Side (Spin Backward)
  digitalWrite(R_IN1, LOW);  digitalWrite(R_IN2, HIGH);
  digitalWrite(R_IN3, LOW);  digitalWrite(R_IN4, HIGH);
  Serial.println("[MOTOR] ➡ RIGHT | Speed: " + String(motorSpeed));
}

void processCommand(String cmd) {
  cmd.trim();
  Serial.println("[ESP-NOW] 📩 CMD: " + cmd);

  if      (cmd == "F")   forward();
  else if (cmd == "B")   backward();
  else if (cmd == "L")   turnLeft();
  else if (cmd == "R")   turnRight();
  else if (cmd == "S")   stopMotors();

  // ✅ Pump button repurposed as STOP
  else if (cmd == "P0")  stopMotors();
  else if (cmd == "P1")  stopMotors();

  // Speed control — master can send "SPD:150" to set speed
  else if (cmd.startsWith("SPD:")) {
    int spd = cmd.substring(4).toInt();
    motorSpeed = constrain(spd, 60, 255);
    Serial.println("[SPEED] ⚡ Set to: " + String(motorSpeed));
  }

  else if (cmd == "HF1") Serial.println("[MODE] 👤 Human Follow ON");
  else if (cmd == "HF0") Serial.println("[MODE] 👤 Human Follow OFF");
  else if (cmd.startsWith("V:")) Serial.println("[VOICE] 🎤 " + cmd.substring(2));
  else Serial.println("[WARN] ⚠ Unknown: " + cmd);
}

void onDataReceive(const esp_now_recv_info_t *recv_info,
                   const uint8_t *data, int len) {
  int copyLen = min(len, 31);
  memcpy((void*)cmdBuffer, data, copyLen);
  cmdBuffer[copyLen] = '\0';
  newCommand = true;
}

void updateServo() {
  if (!servoSweepActive) return;
  // ✅ 1.5 seconds per step = completes Center -> Left -> Center -> Right in 4.5s
  if (millis() - lastServoMove >= 1500) {
    lastServoMove = millis();
    servoAngleIndex = (servoAngleIndex + 1) % 4;
    servoAngle = servoAngles[servoAngleIndex];
    headServo.write(servoAngle);
    Serial.print("[SERVO] 📐 Angle → ");
    Serial.println(servoAngle);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(500);
  Serial.println("\n");
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║   SLAVE ESP32 — ESP-NOW MODE          ║");
  Serial.println("╚══════════════════════════════════════╝");

  pinMode(L_IN1, OUTPUT); pinMode(L_IN2, OUTPUT);
  pinMode(L_IN3, OUTPUT); pinMode(L_IN4, OUTPUT);
  pinMode(L_ENA, OUTPUT); pinMode(L_ENB, OUTPUT);
  pinMode(R_IN1, OUTPUT); pinMode(R_IN2, OUTPUT);
  pinMode(R_IN3, OUTPUT); pinMode(R_IN4, OUTPUT);
  pinMode(R_ENA, OUTPUT); pinMode(R_ENB, OUTPUT);

  stopMotors();
  Serial.println("✅ Motor pins initialized | Default speed: " + String(motorSpeed));

  headServo.attach(SERVO_PIN);
  headServo.write(90); // Initialize at Center (90°)
  lastServoMove = millis();
  Serial.println("✅ Servo initialized at 90° (Center)");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("📶 Connecting to WiFi '" + String(ssid) + "'");
  
  // ✅ Reduced attempts to 6 (approx 3 seconds total) to prevent long boot delays
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 6) {
    delay(500); Serial.print("."); attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" ✅ Connected!");
    Serial.println("─────────────────────────────────────");
    Serial.print("🔑 SLAVE MAC ADDRESS → ");
    Serial.println(WiFi.macAddress());
    Serial.print("📡 WiFi Channel: ");
    Serial.println(WiFi.channel());
    Serial.println("─────────────────────────────────────");
  } else {
    Serial.println("\n⚠️  WiFi failed!");
    Serial.print("🔑 SLAVE MAC ADDRESS → ");
    Serial.println(WiFi.macAddress());
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW Init FAILED! Rebooting...");
    delay(3000); ESP.restart(); return;
  }
  esp_now_register_recv_cb(onDataReceive);
  Serial.println("✅ ESP-NOW Ready!");
  Serial.println("⏳ Waiting for commands from Master...");
  Serial.println("═══════════════════════════════════════");
}

void loop() {
  if (newCommand) {
    newCommand = false;
    processCommand(String(cmdBuffer));
  }
  updateServo();
}