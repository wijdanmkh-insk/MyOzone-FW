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
  strncpy(firebaseUID, "offline_user", sizeof(firebaseUID) - 1);
  firebaseUID[sizeof(firebaseUID) - 1] = '\0';

  strncpy(macAddress, "OFFLINENODE01", sizeof(macAddress) - 1);
  macAddress[sizeof(macAddress) - 1] = '\0';
  return;

#else
  // MODE SIMULASI QEMU (Tanpa radio WiFi fisik)
  #ifdef USE_QEMU
    Serial.println("[QEMU] Bypass Radio WiFi - Uji Dual-Core & Memori...");
    strncpy(firebaseUID, "qemu_user", sizeof(firebaseUID) - 1);
    firebaseUID[sizeof(firebaseUID) - 1] = '\0';

    strncpy(macAddress, "246F28AABBCC", sizeof(macAddress) - 1);
    macAddress[sizeof(macAddress) - 1] = '\0';
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

    strncpy(firebaseUID, "simulasi_wokwi", sizeof(firebaseUID) - 1);
    firebaseUID[sizeof(firebaseUID) - 1] = '\0';

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    strncpy(macAddress, mac.c_str(), sizeof(macAddress) - 1);
    macAddress[sizeof(macAddress) - 1] = '\0';
    return;
  #endif

  // MODE PRODUKSI ASLI (Captive Portal + NVS Vault)
  #if !defined(USE_QEMU) && !defined(USE_WOKWI)
    WiFiManager wm;

    // Buka NVS namespace "proveggies" (key "uid" dipakai menyimpan Username pemilik)
    preferences.begin("proveggies", false);
    String savedUsername = preferences.getString("uid", "");
    savedUsername.trim();

    bool isLocked = (savedUsername.length() > 0);

    // Atribut HTML: Jika sudah ada pemilik, buat kolom abu-abu & terkunci (readonly)
    const char* customAttr = isLocked
      ? "readonly style='background-color:#e2e8f0; color:#64748b; border:1px solid #cbd5e1; cursor:not-allowed;'"
      : "placeholder='Masukkan Username Akun' required";

    const char* fieldLabel = isLocked
      ? "Username Pemilik (Terkunci - Hapus di App untuk ganti)"
      : "Username Pemilik";

    WiFiManagerParameter custom_user("uid", fieldLabel, savedUsername.c_str(), 64, customAttr);
    wm.addParameter(&custom_user);
    wm.setConfigPortalTimeout(180);

    // Nama AP: PROVEGGIES_SETUP | Password AP: admin123
    if (!wm.autoConnect("PROVEGGIES_SETUP", "admin123")) {
      Serial.println("[NETWORK] Gagal konek WiFi, Restarting...");
      preferences.end();
      delay(3000);
      ESP.restart();
    }

    // Proteksi Backend: Hanya simpan username baru JIKA NVS sebelumnya kosong
    if (!isLocked) {
      String inputUser = custom_user.getValue();
      inputUser.trim();
      if (inputUser.length() > 0) {
        preferences.putString("uid", inputUser);
        savedUsername = inputUser;
        Serial.println("[NVS] Username pemilik baru disimpan: " + savedUsername);
      }
    } else {
      Serial.println("[NVS] Mempertahankan pemilik lama: " + savedUsername);
    }

    // Salin ke buffer global firebaseUID (dipakai firebase_manager untuk lookup /usernames/<username>)
    strncpy(firebaseUID, savedUsername.c_str(), sizeof(firebaseUID) - 1);
    firebaseUID[sizeof(firebaseUID) - 1] = '\0';

    // Ambil MAC Address tanpa tanda titik dua (contoh: 246F28AABBCC)
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    strncpy(macAddress, mac.c_str(), sizeof(macAddress) - 1);
    macAddress[sizeof(macAddress) - 1] = '\0';

    preferences.end();
    Serial.printf("[NETWORK] Siap! MAC: %s | Owner Username: %s\n", macAddress, firebaseUID);
  #endif
#endif
}