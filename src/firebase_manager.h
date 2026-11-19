#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Arduino.h>
#include "shared_data.h"
#include "config.h"

class FirebaseBareMetal {
private:
  char _host[128];
  char _auth[128];
  char _basePath[192];

  unsigned long _lastUploadTime;
  unsigned long _lastPollTime;

  void uploadData();
  void pollCommands();

public:
  FirebaseBareMetal();
  void begin(const char* host, const char* auth);
  void handleTasks();
};

extern FirebaseBareMetal cloudSystem;

#endif