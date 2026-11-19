#include "firebase_manager.h"

#ifdef USE_FIREBASE
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

FirebaseBareMetal cloudSystem;

FirebaseBareMetal::FirebaseBareMetal() : _lastUploadTime(0), _lastPollTime(0) {}

void FirebaseBareMetal::begin(const char* host, const char* auth) {
  strncpy(_host, host, sizeof(_host) - 1);
  _host[sizeof(_host) - 1] = '\0';

  strncpy(_auth, auth, sizeof(_auth) - 1);
  _auth[sizeof(_auth) - 1] = '\0';

  snprintf(_basePath, sizeof(_basePath), "https://%s/devices/%s", _host, macAddress);

#ifndef USE_FIREBASE
  Serial.println("[FIREBASE] USE_FIREBASE OFF -> Telemetri hanya dicetak ke Serial.");
#endif
}

void FirebaseBareMetal::handleTasks() {
  unsigned long currentMillis = millis();

#ifdef USE_FIREBASE
  // 1. POLLING PERINTAH REAL-TIME (Cek Relay tiap 2 detik)
  if (currentMillis - _lastPollTime >= 2000) {
    _lastPollTime = currentMillis;
    pollCommands();
  }
#endif

  // 2. UPLOAD / CETAK TELEMETRI SENSOR (Tiap 5 detik)
  if (currentMillis - _lastUploadTime >= 5000) {
    _lastUploadTime = currentMillis;
    uploadData();
  }
}

void FirebaseBareMetal::uploadData() {
  // Rakit JSON statis di RAM (tetap jalan baik mode Online maupun Offline Debug)
  char jsonPayload[512];
  snprintf(jsonPayload, sizeof(jsonPayload),
    "{\"ozon\":{\"suhu\":%.2f,\"tekanan\":%.2f,\"status_mesin\":%s},"
    "\"inverter\":{\"tegangan\":%.2f,\"daya\":%.2f},"
    "\"baterai\":{\"tegangan\":%.2f,\"arus\":%.2f},"
    "\"kondisi\":{\"suhu\":%.2f,\"kelembapan\":%.2f}}",
    sensorData.ozon_temp,
    sensorData.ozon_pressure,
    sensorData.ozon_status ? "true" : "false",
    sensorData.inv_v,
    sensorData.inv_p,
    sensorData.bat_v,
    sensorData.bat_i,
    sensorData.room_temp,
    sensorData.room_hum
  );

#if !defined(USE_FIREBASE)
  Serial.printf("[LOCAL TELEMETRY] %s\n", jsonPayload);
  return;

#elif defined(USE_QEMU)
  Serial.printf("[QEMU MOCK TX] URL: %s/sensors.json | Payload: %s\n", _basePath, jsonPayload);
  return;

#else
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  char url[384];
  snprintf(url, sizeof(url), "%s/sensors.json?auth=%s", _basePath, _auth);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.PATCH((uint8_t*)jsonPayload, strlen(jsonPayload));
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("[Firebase] Telemetry Synced (200 OK)");
  } else {
    Serial.printf("[Firebase] Upload Failed: HTTP %d\n", httpCode);
  }

  http.end();
#endif
}

void FirebaseBareMetal::pollCommands() {
#if !defined(USE_FIREBASE) || defined(USE_QEMU)
  return;
#else
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  char url[384];
  snprintf(url, sizeof(url), "%s/commands/ozon_power.json?auth=%s", _basePath, _auth);

  http.begin(url);
  int httpCode = http.GET();

  // MISRA SAFETY: Eksekusi relay HANYA jika HTTP 200 OK
  if (httpCode == HTTP_CODE_OK) {
    WiFiClient* stream = http.getStreamPtr();
    char resp[16] = {0};
    size_t len = stream->readBytes(resp, sizeof(resp) - 1);
    resp[len] = '\0';

    bool command = (strstr(resp, "true") != NULL);
    digitalWrite(PIN_RELAY_OZON, command ? HIGH : LOW);
    sensorData.ozon_status = command;
  } else {
    Serial.printf("[Firebase] Poll Ignored (HTTP %d) - State Relay Dikunci.\n", httpCode);
  }

  http.end();
#endif
}