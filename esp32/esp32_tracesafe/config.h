#pragma once

// ─── WiFi ────────────────────────────────────────────────────────────────────
#define WIFI_SSID "RubahWifi"
#define WIFI_PASSWORD "4ccm7veh"

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
    {{0xFE, 0xAF, 0x26, 0x47}, "Q1sLd4hIslPJCfPAbymrWwg9oOh1", "Ruben"},
    {{0x08, 0x09, 0xAF, 0xBF}, "fntMn9aaFBOkkG7g6qtuR5T2kf92", "Santi"},
    {{0x01, 0x9E, 0x98, 0x9D}, "ka49ivYMtYWGEX6XUhmBsRTC5FP2", "Arturo"},
    {{0x0E, 0xEE, 0xE2, 0x19}, "UWQmIsiquQT1WO5qO5rVxg6MxYh1", "Raul"},
    {{0xCA, 0x1C, 0x47, 0x77}, "K55Vmwhh56S4wjRwIGKwmxkb4X13", "Mercedes"},
};
static const int NUM_USUARIOS =
    sizeof(BRACELET_USERS) / sizeof(BRACELET_USERS[0]);

// ─── Pines ESP32 DEVKITV1 (WROOM-32) ────────────────────────────────────────
// ⚠️  GPIO 6-11: reservados para flash interna → NO USAR
// ⚠️  GPIO 34,35,36,39: solo entrada → NO USAR como salida
// ⚠️  GPIO 1,3: UART TX/RX del Serial Monitor → NO USAR

// Actuadores / sensores
#define PIN_SERVO 4
#define PIN_LED_ROJO 17
#define PIN_LED_NARANJA 25
#define PIN_LED_VERDE 26
#define PIN_BOTON_EMERG 16

// LCD 16x2 modo 4-bit (RS, EN, D4, D5, D6, D7)
#define PIN_LCD_RS 13
#define PIN_LCD_EN 5
#define PIN_LCD_D4 23
#define PIN_LCD_D5 21
#define PIN_LCD_D6 32
#define PIN_LCD_D7 33