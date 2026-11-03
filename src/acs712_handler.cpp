#include "acs712_handler.h"

ACS712Handler::ACS712Handler(uint8_t pin, float sensitivity) {
  _pin = pin;
  _sensitivity = sensitivity;
}

void ACS712Handler::begin() {
  pinMode(_pin, INPUT);
  
  // Auto-zero calibration saat 0 Amper
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(_pin);
    delay(2);
  }
  _adcZeroOffset = sum / 100;
}

float ACS712Handler::readCurrent() {
  int samples = 100;
  float adcTotal = 0;
  
  for (int i = 0; i < samples; i++) {
    adcTotal += analogRead(_pin);
    delayMicroseconds(500);
  }
  
  float adcAverage = adcTotal / samples;
  // Hitung selisih tegangan (mV) dari Zero Offset ADC ESP32 (3.3V / 4095.0)
  float voltageDiff_mV = (adcAverage - _adcZeroOffset) * (3300.0 / 4095.0);
  
  float current = voltageDiff_mV / _sensitivity;
  
  // Cut-off noise arus mendekati 0
  if (abs(current) < 0.08) {
    current = 0.0;
  }
  
  return current;
}