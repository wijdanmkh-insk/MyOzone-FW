#ifndef PZEM_HANDLER_H
#define PZEM_HANDLER_H

#include <Arduino.h>
#include <PZEM004Tv30.h>

class PZEMHandler {
private:
  PZEM004Tv30 _pzem;

public:
  PZEMHandler(uint8_t rxPin, uint8_t txPin);
  void begin();
  void readACData(float &v_ac, float &i_ac, float &p_ac, float &f_ac, float &pf);
};

#endif