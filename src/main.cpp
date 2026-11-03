#include <Arduino.h>

// Menggunakan pin Analog A0 pada Arduino Mega
const int acsPin = A0; 

// Konfigurasi Kalibrasi ACS712
// Sesuaikan dengan modul: 5A (0.185), 20A (0.100), 30A (0.066) V/A
const float sensitivity = 0.185; 

// Tegangan referensi saat arus 0A pada sistem 5V
float vRef = 2.50; 

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(1); }
  
  Serial.println("Memulai pembacaan sensor ACS712 di Pin A0 (Arduino Mega)...");
}

void loop() {
  int numSamples = 150;
  float adcTotal = 0;
  
  // Mengambil banyak sampel untuk meredam noise
  for (int i = 0; i < numSamples; i++) {
    adcTotal += analogRead(acsPin);
    delay(1); 
  }
  
  float adcAverage = adcTotal / numSamples;
  
  // Konversi nilai ADC (0-1023) ke Tegangan (Volt)
  // ADC Arduino Mega adalah 10-bit dengan referensi tegangan 5.0V
  float voltage = (adcAverage / 1023.0) * 5.0;
  
  // Menghitung nilai arus (Ampere)
  float current = (voltage - vRef) / sensitivity;
  
  Serial.print("ADC Avg: "); 
  Serial.print(adcAverage, 1);
  Serial.print(" | Tegangan Sensor: "); 
  Serial.print(voltage, 3);
  Serial.print(" V | Arus: "); 
  Serial.print(current, 3);
  Serial.println(" A");
  
  delay(500);
} 