#ifndef SHARED_DATA_H
#define SHARED_DATA_H
#include <Arduino.h>

// MISRA Rule: Struct untuk Inter-Process Communication (IPC)
// Harus menggunakan 'volatile' agar Dual-Core tidak saling menipu data dari Cache
struct ProveggiesData {
  volatile float ozon_temp;
  volatile float ozon_pressure;
  volatile float bat_v;
  volatile float bat_i;
  volatile float inv_v;
  volatile float inv_p;
  volatile float room_temp;
  volatile float room_hum;
  volatile bool ozon_status;
};

extern ProveggiesData sensorData;
extern char firebaseUID[64]; // MISRA: Static Array, bukan String!
extern char macAddress[20];

#endif