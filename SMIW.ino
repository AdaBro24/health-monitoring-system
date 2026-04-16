#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"
#include "heartRate.h"

#define BUZZER_PIN 4
#define GSR_PIN    A0
#define TEMP_PIN   A1

#define IR_FINGER_THRESHOLD 50000UL

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// rolling average for BPM smoothing
const byte BPM_BUF_SIZE = 4;
byte bpmBuffer[BPM_BUF_SIZE];
byte bpmIndex = 0;
long lastBeatMs = 0;
float bpm = 0;
int bpmAvg = 0;

int gsrValue = 0;
float tempC = 0.0;
unsigned long lastTempMs = 0;

bool fingerWasPresent = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Starting...");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    lcd.setCursor(0, 1);
    lcd.print("MAX30105 error!");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  tone(BUZZER_PIN, 1000, 200);
  delay(500);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("GSR:      ADC");
}

void loop() {
  long irValue = particleSensor.getIR();
  bool fingerPresent = (irValue > IR_FINGER_THRESHOLD);

  // GSR: average a few samples to reduce noise
  long gsrSum = 0;
  for (int i = 0; i < 5; i++) {
    gsrSum += analogRead(GSR_PIN);
    delay(2);
  }
  gsrValue = gsrSum / 5;

  // LM35: 10mV/°C, Vref = 5V -> factor 0.48828
  if (millis() - lastTempMs > 200) {
    tempC = analogRead(TEMP_PIN) * 0.48828125;
    lastTempMs = millis();
  }

  // clear line 0 when switching modes
  if (fingerPresent != fingerWasPresent) {
    lcd.setCursor(0, 0);
    lcd.print("                ");
    fingerWasPresent = fingerPresent;
  }

  if (fingerPresent) {
    if (checkForBeat(irValue)) {
      tone(BUZZER_PIN, 1200, 50);

      long delta = millis() - lastBeatMs;
      lastBeatMs = millis();
      bpm = 60 / (delta / 1000.0);

      if (bpm > 20 && bpm < 255) {
        bpmBuffer[bpmIndex++] = (byte)bpm;
        bpmIndex %= BPM_BUF_SIZE;

        bpmAvg = 0;
        for (byte i = 0; i < BPM_BUF_SIZE; i++) bpmAvg += bpmBuffer[i];
        bpmAvg /= BPM_BUF_SIZE;
      }
    }

    lcd.setCursor(0, 0);
    lcd.print("HR: ");
    lcd.print(bpmAvg > 0 ? bpmAvg : 0);
    lcd.print(" BPM    ");

  } else {
    bpmAvg = 0;
    noTone(BUZZER_PIN);

    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(tempC, 1);
    lcd.print((char)223); // degree symbol
    lcd.print("C   ");
  }

  lcd.setCursor(5, 1);
  lcd.print(gsrValue);
  lcd.print("  ");

  Serial.print("IR=");    Serial.print(irValue);
  Serial.print(" BPM=");  Serial.print(bpmAvg);
  Serial.print(" Temp="); Serial.print(tempC);
  Serial.print(" GSR=");  Serial.println(gsrValue);
}
