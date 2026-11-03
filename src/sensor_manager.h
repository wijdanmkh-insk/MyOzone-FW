#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "config.h"
#include "acs712_handler.h"
#include "pzem_handler.h"

class SensorManager {
private:
  ACS712Handler _acs;
  PZEMHandler   _pzem;

public:
  SensorManager();
  void begin();
  PowerData readAll();
};

#endif