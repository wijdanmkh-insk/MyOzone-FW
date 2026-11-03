#include <Arduino.h>
#include "config.h"
#include "wifi_handler.h"
#include "sensor_manager.h"
#include "firebase_handler.h"

CustomWiFiManager wifiManager;
SensorManager     sensorManager;
FirebaseBareMetal firebase(FIREBASE_HOST, FIREBASE_AUTH, "/monitoring.json");

QueueHandle_t sensorQueue;

// TASK 1: BACA SENSOR (Core 1)
void TaskReadSensors(void * pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(1000);

  for (;;) {
    PowerData data = sensorManager.readAll();
    xQueueOverwrite(sensorQueue, &data);
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// TASK 2: KIRIM FIREBASE (Core 0)
void TaskSendFirebase(void * pvParameters) {
  PowerData receivedData;

  for (;;) {
    if (xQueueReceive(sensorQueue, &receivedData, portMAX_DELAY) == pdPASS) {
      firebase.sendData(receivedData);
    }
  }
}

void setup() {
  Serial.begin(115200);

  wifiManager.init();
  sensorManager.begin();

  sensorQueue = xQueueCreate(1, sizeof(PowerData));

  if (sensorQueue != NULL) {
    xTaskCreatePinnedToCore(TaskReadSensors, "TaskReadSensors", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(TaskSendFirebase, "TaskSendFirebase", 8192, NULL, 1, NULL, 0);
  }
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}