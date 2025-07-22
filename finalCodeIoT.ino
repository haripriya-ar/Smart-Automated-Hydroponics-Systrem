
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// ------------------- Sensor & Pin Definitions -------------------
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define RELAY_PIN 18
#define TRIG_PIN 25
#define ECHO_PIN 26
#define PH_PIN 34
#define PH_TEMP_PIN 35
#define SERVO_PIN 13
#define TANK_HEIGHT_CM 7.0

// ------------------- Wi-Fi Credentials -------------------
const char* ssid = "SANJU";
const char* password = "sanju2453";

// ------------------- Firebase Configuration -------------------
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

#define API_KEY "AIzaSyDFOoLm6hkw6Skb2o5pxo2x3ex74Dz4dlw"
#define DATABASE_URL "https://hydroponicsa3-default-rtdb.asia-southeast1.firebasedatabase.app"

// ------------------- Telegram Bot -------------------
#define BOT_TOKEN "7696362630:AAGtNFIz33K3QAjh4qiigE6SV0Li7kkDVKk"
#define CHAT_ID "1886130033"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ------------------- Timers -------------------
unsigned long lastDisplayTime = 0;
unsigned long lastAlertTime = 0;
const long alertInterval = 60000;

enum ServoState { IDLE_0, MOVING_TO_90, HOLD_90, MOVING_BACK_0 };
ServoState servoState = IDLE_0;
unsigned long servoTimer = 0;
const unsigned long interval_0_wait = 5 * 60 * 1000UL;
const unsigned long move_delay = 2000;
const unsigned long hold_90_duration = 10000;

Servo servo;

unsigned long lastPumpTime = 0;
const unsigned long pumpInterval = 2 * 60 * 1000UL;
const unsigned long pumpDuration = 10 * 1000UL;
bool isScheduledPumpRunning = false;
unsigned long pumpStartTime = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT); digitalWrite(RELAY_PIN, HIGH);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(PH_PIN, INPUT); pinMode(PH_TEMP_PIN, INPUT);
  servo.attach(SERVO_PIN); servo.write(0);
  servoTimer = millis();

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  client.setInsecure();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi Connected!");
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = "salujasreekumar2001@gmail.com";
  auth.user.password = "injlro3d";
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(2);
  Serial.println("✅ Firebase initialized");
}

float readWaterLevel() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  if (duration == 0) return -1;
  float distance = duration * 0.034 / 2.0;
  if (distance < 1 || distance > TANK_HEIGHT_CM + 10) return -1;
  float level = TANK_HEIGHT_CM - distance;
  return level < 0 ? 0 : level;
}

float readPH() {
  int phRaw = analogRead(PH_PIN);
  float voltage = phRaw * (3.3 / 4095.0);
  if (voltage < 0.1) return -1;
  float ph = voltage * 3.5;
  return ph;
}

float readPHTemperature() {
  int raw = analogRead(PH_TEMP_PIN);
  float voltage = raw * (3.3 / 4095.0);
  float temperature = (voltage - 0.5) * 100.0;
  return temperature;
}

