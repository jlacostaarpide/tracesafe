#pragma once

// ─── WiFi ────────────────────────────────────────────────────────────────────
#define WIFI_SSID "in3wifi"
#define WIFI_PASSWORD "12345678"

// ─── Firebase Realtime Database ──────────────────────────────────────────────
// Firebase Console → Realtime Database → panel superior (URL terminada en
// .firebaseio.com)
#define FIREBASE_RTDB_URL "https://tracesafe-7f238-default-rtdb.firebaseio.com/"

// ─── Usuarios autorizados
// ───────────────────────────────────────────────────── Cada entrada vincula un
// brazalete RFID con un usuario de Firebase Auth.
//
// ¿Dónde encuentro el firebaseUid?
//   Firebase Console → Build → Authentication → Users → columna "User UID"
//
struct BrazaleteUsuario {
  byte uid[4];             // UID físico del brazalete
  const char *firebaseUid; // UID del usuario en Firebase Auth
  const char *nombre;      // Nombre que aparece en la app
};

static BrazaleteUsuario BRACELET_USERS[] = {
    {{0x11, 0xEF, 0xD4, 0x5D}, "O5SCuUuuaaPSfYjTNjyhXeYgVeG2", "Juan"},
    {{0xAA, 0xBB, 0xCC, 0xDD}, "Q1sLd4hIslPJCfPAbymrWwg9oOh1", "Ruben"},
    {{0x11, 0x22, 0x33, 0x44}, "fntMn9aaFBOkkG7g6qtuR5T2kf92", "Santi"},
};
static const int NUM_USUARIOS =
    sizeof(BRACELET_USERS) / sizeof(BRACELET_USERS[0]);

// ─── Pines ESP32-S3 ──────────────────────────────────────────────────────────
// SPI para MFRC522 (FSPI)
#define PIN_SPI_SCK 36
#define PIN_SPI_MISO 37
#define PIN_SPI_MOSI 35
#define PIN_RFID_SS 10
#define PIN_RFID_RST 9

// Actuadores / sensores
#define PIN_SERVO 4
#define PIN_LED_ROJO 5
#define PIN_LED_NARANJA 6
#define PIN_LED_VERDE 7
#define PIN_BOTON_EMERG 2

// LCD 16x2 modo 4-bit (RS, EN, D4, D5, D6, D7)
#define PIN_LCD_RS 15
#define PIN_LCD_EN 16
#define PIN_LCD_D4 17
#define PIN_LCD_D5 18
#define PIN_LCD_D6 21
#define PIN_LCD_D7 38
