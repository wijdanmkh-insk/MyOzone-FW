#include "firebase_manager.h"
#include "config.h"
#include "shared_data.h"
#include <Preferences.h>

#ifdef USE_FIREBASE
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPClient.h>
#endif

FirebaseBareMetal cloudSystem;

FirebaseBareMetal::FirebaseBareMetal() 
  : _isBound(false), _lastUploadTime(0), _lastPollTime(0), _lastSyncTime(0) {
  memset(_resolvedUID, 0, sizeof(_resolvedUID));
}

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
  if (WiFi.status() != WL_CONNECTED) return;

  // 0. Ikat alat ke UID pemilik (Retry tiap 5 detik jika belum terikat)
  if (!_isBound) {
    if (currentMillis - _lastSyncTime >= 5000 || _lastSyncTime == 0) {
      _lastSyncTime = currentMillis;
      syncOwnership();
    }
    return; // Tunggu sampai _isBound == true sebelum upload/polling
  }

  // 1. POLLING PERINTAH & STATUS KEPEMILIKAN (Tiap 2 detik)
  if (currentMillis - _lastPollTime >= 2000) {
    _lastPollTime = currentMillis;
    pollCommands();
    checkOwnership();
  }
#endif

  // 2. UPLOAD / CETAK TELEMETRI SENSOR (Tiap 5 detik)
  if (currentMillis - _lastUploadTime >= 5000) {
    _lastUploadTime = currentMillis;
    uploadData();
  }
}

void FirebaseBareMetal::syncOwnership() {
#if !defined(USE_FIREBASE) || defined(USE_QEMU)
  _isBound = true;
  return;
#else
  if (strlen(firebaseUID) == 0) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  char url[384];

  // Langkah A: Cari UID asli berdasarkan username di /usernames/<username>.json
  snprintf(url, sizeof(url), "https://%s/usernames/%s.json?auth=%s", _host, firebaseUID, _auth);
  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    payload.trim();
    payload.replace("\"", "");

    if (payload.length() > 0 && payload != "null") {
      strncpy(_resolvedUID, payload.c_str(), sizeof(_resolvedUID) - 1);
      _resolvedUID[sizeof(_resolvedUID) - 1] = '\0';
    } else {
      Serial.printf("[Firebase] Username '%s' belum ditemukan di /usernames!\n", firebaseUID);
      http.end();
      return;
    }
  } else {
    Serial.printf("[Firebase] Gagal lookup username: HTTP %d\n", httpCode);
    http.end();
    return;
  }
  http.end();

  // Langkah B: Tulis pemilik_uid & info awal ke /devices/<MAC>.json
  char devPayload[256];
  snprintf(devPayload, sizeof(devPayload),
    "{\"pemilik_uid\":\"%s\",\"info\":{\"firmware\":\"MyOzone-FW-v1.0\",\"status\":\"Online\",\"terakhir_aktif\":{\".sv\":\"timestamp\"}}}",
    _resolvedUID
  );

  snprintf(url, sizeof(url), "%s.json?auth=%s", _basePath, _auth);
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  int devCode = http.PATCH((uint8_t*)devPayload, strlen(devPayload));
  http.end();

  // Langkah C: Daftarkan MAC ke /users/<UID>/daftar_alat.json
  char userPayload[64];
  snprintf(userPayload, sizeof(userPayload), "{\"%s\":true}", macAddress);

  snprintf(url, sizeof(url), "https://%s/users/%s/daftar_alat.json?auth=%s", _host, _resolvedUID, _auth);
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  int userCode = http.PATCH((uint8_t*)userPayload, strlen(userPayload));
  http.end();

  if (devCode == HTTP_CODE_OK && userCode == HTTP_CODE_OK) {
    _isBound = true;
    Serial.printf("[Firebase] Alat terikat ke UID: %s (Username: %s)\n", _resolvedUID, firebaseUID);
  }
#endif
}

void FirebaseBareMetal::checkOwnership() {
#if !defined(USE_FIREBASE) || defined(USE_QEMU)
  return;
#else
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  char url[384];
  snprintf(url, sizeof(url), "%s/pemilik_uid.json?auth=%s", _basePath, _auth);

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String ownerVal = http.getString();
    ownerVal.trim();
    ownerVal.replace("\"", "");

    // Jika pemilik_uid dikosongkan ("") atau dihapus (null) dari Web App -> UNBIND!
    if (ownerVal.length() == 0 || ownerVal == "null") {
      Serial.println("[SYSTEM] Kepemilikan dihapus dari Web! Resetting username & masuk mode AP...");

      digitalWrite(PIN_RELAY_OZON, LOW);
      sensorData.ozon_status = false;

      Preferences prefs;
      prefs.begin("proveggies", false);
      prefs.remove("uid");
      prefs.end();

      http.end();
      vTaskDelay(pdMS_TO_TICKS(500));
      ESP.restart();
    }
  }

  http.end();
#endif
}

void FirebaseBareMetal::uploadData() {
  char jsonPayload[640];
  snprintf(jsonPayload, sizeof(jsonPayload),
    "{\"sensors\":{"
      "\"ozon\":{\"suhu\":%.2f,\"tekanan\":%.2f,\"status_mesin\":%s},"
      "\"inverter\":{\"tegangan\":%.2f,\"daya\":%.2f},"
      "\"baterai\":{\"tegangan\":%.2f,\"arus\":%.2f},"
      "\"kondisi\":{\"suhu\":%.2f,\"kelembapan\":%.2f}"
    "},"
    "\"info\":{\"status\":\"Online\",\"terakhir_aktif\":{\".sv\":\"timestamp\"}}}",
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
  Serial.printf("[QEMU MOCK TX] URL: %s.json | Payload: %s\n", _basePath, jsonPayload);
  return;

#else
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  char url[384];
  snprintf(url, sizeof(url), "%s.json?auth=%s", _basePath, _auth);

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.PATCH((uint8_t*)jsonPayload, strlen(jsonPayload));
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("[Firebase] Telemetry & Timestamp Synced (200 OK)");
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
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  char url[384];
  snprintf(url, sizeof(url), "%s/commands/ozon_power.json?auth=%s", _basePath, _auth);

  http.begin(client, url);
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