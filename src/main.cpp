#include <Arduino.h>
#include "config.h"
#include "shared_data.h"
#include "network_manager.h"
#include "sensor_manager.h"
#include "firebase_manager.h"

// Inisialisasi Memori Global (Sesuai MISRA)
ProveggiesData sensorData = {0, 0, 0, 0, 0, 0, 0, 0, false};
char firebaseUID[64] = {0};
char macAddress[20] = {0};

void TaskCore1_Sensors(void *pvParameters) {
  initSensors();
  for (;;) {
    pollSensorsAsync();
  }
}

void TaskCore0_Network(void *pvParameters) {
  cloudSystem.begin(FIREBASE_HOST, FIREBASE_AUTH);

  for (;;) {
    cloudSystem.handleTasks();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Wajib ada agar IDLE0 tidak kena Watchdog Reset
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELAY_OZON, OUTPUT);
  digitalWrite(PIN_RELAY_OZON, LOW);

  // 1. Inisialisasi Jaringan / Mode Offline (Jalankan sekali sebelum task)
  initCaptivePortalAndNVS();

  // 2. Operasi Pembelahan Otak (Dual-Core SMP)
  xTaskCreatePinnedToCore(TaskCore1_Sensors, "Sensors", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskCore0_Network, "Cloud", 8192, NULL, 1, NULL, 0);
}

void loop() {
  vTaskDelay(portMAX_DELAY); // RTOS mengambil alih sepenuhnya
}