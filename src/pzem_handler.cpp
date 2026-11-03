#include "pzem_handler.h"

PZEMHandler::PZEMHandler(uint8_t rxPin, uint8_t txPin) 
  : _pzem(Serial2, rxPin, txPin) {} // Menggunakan HardwareSerial2 ESP32

void PZEMHandler::begin() {
  // PZEM-004T v3.0 otomatis memunculkan instance serial
}

void PZEMHandler::readACData(float &v_ac, float &i_ac, float &p_ac, float &f_ac, float &pf) {
  v_ac = _pzem.voltage();
  i_ac = _pzem.current();
  p_ac = _pzem.power();
  f_ac = _pzem.frequency();
  pf   = _pzem.pf();

  // Handle jika pembacaan gagal / sensor tidak terhubung
  if (isnan(v_ac)) v_ac = 0.0;
  if (isnan(i_ac)) i_ac = 0.0;
  if (isnan(p_ac)) p_ac = 0.0;
  if (isnan(f_ac)) f_ac = 0.0;
  if (isnan(pf))   pf   = 0.0;
}