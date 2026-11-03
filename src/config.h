#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- PIN ACS712 (DC CURRENT) ---
#define ACS712_PIN          34    // ADC Pin ESP32
#define ACS712_SENSITIVITY  185.0 // 185 mV/A (5A), 100 mV/A (20A), 66 mV/A (30A)

// --- PIN PZEM-004T (AC MONITORING) ---
#define PZEM_RX_PIN         16    // Hubungkan ke TX PZEM
#define PZEM_TX_PIN         17    // Hubungkan ke RX PZEM

// --- KONFIGURASI FIREBASE ---
const char FIREBASE_HOST[] = "https://PROJECT_ID-default-rtdb.firebaseio.com";
const char FIREBASE_AUTH[] = "DATABASE_SECRET_ATAU_AUTH_TOKEN";

// --- STRUCT DATA MONITORING ---
struct PowerData {
  float v_dc;
  float i_dc;
  float p_dc;
  float v_ac;
  float i_ac;
  float p_ac;
  float f_ac; // Frekuensi AC (Hz)
  float pf;   // Power Factor
};

#endif