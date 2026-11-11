#include <Arduino.h>
#include <stdlib.h>

constexpr int ENA = 25;
constexpr int IN1 = 26;
constexpr int IN2 = 27;

constexpr unsigned long DURASI_MOTOR = 1000; // 1 detik

bool motorAktif = false;
unsigned long waktuMulai = 0;

String barisSerial = "";

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, LOW);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Arah putaran tetap.
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  analogWrite(ENA, 0);

  Serial.println("Masukkan PWM 0.00 sampai 1.00, lalu Enter.");
  Serial.println("Motor akan menyala selama 1 detik.");
}

void loop() {
  // Matikan motor setelah 1 detik.
  if (motorAktif && millis() - waktuMulai >= DURASI_MOTOR) {
    analogWrite(ENA, 0);
    motorAktif = false;

    Serial.println("Selesai 1 detik. Motor dimatikan.");
  }

  // Baca serial per karakter, tanpa menunggu satu baris.
  if (Serial.available() > 0) {
    char karakter = Serial.read();

    if (karakter == '\r') {
      return;
    }

    if (karakter != '\n') {
      barisSerial += karakter;
      return;
    }

    String input = barisSerial;
    barisSerial = "";
    input.trim();

    if (input.length() == 0) {
      return;
    }

    char *akhir;
    float pwm = strtof(input.c_str(), &akhir);

    if (akhir == input.c_str() ||
        *akhir != '\0' ||
        !(pwm >= 0.0f && pwm <= 1.0f)) {
      Serial.println("Input salah. Gunakan angka 0.00 sampai 1.00.");
      return;
    }

    int duty = (int)(pwm * 255.0f + 0.5f);
    analogWrite(ENA, duty);

    if (duty > 0) {
      waktuMulai = millis();
      motorAktif = true;

      Serial.printf("PWM %.2f: motor aktif selama 1 detik.\n", pwm);
    } else {
      motorAktif = false;
      Serial.println("Motor dimatikan.");
    }
  }
}