#include <Arduino.h>

constexpr int ENCB2 = 36;
constexpr int ENCB1 = 39;
constexpr int ENCA2 = 32;
constexpr int ENCA1 = 33;

// Jumlah hitungan per putaran untuk metode pembacaan ini.
constexpr float COUNT_PER_REV_A = 1000.0f;
constexpr float COUNT_PER_REV_B = 600.0f;

volatile int32_t posisiA = 0;
volatile int32_t posisiB = 0;

float cm_deg = 27.5/981.84;

void ARDUINO_ISR_ATTR bacaEncoderA() {
  if (digitalRead(ENCA2) == HIGH) {
    posisiA++;
  } else {
    posisiA--;
  }
}

void ARDUINO_ISR_ATTR bacaEncoderB() {
  if (digitalRead(ENCB2) == HIGH) {
    posisiB++;
  } else {
    posisiB--;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ENCA1, INPUT);
  pinMode(ENCA2, INPUT);
  pinMode(ENCB1, INPUT);
  pinMode(ENCB2, INPUT);

  attachInterrupt(ENCA1, bacaEncoderA, RISING);
  attachInterrupt(ENCB1, bacaEncoderB, RISING);
}

void loop() {
  noInterrupts();
  int32_t a = posisiA;
  int32_t b = posisiB;
  interrupts();

  float sudutA = a * 360.0f / COUNT_PER_REV_A;
  float sudutB = b * 360.0f / COUNT_PER_REV_B;
  float jarak = sudutB * cm_deg;

  Serial.printf("Encoder A: %.2f deg | Encoder B: %.2f deg | Jarak cart: %.2f cm\n",
                sudutA, sudutB, jarak);

  delay(100);
}