void handleServoCycle() {
  unsigned long currentMillis = millis();
  switch (servoState) {
    case IDLE_0:
      if (currentMillis - servoTimer >= interval_0_wait) {
        servo.write(90);
        Serial.println("Servo moving to 90°...");
        servoTimer = currentMillis;
        servoState = MOVING_TO_90;
      } break;
    case MOVING_TO_90:
      if (currentMillis - servoTimer >= move_delay) {
        Serial.println("⏳ Holding at 90° for 10 seconds...");
        servoTimer = currentMillis;
        servoState = HOLD_90;
      } break;
    case HOLD_90:
      if (currentMillis - servoTimer >= hold_90_duration) {
        servo.write(0);
        Serial.println("↩ Moving back to 0°...");
        servoTimer = currentMillis;
        servoState = MOVING_BACK_0;
      } break;
    case MOVING_BACK_0:
      if (currentMillis - servoTimer >= move_delay) {
        Serial.println("✅ Servo reset to 0°, waiting 5 minutes...");
        servoTimer = currentMillis;
        servoState = IDLE_0;
      } break;
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ Wi-Fi lost. Reconnecting...");
    WiFi.begin(ssid, password);
    delay(1000);
    return;
  }

  float waterLevel = readWaterLevel();
  float phValue = readPH();
  float phSensorTemp = readPHTemperature();

  if (waterLevel == -1) {
    Serial.println("❌ Ultrasonic sensor error");
  } else {
    Serial.print("✅ Water Level (cm): "); Serial.println(waterLevel);
    if (waterLevel < 2) {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("🔴 Water Level LOW — Pump ON");
      if (millis() - lastAlertTime > alertInterval) {
        String msg = "🚨 <b>Water Level Alert!</b>\nWater level is LOW: " + String(waterLevel, 1) + " cm";
        bot.sendMessage(CHAT_ID, msg, "HTML");
        lastAlertTime = millis();
      }
    } else {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("✅ Water level OK — Pump OFF");
    }
  }

  if ((phValue != -1) && (phValue < 6 || phValue > 8) && (millis() - lastAlertTime > alertInterval)) {
    String msg = "⚠ <b>pH Level Alert!</b>\nCurrent pH: " + String(phValue, 2) + "\nRecommended: 5.5–6.5";
    bot.sendMessage(CHAT_ID, msg, "HTML");
    lastAlertTime = millis();
  }

  if (millis() - lastDisplayTime > 10000) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    Serial.println("--------------------------");
    Serial.println("📊 Sensor Readings:");
    Serial.print("🌡 DHT Temp: "); Serial.print(temp); Serial.println(" °C");
    Serial.print("🌡 pH Sensor Temp: "); Serial.print(phSensorTemp); Serial.println(" °C");
    Serial.print("💧 Humidity: "); Serial.print(hum); Serial.println(" %");
    Serial.print("🔬 pH Value: "); phValue == -1 ? Serial.println("Not immersed") : Serial.println(phValue, 2);
    Serial.print("📏 Water Level: "); Serial.println(waterLevel, 2);
    Serial.println("--------------------------");

    // 🛠 FIXED: Use millis() instead of broken timestamp
    String timestamp = String(millis());
    String path = "/readings/" + timestamp;
    Serial.println("Writing to Firebase path: " + path);

    bool tOK = Firebase.RTDB.setFloat(&firebaseData, path + "/temperature", temp);
    bool hOK = Firebase.RTDB.setFloat(&firebaseData, path + "/humidity", hum);
    bool pHOK = Firebase.RTDB.setFloat(&firebaseData, path + "/ph", phValue);
    bool wOK = Firebase.RTDB.setFloat(&firebaseData, path + "/water_level", waterLevel);
    bool ptOK = Firebase.RTDB.setFloat(&firebaseData, path + "/ph_sensor_temp", phSensorTemp);

    if (tOK && hOK && pHOK && wOK && ptOK) {
      Serial.println("✅ All values sent to Firebase!");
    } else {
      Serial.print("❌ Firebase Error: ");
      Serial.println(firebaseData.errorReason());
    }

    lastDisplayTime = millis();
  }

  if (!Firebase.ready()) {
    Serial.println("⚠ Firebase is not ready!");
  } else {
    Serial.println("✅ Firebase is ready.");
  }

  unsigned long currentMillis = millis();
  if (!isScheduledPumpRunning && (currentMillis - lastPumpTime >= pumpInterval)) {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("🔁 Scheduled Pump ON");
    pumpStartTime = currentMillis;
    isScheduledPumpRunning = true;
  }

  if (isScheduledPumpRunning && (currentMillis - pumpStartTime >= pumpDuration)) {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("✅ Scheduled Pump OFF");
    lastPumpTime = currentMillis;
    isScheduledPumpRunning = false;
  }

  handleServoCycle();
  delay(1000);
}
