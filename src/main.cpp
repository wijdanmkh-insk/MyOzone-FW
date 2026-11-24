#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "shared_data.h"
#include "network_manager.h"
#include "sensor_manager.h"
#include "firebase_manager.h"

#define PIN_WIFI_RESET  0     // Tombol BOOT bawaan ESP32 DevKit
#define RESET_HOLD_MS   4000  // Tahan 4 detik untuk reset Wi-Fi (Username tetap aman)

// Inisialisasi Memori Global (Sesuai MISRA)
ProveggiesData sensorData = {0, 0, 0, 0, 0, 0, 0, 0, false};
char firebaseUID[64] = {0};
char macAddress[20] = {0};

void checkWiFiResetButton() {
#ifdef USE_WIFI
  #if !defined(USE_QEMU) && !defined(USE_WOKWI)
    static unsigned long pressStart = 0;
    static bool isHolding = false;

    if (digitalRead(PIN_WIFI_RESET) == LOW) {
      if (!isHolding) {
        isHolding = true;
        pressStart = millis();
      } else if (millis() - pressStart >= RESET_HOLD_MS) {
        Serial.println("[SYSTEM] Tombol BOOT ditahan 4s -> Reset Wi-Fi (Username dipertahankan)...");

        // 1. Matikan relay ozon demi keamanan sebelum restart
        digitalWrite(PIN_RELAY_OZON, LOW);
        sensorData.ozon_status = false;

        // 2. Hapus kredensial Wi-Fi bawaan SDK (NVS "proveggies" key "uid" TIDAK dihapus)
        WiFi.disconnect(true, true);

        vTaskDelay(pdMS_TO_TICKS(500));
        ESP.restart(); // Restart otomatis masuk ke Portal AP dengan username terkunci
      }
    } else {
      isHolding = false;
    }
  #endif
#endif
}

void TaskCore1_Sensors(void *pvParameters) {
  initSensors();
  for (;;) {
    pollSensorsAsync();
  }
}

void TaskCore0_Network(void *pvParameters) {
  cloudSystem.begin(FIREBASE_HOST, FIREBASE_AUTH);

  for (;;) {
    checkWiFiResetButton();
    cloudSystem.handleTasks();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Wajib ada agar IDLE0 tidak kena Watchdog Reset
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELAY_OZON, OUTPUT);
  digitalWrite(PIN_RELAY_OZON, LOW);
  pinMode(PIN_WIFI_RESET, INPUT_PULLUP);

  // 1. Inisialisasi Jaringan / Mode Offline (Jalankan sekali sebelum task)
  initCaptivePortalAndNVS();

  // 2. Operasi Pembelahan Otak (Dual-Core SMP)
  xTaskCreatePinnedToCore(TaskCore1_Sensors, "Sensors", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskCore0_Network, "Cloud", 8192, NULL, 1, NULL, 0);
}

void loop() {
  vTaskDelay(portMAX_DELAY); // RTOS mengambil alih sepenuhnya
}