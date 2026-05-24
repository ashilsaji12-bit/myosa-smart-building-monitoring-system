#define BLYNK_TEMPLATE_ID   "ID"
#define BLYNK_TEMPLATE_NAME "Smart Building"
#define BLYNK_AUTH_TOKEN    "TOKEN"
char ssid[] = "WIFI NAME";
char pass[] = "PASSWORD";

// ================= LIBRARIES =================

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>
#include <math.h>

// ================= OLED =================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT,
  &Wire, OLED_RESET
);

// ================= APDS9960 DIRECT I2C =================

#define APDS9960_ADDR    0x39
#define APDS9960_ENABLE  0x80
#define APDS9960_ATIME   0x81
#define APDS9960_CONTROL 0x8F
#define APDS9960_CDATAL  0x94
#define APDS9960_CDATAH  0x95

bool apdsReady = false;

void apdsWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(APDS9960_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t apdsRead(uint8_t reg) {
  Wire.beginTransmission(APDS9960_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(APDS9960_ADDR, 1, true);
  return Wire.read();
}

bool apdsInit() {
  Wire.beginTransmission(APDS9960_ADDR);
  if (Wire.endTransmission() != 0) return false;
  apdsWrite(APDS9960_ENABLE, 0x03);
  apdsWrite(APDS9960_ATIME, 0xDB);
  apdsWrite(APDS9960_CONTROL, 0x00);
  delay(120);
  return true;
}

uint16_t apdsReadLight() {
  uint8_t lo = apdsRead(APDS9960_CDATAL);
  uint8_t hi = apdsRead(APDS9960_CDATAH);
  return (uint16_t)(hi << 8 | lo);
}

// ================= PINS =================

#define BUZZER_PIN 4
#define LED_PIN    2

// ================= VARIABLES =================

float temperature = 0;
float pressure    = 0;
float altitude    = 0;

float accelX = 0;
float accelY = 0;
float accelZ = 0;

float tiltAngle = 0;
float vibration = 0;

// ================= EARTHQUAKE =================
// Simple approach:
// MPU6050 flat & still = accelZ ~ 1.0g, vibration magnitude ~ 1.0
// When shaken hard, individual axes spike well above or below normal.
// We measure how far each axis deviates from resting values.
// Resting baseline is calibrated at startup.

float baseX = 0;  // Resting accelX
float baseY = 0;  // Resting accelY
float baseZ = 0;  // Resting accelZ (usually ~1.0)

float shakeVal = 0; // Current deviation from baseline

// Shake threshold — how much deviation = earthquake
// 0.5g deviation is a strong shake. Lower = more sensitive.
#define SHAKE_THRESHOLD  0.5

// How long to keep alert ON after detected (milliseconds)
#define ALERT_HOLD_MS    6000

unsigned long earthquakeAlertTime = 0;

// ================= LIGHT =================

uint16_t ambientLight     = 0;
uint16_t prevAmbientLight = 0;

// ================= ALERTS =================

bool earthquakeAlert = false;
bool tiltAlert       = false;
bool fireAlert       = false;
bool wallBreakAlert  = false;

// ================= LIMITS =================

float    TILT_LIMIT       = 10.0;
uint16_t FIRE_LIGHT_LIMIT = 100;
float    FIRE_TEMP_LIMIT  = 50.0;
uint16_t WALL_BREAK_SPIKE = 10;

// ================= BMP180 =================

Adafruit_BMP085 bmp;

// ================= TIMERS =================

BlynkTimer timer;
unsigned long lastOLED = 0;
int oledPage = 0;

// ======================================================
// BUZZER
// ======================================================

void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH); delay(150);
    digitalWrite(BUZZER_PIN, LOW);  delay(120);
  }
}

void beepFast(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH); delay(80);
    digitalWrite(BUZZER_PIN, LOW);  delay(60);
  }
}

// ======================================================
// CALIBRATE MPU6050 BASELINE
// Takes 50 readings at startup while device is still
// and averages them to get resting accel values
// ======================================================

void calibrateMPU() {

  Serial.println("Calibrating MPU6050... keep still");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Calibrating...");
  display.println("Keep device STILL");
  display.display();

  float sumX = 0, sumY = 0, sumZ = 0;
  int samples = 50;

  for (int i = 0; i < samples; i++) {
    Wire.beginTransmission(0x69);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x69, 6, true);

    int16_t ax = Wire.read() << 8 | Wire.read();
    int16_t ay = Wire.read() << 8 | Wire.read();
    int16_t az = Wire.read() << 8 | Wire.read();

    sumX += ax / 16384.0;
    sumY += ay / 16384.0;
    sumZ += az / 16384.0;

    delay(20);
  }

  baseX = sumX / samples;
  baseY = sumY / samples;
  baseZ = sumZ / samples;

  Serial.print("Baseline X:");
  Serial.print(baseX);
  Serial.print(" Y:");
  Serial.print(baseY);
  Serial.print(" Z:");
  Serial.println(baseZ);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Calibration OK!");
  display.print("Z baseline:");
  display.println(baseZ);
  display.display();
  delay(1000);
}

