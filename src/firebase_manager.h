#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "shared_data.h"

class FirebaseBareMetal {
public:
  FirebaseBareMetal();
  void begin(const char* host, const char* auth);
  void handleTasks();

private:
  char _host[128];
  char _auth[128];
  char _basePath[256];
  char _resolvedUID[64];
  bool _isBound;

  unsigned long _lastUploadTime;
  unsigned long _lastPollTime;
  unsigned long _lastSyncTime;

  void syncOwnership();
  void checkOwnership();
  void uploadData();
  void pollCommands();
};

extern FirebaseBareMetal cloudSystem;

#endif