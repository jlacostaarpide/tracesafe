#pragma once

// ─── WiFi ────────────────────────────────────────────────────────────────────
#define WIFI_SSID "RubahWifi"
#define WIFI_PASSWORD "4ccm7veh"

// ─── Firebase Realtime Database ──────────────────────────────────────────────
#define FIREBASE_RTDB_URL "https://tracesafe-7f238-default-rtdb.firebaseio.com/"

// ─── Usuarios autorizados ────────────────────────────────────────────────────
// El índice del array coincide con (id - 1):
//   id=1 → Arturo
//   id=2 → Juan
//   id=3 → Mercedes
//   id=4 → Ruben
//   id=5 → Raul
//   id=6 → Santi

struct BrazaleteUsuario {
  byte uid[4];             // UID físico del brazalete
  const char *firebaseUid; // UID del usuario en Firebase Auth
  const char *nombre;      // Nombre que aparece en la app
};

static BrazaleteUsuario BRACELET_USERS[] = {
    {{0x01, 0x9E, 0x98, 0x9D},
     "ka49ivYMtYWGEX6XUhmBsRTC5FP2",
     "Arturo"},                                                         // id=1
    {{0x11, 0xEF, 0xD4, 0x5D}, "O5SCuUuuaaPSfYjTNjyhXeYgVeG2", "Juan"}, // id=2
    {{0xCA, 0x1C, 0x47, 0x77},
     "K55Vmwhh56S4wjRwIGKwmxkb4X13",
     "Mercedes"},                                                        // id=3
    {{0xFE, 0xAF, 0x26, 0x47}, "Q1sLd4hIslPJCfPAbymrWwg9oOh1", "Ruben"}, // id=4
    {{0x0E, 0xEE, 0xE2, 0x19}, "UWQmIsiquQT1WO5qO5rVxg6MxYh1", "Raul"},  // id=5
    {{0x08, 0x09, 0xAF, 0xBF}, "fntMn9aaFBOkkG7g6qtuR5T2kf92", "Santi"}  // id=6

};

static const int NUM_USUARIOS =
    sizeof(BRACELET_USERS) / sizeof(BRACELET_USERS[0]);

// ─── Pines ESP32-S3 ──────────────────────────────────────────────────────────
#define PIN_SPI_SCK 36
#define PIN_SPI_MISO 37
#define PIN_SPI_MOSI 35
#define PIN_RFID_SS 10
#define PIN_RFID_RST 9

#define PIN_SERVO 3
#define PIN_LED_ROJO 8
#define PIN_LED_NARANJA 7
#define PIN_LED_VERDE 6
#define PIN_BOTON_EMERG 2

#define PIN_LCD_RS 15
#define PIN_LCD_EN 16
#define PIN_LCD_D4 17
#define PIN_LCD_D5 18
#define PIN_LCD_D6 21
#define PIN_LCD_D7 38
