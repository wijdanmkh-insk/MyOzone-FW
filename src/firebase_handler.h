#ifndef FIREBASE_HANDLER_H
#define FIREBASE_HANDLER_H

#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>

class FirebaseBareMetal {
private:
  String _host;
  String _auth;
  String _path;

public:
  FirebaseBareMetal(const char* host, const char* auth, const char* path = "/monitoring.json");
  bool sendData(const PowerData& data);
};

#endif