// ======================================================
// OLED DISPLAY
// ======================================================

void showPage() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  switch (oledPage) {

    // ================= PAGE 1 =================
    case 0:
      display.println("MYOSA SYSTEM");
      display.println("----------------");
      display.print("Temp: ");
      display.print(temperature);
      display.println(" C");
      display.print("Pressure:");
      display.print(pressure);
      display.println(" hPa");
      break;

    // ================= PAGE 2 =================
    case 1:
      display.println("TILT MONITOR");
      display.println("----------------");
      display.print("Tilt:");
      display.print(tiltAngle);
      display.println(" deg");
      display.print("Shake:");
      display.println(shakeVal, 3);
      break;

    // ================= PAGE 3 — EARTHQUAKE =================
    case 2:
      display.println("ALERT STATUS");
      display.println("----------------");

      if (earthquakeAlert) {
        // Big warning when earthquake active
        display.setTextSize(2);
        display.setCursor(0, 16);
        display.println("EARTHQUAKE");
        display.println("DETECTED!");
        display.setTextSize(1);
      } else {
        display.print("Earthquake: NO");
        display.println();
        display.print("Tilt: ");
        display.println(tiltAlert ? "YES" : "NO");
        display.print("Shake val:");
        display.println(shakeVal, 3);
      }
      break;

    // ================= PAGE 4 =================
    case 3:
      display.println("NETWORK STATUS");
      display.println("----------------");
      display.println(
        WiFi.status() == WL_CONNECTED ? "WiFi OK" : "WiFi FAIL"
      );
      display.println(
        Blynk.connected() ? "Blynk OK" : "Blynk FAIL"
      );
      break;

    // ================= PAGE 5 =================
    case 4:
      display.println("LIGHT SENSOR");
      display.println("----------------");
      display.print("Light: ");
      display.println(ambientLight);
      if (!apdsReady) display.println("[SENSOR FAIL]");
      break;

    // ================= PAGE 6 =================
    case 5:
      display.println("SAFETY ALERTS");
      display.println("----------------");
      display.print("Fire: ");
      display.println(fireAlert ? "!!! YES !!!" : "NO");
      display.print("Wall Break: ");
      display.println(wallBreakAlert ? "!!! YES !!!" : "NO");
      break;
  }

  display.display();
}

// ======================================================
// READ BMP180
// ======================================================

void readEnvironment() {
  temperature = bmp.readTemperature();
  pressure    = bmp.readPressure() / 100.0F;
  altitude    = bmp.readAltitude();

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" Pressure: ");
  Serial.println(pressure);
}

// ======================================================
// RAW MPU6050 READ
// ======================================================

void readMPU() {
  Wire.beginTransmission(0x69);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x69, 6, true);

  int16_t AcX = Wire.read() << 8 | Wire.read();
  int16_t AcY = Wire.read() << 8 | Wire.read();
  int16_t AcZ = Wire.read() << 8 | Wire.read();

  accelX = AcX / 16384.0;
  accelY = AcY / 16384.0;
  accelZ = AcZ / 16384.0;

  // Total magnitude (still ~1.0 when flat due to gravity)
  vibration = sqrt(
    accelX * accelX +
    accelY * accelY +
    accelZ * accelZ
  );

  // Deviation from calibrated baseline on each axis
  float dX = accelX - baseX;
  float dY = accelY - baseY;
  float dZ = accelZ - baseZ;

  // shakeVal = how far we moved from resting position
  shakeVal = sqrt(dX*dX + dY*dY + dZ*dZ);

  tiltAngle = atan2(
    sqrt(accelX * accelX + accelY * accelY),
    accelZ
  ) * 180.0 / PI;

  Serial.print(" Shake: ");
  Serial.print(shakeVal, 3);
  Serial.print(" Tilt: ");
  Serial.println(tiltAngle);
}

// ======================================================
// READ LIGHT SENSOR
// ======================================================

void readLight() {
  if (apdsReady) {
    prevAmbientLight = ambientLight;
    ambientLight     = apdsReadLight();
    Serial.print(" Light: ");
    Serial.println(ambientLight);
  } else {
    ambientLight = 0;
  }
}

// ======================================================
// ALERT CHECK
// ======================================================

