# Biometric Monitor — Arduino

A compact biofeedback device built on Arduino Nano that measures heart rate, skin conductance (GSR), and ambient temperature in real time. All readings are displayed on a 16x2 LCD; a passive buzzer ticks on every detected heartbeat.

---

## How it works

The device runs in two modes, switched automatically based on whether a finger is placed on the sensor:

- **Idle** — displays ambient temperature (LM35) and current GSR value
- **Measuring** — runs the PBA heart rate algorithm on the MAX30102 IR signal, displays BPM, and beeps on each beat

Switching between modes clears only line 0 of the LCD to avoid flickering — `lcd.clear()` is never called in the main loop.

---

## Hardware

| Component | Role |
|---|---|
| Arduino Nano | Microcontroller |
| MAX30102 | Optical pulse sensor (I2C) |
| LM35DZ | Analog temperature sensor |
| GSR module | Skin conductance (voltage divider) |
| LCD 16x2 + I2C converter | Display (addr `0x27`) |
| Passive buzzer | Heartbeat audio feedback |

<img width="881" height="574" alt="image" src="https://github.com/user-attachments/assets/2435a3d8-0f03-468b-818c-7a3385aa13a8" />


Pin assignments:

```
D4  — Buzzer
A0  — GSR
A1  — LM35
SDA/SCL — MAX30102 + LCD (shared I2C bus)
```

---

## Display layout

```
Line 0:  HR: 72 BPM          (finger present)
         Temp: 24.5°C        (no finger)

Line 1:  GSR: 340  ADC       (always visible)
```

---

## Key constants

| Constant | Value | Notes |
|---|---|---|
| `IR_FINGER_THRESHOLD` | `50000` | IR level above which a finger is considered present |
| `BPM_BUF_SIZE` | `4` | Rolling average window for BPM smoothing |
| `BUZZER_PIN` | `4` | Beep: 1200 Hz, 50 ms per beat |

Temperature conversion formula (LM35, 5V reference):
```
tempC = analogRead(TEMP_PIN) * 0.48828125
      = ADC * (5.0 / 1024) * 100
```

---

## Dependencies

- [SparkFun MAX3010x library](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library) — `MAX30105.h`, `heartRate.h`
- [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)

Install via Arduino Library Manager or drop into `libraries/`.

---

## Usage

1. Wire up the components according to the pin assignments above.
2. Install dependencies.
3. Flash `biometric_monitor.ino` to the Arduino Nano.
4. Open Serial Monitor at **115200 baud** to see raw values (`IR`, `BPM`, `Temp`, `GSR`).
5. Place a fingertip lightly on the MAX30102 sensor — pressing too hard cuts off blood flow and kills the reading.
6. After 2–3 seconds the buzzer will start ticking and BPM will appear on the display.

---

## Known issues / notes

- **Finger pressure matters.** The diode amplitude is intentionally set low (`0x0A`) — if BPM never stabilizes, try adjusting finger pressure rather than increasing LED power.
- **GSR is relative.** The raw ADC value (0–1023) has no absolute unit; it's useful for tracking changes, not comparing between users.
- The schema for the LM35 assumes a 5 V Arduino. On a 3.3 V board the conversion factor needs to change to `ADC * (3.3 / 1024) * 100`.

---

## Author

Adam Bronikowski
