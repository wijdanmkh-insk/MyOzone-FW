#include "network_manager.h"
#include "config.h"
#include "shared_data.h"
#include <Preferences.h>

#ifdef USE_WIFI
  #include <WiFi.h>
  #if !defined(USE_WOKWI) && !defined(USE_QEMU)
    #include <WiFiManager.h>
  #endif
#endif

static Preferences preferences;

void initCaptivePortalAndNVS() {
#ifndef USE_WIFI
  // MODE OFFLINE: Lewati radio WiFi & Captive Portal sepenuhnya
  Serial.println("[NETWORK] USE_WIFI OFF -> Mode Uji Sensor Offline.");
  strncpy(firebaseUID, "OFFLINE_TEST_UID", sizeof(firebaseUID) - 1);
  firebaseUID[sizeof(firebaseUID) - 1] = '\0';

  strncpy(macAddress, "OFFLINENODE01", sizeof(macAddress) - 1);
  macAddress[sizeof(macAddress) - 1] = '\0';
  return;

#else
  // MODE SIMULASI QEMU (Tanpa radio WiFi fisik)
  #ifdef USE_QEMU
    Serial.println("[QEMU] Bypass Radio WiFi - Uji Dual-Core & Memori...");
    strncpy(firebaseUID, "QEMU_TEST_UID", sizeof(firebaseUID) - 1);
    strncpy(macAddress, "246F28AABBCC", sizeof(macAddress) - 1);
    return;
  #endif

  // MODE SIMULASI WOKWI (Virtual WiFi Gateway)
  #ifdef USE_WOKWI
    Serial.println("[WOKWI] Menghubungkan ke Virtual WiFi...");
    WiFi.begin("Wokwi-GUEST", "", 6);
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      Serial.print(".");
    }
    Serial.println("\n[WOKWI] WiFi Terhubung!");

    strncpy(firebaseUID, "SIMULASI_WOKWI_UID", sizeof(firebaseUID) - 1);
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    strncpy(macAddress, mac.c_str(), sizeof(macAddress) - 1);
    return;
  #endif

  // MODE PRODUKSI ASLI (Captive Portal + NVS Vault)
  #if !defined(USE_QEMU) && !defined(USE_WOKWI)
    WiFiManager wm;

    preferences.begin("proveggies", false);
    String savedUID = preferences.getString("uid", "");

    WiFiManagerParameter custom_uid("uid", "Firebase UID Pemilik", savedUID.c_str(), 64);
    wm.addParameter(&custom_uid);
    wm.setConfigPortalTimeout(180);

    if (!wm.autoConnect("PROVEGGIES_SETUP", "admin123")) {
      Serial.println("[NETWORK] Gagal konek WiFi, Restarting...");
      delay(3000);
      ESP.restart();
    }

    String currentUID = custom_uid.getValue();
    if (currentUID != savedUID) {
      preferences.putString("uid", currentUID);
    }

    strncpy(firebaseUID, currentUID.c_str(), sizeof(firebaseUID) - 1);
    firebaseUID[sizeof(firebaseUID) - 1] = '\0';

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    strncpy(macAddress, mac.c_str(), sizeof(macAddress) - 1);
    macAddress[sizeof(macAddress) - 1] = '\0';

    preferences.end();
    Serial.println("[NETWORK] WiFi & NVS Siap!");
  #endif
#endif
}