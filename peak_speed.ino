#include <Arduino.h>
#include <stdlib.h>
#include <math.h>

// ================= PIN ENCODER =================
constexpr int ENCB2 = 36;
constexpr int ENCB1 = 39;
constexpr int ENCA2 = 32;
constexpr int ENCA1 = 33;

constexpr float COUNT_PER_REV_A = 1000.0f;
constexpr float COUNT_PER_REV_B = 600.0f;

// Konversi sesuai kalibrasimu.
constexpr float CM_PER_DEG = 27.5f / 981.84f;
constexpr float CM_PER_COUNT_B =
    (360.0f / COUNT_PER_REV_B) * CM_PER_DEG;

volatile int32_t posisiA = 0;
volatile int32_t posisiB = 0;

// ================= PIN MOTOR =================
constexpr int ENA = 25;
constexpr int IN1 = 26;
constexpr int IN2 = 27;

// ================= PENGUKURAN =================
constexpr unsigned long DURASI_MOTOR = 1000;
constexpr unsigned long INTERVAL_SAMPEL = 100;

bool motorAktif = false;
unsigned long waktuMulai = 0;
unsigned long waktuSampel = 0;

int32_t posisiBTerakhir = 0;
float kecepatanMaks = 0.0f;

String barisSerial = "";

// ================= INTERRUPT =================
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

// Ambil salinan hitungan encoder.
void ambilPosisi(int32_t &a, int32_t &b) {
  noInterrupts();
  a = posisiA;
  b = posisiB;
  interrupts();
}

// ================= INPUT PWM =================
void prosesPerintah(String input) {
  input.trim();

  if (input.length() == 0) return;

  char *akhir;
  float pwm = strtof(input.c_str(), &akhir);

  if (akhir == input.c_str() ||
      *akhir != '\0' ||
      !(pwm >= 0.0f && pwm <= 1.0f)) {
    Serial.println("Input salah. Masukkan PWM 0.00 sampai 1.00.");
    return;
  }

  int duty = (int)(pwm * 255.0f + 0.5f);

  if (duty == 0) {
    analogWrite(ENA, 0);

    if (motorAktif) {
      Serial.printf("Pengukuran dihentikan. Maks: %.3f cm/s\n",
                    kecepatanMaks);
    }

    motorAktif = false;
    Serial.println("Motor dimatikan.");
    return;
  }

  // Mulai pengukuran baru; posisi encoder tidak direset.
  int32_t a, b;
  ambilPosisi(a, b);

  posisiBTerakhir = b;
  kecepatanMaks = 0.0f;

  waktuMulai = millis();
  waktuSampel = waktuMulai;
  motorAktif = true;

  analogWrite(ENA, duty);

  Serial.printf("\nMulai: PWM %.2f | Duty %d/255 | Durasi 1 detik\n",
                pwm, duty);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, LOW);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  pinMode(ENCA1, INPUT);
  pinMode(ENCA2, INPUT);
  pinMode(ENCB1, INPUT);
  pinMode(ENCB2, INPUT);

  attachInterrupt(ENCA1, bacaEncoderA, RISING);
  attachInterrupt(ENCB1, bacaEncoderB, RISING);

  Serial.println("Masukkan PWM 0.00 sampai 1.00, lalu Enter.");
  Serial.println("Input 0.00 untuk mematikan motor.");
}

// ================= LOOP =================
void loop() {
  unsigned long sekarang = millis();

  if (motorAktif) {
    bool selesai = sekarang - waktuMulai >= DURASI_MOTOR;

    // Putus daya sebelum mencetak hasil akhir.
    if (selesai) {
      analogWrite(ENA, 0);
    }

    unsigned long dt = sekarang - waktuSampel;

    if (dt >= INTERVAL_SAMPEL) {
      int32_t a, b;
      ambilPosisi(a, b);

      float sudutA = a * 360.0f / COUNT_PER_REV_A;
      float jarakCart = b * CM_PER_COUNT_B;

      float perubahanJarak =
          (b - posisiBTerakhir) * CM_PER_COUNT_B;

      // Nilai mutlak: besar kecepatan tanpa melihat arah.
      float kecepatan =
          fabsf(perubahanJarak) * 1000.0f / dt;

      if (kecepatan > kecepatanMaks) {
        kecepatanMaks = kecepatan;
      }

      posisiBTerakhir = b;
      waktuSampel = sekarang;

      Serial.printf(
          "A: %.2f deg | Cart: %.2f cm | Speed: %.3f cm/s"
          " | Maks: %.3f cm/s\n",
          sudutA, jarakCart, kecepatan, kecepatanMaks
      );
    }

    if (selesai) {
      motorAktif = false;

      Serial.printf(
          "SELESAI — kecepatan maksimum: %.3f cm/s\n\n",
          kecepatanMaks
      );
    }
  }

  // Baca serial tanpa delay atau menunggu satu baris.
  if (Serial.available() > 0) {
    char karakter = Serial.read();

    if (karakter == '\n') {
      prosesPerintah(barisSerial);
      barisSerial = "";
    } else if (karakter != '\r') {
      barisSerial += karakter;
    }
  }
}