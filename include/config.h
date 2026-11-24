#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. TOGGLE JARINGAN & CLOUD (COMPILE-TIME)
// ==========================================
// Beri komentar (//) untuk mematikan modul secara fisik dari memori
#define USE_WIFI        // Aktifkan Modul WiFi & Captive Portal
//#define USE_FIREBASE    // Aktifkan Uplink/Downlink Firebase REST API

// ==========================================
// 2. TOGGLE SIMULATOR (AKTIFKAN SALAH SATU SAJA)
// ==========================================
// #define USE_WOKWI    // Bypass Captive Portal -> konek ke Wokwi-GUEST
// #define USE_QEMU     // Bypass radio WiFi -> cetak payload JSON di QEMU

// ==========================================
// 3. TOGGLE SENSOR HARDWARE (COMPILE-TIME)
// ==========================================
//#define USE_PZEM        // Sensor AC (Modbus UART2)
//#define USE_INA226      // Sensor Baterai DC (I2C)
//#define USE_DS18B20     // Sensor Suhu Ozon (1-Wire)
//#define USE_PRESSURE    // Sensor Tekanan Analog (ADC1)
//#define USE_DHT         // Sensor Suhu & Kelembapan Ruangan

// Pengaman MISRA Preprocessor: Cegah Firebase aktif tanpa jalur jaringan
#if defined(USE_FIREBASE) && !defined(USE_WIFI) && !defined(USE_QEMU)
  #error "CONFIG ERROR: USE_FIREBASE membutuhkan USE_WIFI aktif! Nyalakan #define USE_WIFI atau matikan #define USE_FIREBASE."
#endif

// ==========================================
// 4. PIN MAPPING
// ==========================================
#define PIN_RELAY_OZON  23
#define PIN_PRESSURE    34  // ADC1
#define PIN_DS18B20     4
#define PIN_DHT         5
#define PZEM_RX_PIN     16
#define PZEM_TX_PIN     17
#define DHTTYPE         DHT11 // atau DHT22

// ==========================================
// 5. KREDENSIAL FIREBASE
// ==========================================
#define FIREBASE_HOST   "https://pengmas-tel-u-26-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH   "AIzaSyCA8IlBAuMTx7WJU3liiStw0xJxBTRheCs"

#endif