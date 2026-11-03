#include "firebase_handler.h"

FirebaseBareMetal::FirebaseBareMetal(const char* host, const char* auth, const char* path) {
  _host = String(host);
  _auth = String(auth);
  _path = String(path);
}

void FirebaseBareMetal::begin() {
  // Setup dasar jika diperlukan
}

bool FirebaseBareMetal::sendData(const PowerData& data) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[FirebaseLib] Error: Wi-Fi terputus!");
    return false;
  }

  HTTPClient http;
  
  // URL endpoint REST API Firebase
  String url = _host + _path + "?auth=" + _auth;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Format String JSON Manual (Zero-Overhead Memory)
  String jsonPayload = "{";
  jsonPayload += "\"v_dc\":" + String(data.v_dc, 2) + ",";
  jsonPayload += "\"i_dc\":" + String(data.i_dc, 2) + ",";
  jsonPayload += "\"p_dc\":" + String(data.p_dc, 2) + ",";
  jsonPayload += "\"v_ac\":" + String(data.v_ac, 2) + ",";
  jsonPayload += "\"i_ac\":" + String(data.i_ac, 2) + ",";
  jsonPayload += "\"p_ac\":" + String(data.p_ac, 2);
  jsonPayload += "}";

  // Tembak REST API PATCH agar update realtime tanpa menghapus field lain
  int httpResponseCode = http.PATCH(jsonPayload);
  http.end();

  if (httpResponseCode > 0) {
    Serial.printf("[FirebaseLib] Success Update Data! Code: %d\n", httpResponseCode);
    return true;
  } else {
    Serial.printf("[FirebaseLib] Error: %s\n", http.errorToString(httpResponseCode).c_str());
    return false;
  }
}