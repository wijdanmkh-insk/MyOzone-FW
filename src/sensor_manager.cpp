#include "sensor_manager.h"

SensorManager::SensorManager() 
  : _acs(ACS712_PIN, ACS712_SENSITIVITY), 
    _pzem(PZEM_RX_PIN, PZEM_TX_PIN) {}

void SensorManager::begin() {
  _acs.begin();
  _pzem.begin();
}

PowerData SensorManager::readAll() {
  PowerData data;

  // 1. Baca Arus DC dari modul ACS712
  data.i_dc = _acs.readCurrent();
  data.v_dc = 12.6; // Nilai simulasi / bisa ditambah pembacaan Voltage Divider DC
  data.p_dc = data.v_dc * data.i_dc;

  // 2. Baca AC dari modul PZEM-004T
  _pzem.readACData(data.v_ac, data.i_ac, data.p_ac, data.f_ac, data.pf);

  return data;
}