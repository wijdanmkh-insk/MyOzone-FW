#include "sensor_manager.h"
#include "config.h"
#include "shared_data.h"

#ifndef USE_QEMU
  #include "esp_adc_cal.h"

  #ifdef USE_PZEM
    #include <PZEM004Tv30.h>
    PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);
  #endif

  #ifdef USE_DS18B20
    #include <OneWire.h>
    #include <DallasTemperature.h>
    OneWire oneWire(PIN_DS18B20);
    DallasTemperature ds18b20(&oneWire);
  #endif

  #ifdef USE_INA226
    #include <Wire.h>
    #include <INA226.h>
    INA226 ina(0x40);
  #endif

  #ifdef USE_DHT
    #include <DHT.h>
    DHT dht(PIN_DHT, DHTTYPE);
  #endif

  static esp_adc_cal_characteristics_t adc_chars;
#endif

void initSensors() {
#ifdef USE_QEMU
  Serial.println("[QEMU] Mode Simulasi Sensor Aktif (Bypass Bit-Banging GPIO).");
  return;
#else
  #ifdef USE_DS18B20
    ds18b20.begin();
    ds18b20.setWaitForConversion(false);
  #endif

  #ifdef USE_INA226
    Wire.begin();
    ina.begin();
    ina.setAverage(INA226_64_SAMPLES);
    ina.setMaxCurrentShunt(0.8f, 0.1f);
  #endif

  #ifdef USE_DHT
    dht.begin();
  #endif

  #ifdef USE_PRESSURE
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    pinMode(PIN_PRESSURE, INPUT);
  #endif
#endif
}

void pollSensorsAsync() {
#ifdef USE_QEMU
  // Suntikkan data dummy realistis untuk memvalidasi pipeline Dual-Core -> JSON
  sensorData.ozon_temp     = 26.80f;
  sensorData.ozon_pressure = 1.45f;
  sensorData.inv_v         = 221.40f;
  sensorData.inv_p         = 185.50f;
  sensorData.bat_v         = 12.75f;
  sensorData.bat_i         = 14.50f;
  sensorData.room_temp     = 29.20f;
  sensorData.room_hum      = 64.00f;

  vTaskDelay(750 / portTICK_PERIOD_MS);
  return;
#else
  #ifdef USE_DS18B20
    ds18b20.requestTemperatures();
  #endif

  #ifdef USE_PZEM
    float v = pzem.voltage();
    if (!isnan(v)) sensorData.inv_v = v;

    float p = pzem.power();
    if (!isnan(p)) sensorData.inv_p = p;
  #endif

  #ifdef USE_INA226
    sensorData.bat_v = ina.getBusVoltage();
    sensorData.bat_i = ina.getCurrent();
  #endif

  #ifdef USE_DHT
    float h = dht.readHumidity();
    if (!isnan(h)) sensorData.room_hum = h;

    float t = dht.readTemperature();
    if (!isnan(t)) sensorData.room_temp = t;
  #endif

  #ifdef USE_PRESSURE
    uint32_t raw_adc = analogRead(PIN_PRESSURE);
    uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(raw_adc, &adc_chars);
    sensorData.ozon_pressure = voltage_mv / 1000.0f;
  #endif

  vTaskDelay(750 / portTICK_PERIOD_MS);

  #ifdef USE_DS18B20
    float tempOzon = ds18b20.getTempCByIndex(0);
    if (tempOzon > -55.0f && tempOzon < 125.0f) {
      sensorData.ozon_temp = tempOzon;
    }
  #endif
#endif
}