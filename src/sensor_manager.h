#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "shared_data.h"

void initSensors();
void pollSensorsAsync();

#endif