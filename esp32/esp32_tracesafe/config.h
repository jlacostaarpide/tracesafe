#pragma once

// ─── WiFi ────────────────────────────────────────────────────────────────────
#define WIFI_SSID "in3wifi"
#define WIFI_PASSWORD "12345678"

// ─── Firebase Realtime Database ──────────────────────────────────────────────
// Firebase Console → Realtime Database → panel superior (URL terminada en
// .firebaseio.com)
#define FIREBASE_RTDB_URL "https://tracesafe-7f238-default-rtdb.firebaseio.com"

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

// ─── Pines ESP32 DEVKITV1 (WROOM-32) ────────────────────────────────────────
// ⚠️  GPIO 6-11: reservados para flash interna → NO USAR
// ⚠️  GPIO 34,35,36,39: solo entrada → NO USAR como salida
// ⚠️  GPIO 1,3: UART TX/RX del Serial Monitor → NO USAR

// SPI para MFRC522 — pines VSPI nativos del ESP32
#define PIN_SPI_SCK  18
#define PIN_SPI_MISO 19
#define PIN_SPI_MOSI 23
#define PIN_RFID_SS   5
#define PIN_RFID_RST 17

// Actuadores / sensores
#define PIN_SERVO        4
#define PIN_LED_ROJO    25
#define PIN_LED_NARANJA 26
#define PIN_LED_VERDE   27
#define PIN_BOTON_EMERG 13

// LCD 16x2 modo 4-bit (RS, EN, D4, D5, D6, D7)
#define PIN_LCD_RS 14
#define PIN_LCD_EN 22
#define PIN_LCD_D4 16
#define PIN_LCD_D5 21
#define PIN_LCD_D6 32
#define PIN_LCD_D7 33
