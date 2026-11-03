#ifndef FIREBASE_BARE_METAL_H
#define FIREBASE_BARE_METAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Struktur data telemetri monitoring PLTS & ATS
struct PowerData {
  float v_dc;
  float i_dc;
  float p_dc;
  float v_ac;
  float i_ac;
  float p_ac;
};

class FirebaseBareMetal {
private:
  String _host;
  String _auth;
  String _path;

public:
  FirebaseBareMetal(const char* host, const char* auth, const char* path = "/monitoring.json");
  void begin();
  bool sendData(const PowerData& data);
};

#endif