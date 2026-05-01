/*
 * SafeRoute v2 — Adaptive Emergency Exit Intelligence
 * Autodesk "Press, Power, Play!" Hackathon 2026
 * Samuel Agyapong | Grambling State University
 */

#include <LiquidCrystal.h>

#define SENSOR_A    A0
#define SENSOR_B    A1
#define SENSOR_C    A2
#define TEMP_PIN    A4
#define BUTTON_PIN  19

#define LED_A_GREEN  2
#define LED_A_RED    3
#define LED_B_GREEN  4
#define LED_B_RED    5
#define LED_C_GREEN  6
#define LED_C_RED    7
#define BUZZER       8

LiquidCrystal lcd(9, 10, 11, 12, 13, A3);

const int   SMOKE_THRESHOLD = 30;
const int   FIRE_TEMP_C     = 55;
const int   CAL_SAMPLES     = 20;
const unsigned long SILENCE_MS = 15000;

int  baseA, baseB, baseC;
bool silenced    = false;
unsigned long silenceStart = 0;
bool lastButton  = HIGH;

void setup() {
  Serial.begin(9600);

  pinMode(LED_A_GREEN, OUTPUT);
  pinMode(LED_A_RED,   OUTPUT);
  pinMode(LED_B_GREEN, OUTPUT);
  pinMode(LED_B_RED,   OUTPUT);
  pinMode(LED_C_GREEN, OUTPUT);
  pinMode(LED_C_RED,   OUTPUT);
  pinMode(BUZZER,      OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);

  allGreen();
  lcd.setCursor(0, 0);
  lcd.print("  SafeRoute v2  ");
  lcd.setCursor(0, 1);
  lcd.print(" Calibrating... ");


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  SYSTEM READY  ");
  lcd.setCursor(0, 1);
  lcd.print("  Monitoring... ");
  delay(1500);
}

void loop() {
  unsigned long now = millis();

  bool btnNow = digitalRead(BUTTON_PIN);
  if (lastButton == HIGH && btnNow == LOW) {
    silenced     = true;
    silenceStart = now;
  }
  lastButton = btnNow;

  if (silenced) {
    if (now - silenceStart >= SILENCE_MS) {
      silenced = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  SYSTEM ARMED  ");
      lcd.setCursor(0, 1);
      lcd.print("  Monitoring... ");
      delay(800);
    } else {
      int secsLeft = (SILENCE_MS - (now - silenceStart)) / 1000;
      allGreen();
      noTone(BUZZER);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" FALSE ALARM    ");
      lcd.setCursor(0, 1);
      lcd.print(" Re-arm in: ");
      lcd.print(secsLeft);
      lcd.print("s ");
      delay(500);
      return;
    }
  }

  int readA = analogRead(SENSOR_A);
  int readB = analogRead(SENSOR_B);
  int readC = analogRead(SENSOR_C);

  // Print live values to Serial Monitor
  Serial.print("A:"); Serial.print(readA);
  Serial.print(" B:"); Serial.print(readB);
  Serial.print(" C:"); Serial.print(readC);
  Serial.print(" | Base A:"); Serial.print(baseA);
  Serial.print(" B:"); Serial.print(baseB);
  Serial.print(" C:"); Serial.println(baseC);

  bool dangerA = readA < 200;
  bool dangerB = readB < 200;
  bool dangerC = readC < 200;

  int dangerCount = (int)dangerA + (int)dangerB + (int)dangerC;

  float tempC = readTempC();
  bool thermalAlert = (tempC >= FIRE_TEMP_C);

  if (thermalAlert) {
    setLED(LED_A_GREEN, LED_A_RED, true);
    setLED(LED_B_GREEN, LED_B_RED, true);
    setLED(LED_C_GREEN, LED_C_RED, true);
  } else {
    setLED(LED_A_GREEN, LED_A_RED, dangerA);
    setLED(LED_B_GREEN, LED_B_RED, dangerB);
    setLED(LED_C_GREEN, LED_C_RED, dangerC);
  }

  updateLCD(dangerA, dangerB, dangerC, dangerCount, thermalAlert, tempC);

  if (thermalAlert) {
    tone(BUZZER, 1800, 80); delay(120);
  } else if (dangerCount == 3) {
    tone(BUZZER, 1500, 100); delay(200);
  } else if (dangerCount == 2) {
    tone(BUZZER, 1200, 100); delay(200);
    tone(BUZZER, 1200, 100); delay(400);
  } else if (dangerCount == 1) {
    tone(BUZZER, 880, 150); delay(850);
  } else {
    noTone(BUZZER); delay(200);
  }
}

float readTempC() {
  int   raw     = analogRead(TEMP_PIN);
  float voltage = raw * (5.0 / 1023.0);
  return (voltage - 0.5) * 100.0;
}

void setLED(int greenPin, int redPin, bool danger) {
  digitalWrite(greenPin, danger ? LOW  : HIGH);
  digitalWrite(redPin,   danger ? HIGH : LOW);
}

void allGreen() {
  digitalWrite(LED_A_GREEN, HIGH); digitalWrite(LED_A_RED, LOW);
  digitalWrite(LED_B_GREEN, HIGH); digitalWrite(LED_B_RED, LOW);
  digitalWrite(LED_C_GREEN, HIGH); digitalWrite(LED_C_RED, LOW);
}

void updateLCD(bool dA, bool dB, bool dC, int count, bool thermal, float temp) {
  lcd.clear();

  if (thermal) {
    lcd.setCursor(0, 0); lcd.print("!! FIRE ALERT !!");
    lcd.setCursor(0, 1); lcd.print("Temp:"); lcd.print((int)temp);
    lcd.print((char)223); lcd.print("C  RUN!");
    return;
  }

  if (count == 0) {
    lcd.setCursor(0, 0); lcd.print("   ALL CLEAR    ");
    lcd.setCursor(0, 1); lcd.print("Temp:"); lcd.print((int)temp);
    lcd.print((char)223); lcd.print("C  OK");
    return;
  }

  if (count == 3) {
    lcd.setCursor(0, 0); lcd.print("ALL EXITS BLOCKED");
    lcd.setCursor(0, 1); lcd.print(" >> CALL HELP << ");
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("GO TO EXIT: ");
  if (!dA) lcd.print("A ");
  if (!dB) lcd.print("B ");
  if (!dC) lcd.print("C ");

  lcd.setCursor(0, 1);
  lcd.print("BLOCKED: ");
  if (dA) lcd.print("A ");
  if (dB) lcd.print("B ");
  if (dC) lcd.print("C ");
}
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