void checkAlerts() {

  // ---- Earthquake: hold alert ON for ALERT_HOLD_MS ----
  if (earthquakeAlert) {
    if (millis() - earthquakeAlertTime > ALERT_HOLD_MS) {
      earthquakeAlert = false;
      Serial.println("Earthquake alert cleared");
    }
    // Keep showing on OLED page 3
    oledPage = 2;
  }

  tiltAlert      = false;
  fireAlert      = false;
  wallBreakAlert = false;

  // ================= EARTHQUAKE =================
  // shakeVal = deviation from resting baseline
  // When flat & still: shakeVal ~ 0.0
  // When shaken hard:  shakeVal > 0.5

  if (shakeVal > SHAKE_THRESHOLD && !earthquakeAlert) {

    earthquakeAlert     = true;
    earthquakeAlertTime = millis();

    Serial.print("!!! EARTHQUAKE !!! shakeVal=");
    Serial.println(shakeVal);

    // Force OLED to alert page immediately
    oledPage = 2;
    showPage();

    beepFast(6);

    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);

    Blynk.logEvent("earthquake_alert", "Earthquake detected!");
  }

  // ================= TILT =================

  if (tiltAngle > TILT_LIMIT) {
    tiltAlert = true;
    beep(2);
  }

  // ================= FIRE ALERT =================

  if (apdsReady &&
      ambientLight < FIRE_LIGHT_LIMIT &&
      temperature  > FIRE_TEMP_LIMIT) {
    fireAlert = true;
    Serial.println("!!! FIRE ALERT !!!");
    beepFast(6);
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH); delay(100);
      digitalWrite(LED_PIN, LOW);  delay(100);
    }
    Blynk.logEvent("fire_alert", "FIRE DETECTED!");
  }

  // ================= WALL BREAK ALERT =================

  if (apdsReady && prevAmbientLight > 0) {
    int32_t lightDiff =
      (int32_t)ambientLight - (int32_t)prevAmbientLight;
    if (lightDiff > (int32_t)WALL_BREAK_SPIKE) {
      wallBreakAlert = true;
      Serial.println("!!! WALL BREAK ALERT !!!");
      beep(3);
      beepFast(3);
      digitalWrite(LED_PIN, HIGH); delay(500);
      digitalWrite(LED_PIN, LOW);
      Blynk.logEvent("wall_break_alert", "WALL BREACH DETECTED!");
    }
  }
}

// ======================================================
// SEND TO BLYNK
// ======================================================

void sendBlynk() {
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, pressure);
  Blynk.virtualWrite(V2, tiltAngle);
  Blynk.virtualWrite(V3, vibration);
  Blynk.virtualWrite(V4, earthquakeAlert ? 1 : 0);
  Blynk.virtualWrite(V5, tiltAlert       ? 1 : 0);
  Blynk.virtualWrite(V6, ambientLight);
  Blynk.virtualWrite(V7, fireAlert       ? 1 : 0);
  Blynk.virtualWrite(V8, wallBreakAlert  ? 1 : 0);
}

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  Wire.begin(21, 22);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // ================= OLED =================
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAIL");
    while (1);
  }

  // ================= WELCOME SCREEN =================
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 5);
  display.println("Welcome to");
  display.setTextSize(2);
  display.setCursor(18, 20);
  display.println("MYOSA");
  display.setTextSize(1);
  display.setCursor(10, 45);
  display.println("Smart Building v1.0");
  display.display();
  delay(2500);

  // ================= MPU6050 =================
  Wire.beginTransmission(0x69);
  Wire.write(0x6B);
  Wire.write(0);
  if (Wire.endTransmission(true) == 0) {
    Serial.println("MPU6050 OK");
  } else {
    Serial.println("MPU6050 FAIL");
  }

  // ================= BMP180 =================
  if (bmp.begin()) {
    Serial.println("BMP180 OK");
  } else {
    Serial.println("BMP180 FAIL");
  }

  // ================= APDS9960 =================
  delay(1000);
  if (apdsInit()) {
    apdsReady = true;
    Serial.println("APDS9960 OK");
  } else {
    apdsReady = false;
    Serial.println("APDS9960 FAIL");
  }

  // ================= CALIBRATE =================
  // Must run AFTER MPU init, BEFORE WiFi
  // Keep the device completely still during this
  calibrateMPU();

  // ================= WIFI =================
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected");

  // ================= BLYNK =================
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendBlynk);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("SYSTEM READY");
  display.display();
  delay(1000);
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  Blynk.run();
  timer.run();

  readEnvironment();
  readMPU();
  readLight();
  checkAlerts();

  if (millis() - lastOLED > 3000) {
    lastOLED = millis();
    // Only auto-advance page if no earthquake active
    if (!earthquakeAlert) {
      oledPage++;
      if (oledPage > 5) oledPage = 0;
    }
    showPage();
  }

  delay(100);
}
