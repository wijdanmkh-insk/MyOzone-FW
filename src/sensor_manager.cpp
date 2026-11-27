#include "sensor_manager.h"
#include "config.h"
#include "shared_data.h"

#ifndef USE_QEMU
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

  #ifdef USE_PRESSURE
    // Fallback pin jika belum didefinisikan di config.h
    #ifndef PIN_PRESSURE_OUT
      #define PIN_PRESSURE_OUT PIN_PRESSURE // Default GPIO 34
    #endif
    #ifndef PIN_PRESSURE_SCK
      #define PIN_PRESSURE_SCK 25           // Default GPIO 25
    #endif

    static portMUX_TYPE hx710Mux = portMUX_INITIALIZER_UNLOCKED;

    static long readHX710B() {
      unsigned long timeout = millis();
      while (digitalRead(PIN_PRESSURE_OUT) == HIGH) {
        if (millis() - timeout > 100) {
          return 0; // Timeout jika sensor belum siap / kabel terlepas
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
      }

      long count = 0;

      // Kunci interrupt sesaat (~50us) agar pulsa SCK HIGH tidak melebihi 60us (mode sleep HX710B)
      portENTER_CRITICAL(&hx710Mux);
      for (uint8_t i = 0; i < 24; i++) {
        digitalWrite(PIN_PRESSURE_SCK, HIGH);
        delayMicroseconds(1);
        count = count << 1;
        digitalWrite(PIN_PRESSURE_SCK, LOW);
        delayMicroseconds(1);
        if (digitalRead(PIN_PRESSURE_OUT) == HIGH) {
          count++;
        }
      }

      // Pulsa clock ke-25 untuk mengembalikan pin OUT ke HIGH (Next conversion 10Hz)
      digitalWrite(PIN_PRESSURE_SCK, HIGH);
      delayMicroseconds(1);
      digitalWrite(PIN_PRESSURE_SCK, LOW);
      delayMicroseconds(1);
      portEXIT_CRITICAL(&hx710Mux);

      // Konversi 24-bit two's complement ke signed 32-bit
      if (count & 0x800000) {
        count |= 0xFF000000;
      }

      return count;
    }
  #endif
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
    pinMode(PIN_PRESSURE_OUT, INPUT);
    pinMode(PIN_PRESSURE_SCK, OUTPUT);
    digitalWrite(PIN_PRESSURE_SCK, LOW);
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
    long raw_hx = readHX710B();
    if (raw_hx != 0) {
      // TODO: Sesuaikan offset nol (tare) & faktor skala setelah uji tekanan fisik
      sensorData.ozon_pressure = (float)raw_hx;
    }
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