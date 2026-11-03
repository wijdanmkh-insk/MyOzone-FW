#ifndef ACS712_HANDLER_H
#define ACS712_HANDLER_H

#include <Arduino.h>

class ACS712Handler {
private:
  uint8_t _pin;
  float _sensitivity;
  int _adcZeroOffset;

public:
  ACS712Handler(uint8_t pin, float sensitivity = 185.0);
  void begin();
  float readCurrent();
};

#endif