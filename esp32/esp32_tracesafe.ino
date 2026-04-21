/*
  TraceSafe – ESP32-S3
  ─────────────────────────────────────────────────────────────────────────────
  Librerías necesarias (instalar desde Arduino IDE → Library Manager):
    · MFRC522   by GithubCommunity
    · ESP32Servo by Kevin Harrington / madhephaestus
    · LiquidCrystal ya viene incluida con el core de Arduino

  Board: "ESP32S3 Dev Module"  (esp32 by Espressif, v2.x o superior)
  ─────────────────────────────────────────────────────────────────────────────
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <LiquidCrystal.h>
#include "config.h"

// ─── Objetos hardware ─────────────────────────────────────────────────────────
MFRC522       rfid(PIN_RFID_SS, PIN_RFID_RST);
Servo         servo;
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN,
                  PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// ─── Estado interno ───────────────────────────────────────────────────────────
bool puertaAbierta = false;

// Ruta RTDB donde vive la solicitud activa
const String RTDB_PATH = String(FIREBASE_RTDB_URL) + "/access_requests.json";

// ═════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // SPI con pines personalizados del ESP32-S3 (FSPI)
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_RFID_SS);
  rfid.PCD_Init();

  servo.attach(PIN_SERVO);
  servo.write(0);

  pinMode(PIN_LED_ROJO,    OUTPUT);
  pinMode(PIN_LED_NARANJA, OUTPUT);
  pinMode(PIN_LED_VERDE,   OUTPUT);
  pinMode(PIN_BOTON_EMERG, INPUT_PULLUP);

  lcd.begin(16, 2);
  estadoCerrado();

  conectarWiFi();
  resetearFirebase();
  pantallaInicial();
}

// ═════════════════════════════════════════════════════════════════════════════
void loop() {

  // ── Botón de emergencia ──────────────────────────────────────────────────
  if (digitalRead(PIN_BOTON_EMERG) == LOW && !puertaAbierta) {
    lcd.clear();
    lcd.print("EMERGENCIA");
    lcd.setCursor(0, 1);
    lcd.print("Abriendo...");
    abrirPuerta();
    delay(5000);
    cerrarPuerta();
    pantallaInicial();
    return;
  }

  // ── RFID ─────────────────────────────────────────────────────────────────
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = leerUID();
  Serial.println("UID detectado: " + uid);

  BrazaleteUsuario* usuario = buscarBrazalete();

  if (usuario != nullptr) {
    if (WiFi.status() == WL_CONNECTED) {
      esperarAutorizacion(uid, usuario);
    } else {
      // Sin WiFi: abre localmente con aviso
      lcd.clear();
      lcd.print("Sin WiFi");
      lcd.setCursor(0, 1);
      lcd.print("Acceso local");
      abrirPuerta();
      delay(5000);
      cerrarPuerta();
      pantallaInicial();
    }
  } else {
    lcd.clear();
    lcd.print("Acceso");
    lcd.setCursor(0, 1);
    lcd.print("denegado");
    accesoDenegado();
    pantallaInicial();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ═════════════════════════════════════════════════════════════════════════════
//  FLUJO PRINCIPAL: escribe solicitud → espera respuesta de la app
// ═════════════════════════════════════════════════════════════════════════════
void esperarAutorizacion(String uid, BrazaleteUsuario* usuario) {
  if (!escribirSolicitud(uid, usuario)) {
    lcd.clear();
    lcd.print("Error Firebase");
    delay(2000);
    pantallaInicial();
    return;
  }

  lcd.clear();
  lcd.print("Esperando app");
  lcd.setCursor(0, 1);
  lcd.print("               ");

  unsigned long inicio = millis();
  const unsigned long TIMEOUT_MS = 30000;  // 30 s máximo
  String estado = "pending";
  int dotsCount  = 0;

  while (millis() - inicio < TIMEOUT_MS) {
    delay(1500);

    // Animación de puntos mientras espera
    lcd.setCursor(0, 1);
    lcd.print("               ");
    lcd.setCursor(0, 1);
    for (int i = 0; i < (dotsCount % 4); i++) lcd.print(".");
    dotsCount++;

    estado = leerEstado();
    Serial.println("Estado Firebase: " + estado);

    if (estado == "approved") {
      lcd.clear();
      lcd.print("Autorizado!");
      abrirPuerta();
      delay(5000);
      cerrarPuerta();
      break;
    }

    if (estado == "denied") {
      lcd.clear();
      lcd.print("Acceso");
      lcd.setCursor(0, 1);
      lcd.print("denegado");
      accesoDenegado();
      break;
    }
  }

  if (estado == "pending") {
    // Timeout sin respuesta
    lcd.clear();
    lcd.print("Sin respuesta");
    accesoDenegado();
  }

  resetearFirebase();
  pantallaInicial();
}

// ─── Firebase: escribe solicitud pendiente ────────────────────────────────────
bool escribirSolicitud(String uid, BrazaleteUsuario* usuario) {
  WiFiClientSecure client;
  client.setInsecure();  // omite verificación de certificado (válido para proyecto académico)

  HTTPClient http;
  http.begin(client, RTDB_PATH);
  http.addHeader("Content-Type", "application/json");

  // userId indica a qué teléfono debe llegar la solicitud
  String body = "{\"status\":\"pending\","
                "\"braceletId\":\"" + uid + "\","
                "\"userName\":\"" + String(usuario->nombre) + "\","
                "\"userId\":\"" + String(usuario->firebaseUid) + "\","
                "\"timestamp\":" + String(millis()) + "}";

  int code = http.PUT(body);
  http.end();

  Serial.println("PUT Firebase → HTTP " + String(code));
  return (code == 200);
}

// ─── Firebase: lee el campo status ────────────────────────────────────────────
String leerEstado() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, RTDB_PATH);

  int code = http.GET();
  if (code != 200) {
    http.end();
    return "error";
  }

  String payload = http.getString();
  http.end();

  // Extrae el valor de "status" del JSON plano que devuelve RTDB
  int idx = payload.indexOf("\"status\":\"");
  if (idx == -1) return "error";
  idx += 10;
  int fin = payload.indexOf("\"", idx);
  return payload.substring(idx, fin);
}

// ─── Firebase: resetea el nodo a idle al terminar ─────────────────────────────
void resetearFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, RTDB_PATH);
  http.addHeader("Content-Type", "application/json");

  String body = "{\"status\":\"idle\","
                "\"braceletId\":\"\","
                "\"userName\":\"\","
                "\"timestamp\":0}";

  http.PUT(body);
  http.end();
}

// ═════════════════════════════════════════════════════════════════════════════
//  HELPERS WiFi
// ═════════════════════════════════════════════════════════════════════════════
void conectarWiFi() {
  lcd.clear();
  lcd.print("Conectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    intentos++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi conectado");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    Serial.println("WiFi OK → " + WiFi.localIP().toString());
  } else {
    lcd.print("Sin WiFi");
    lcd.setCursor(0, 1);
    lcd.print("Modo local");
    Serial.println("WiFi FAIL");
  }
  delay(2000);
}

// ═════════════════════════════════════════════════════════════════════════════
//  HELPERS RFID
// ═════════════════════════════════════════════════════════════════════════════
String leerUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

// Devuelve el puntero al usuario dueño del brazalete, o nullptr si no está autorizado
BrazaleteUsuario* buscarBrazalete() {
  if (rfid.uid.size != 4) return nullptr;
  for (int u = 0; u < NUM_USUARIOS; u++) {
    bool match = true;
    for (byte i = 0; i < 4; i++) {
      if (rfid.uid.uidByte[i] != BRACELET_USERS[u].uid[i]) {
        match = false;
        break;
      }
    }
    if (match) return &BRACELET_USERS[u];
  }
  return nullptr;
}

// ═════════════════════════════════════════════════════════════════════════════
//  HELPERS HARDWARE
// ═════════════════════════════════════════════════════════════════════════════
void pantallaInicial() {
  lcd.clear();
  lcd.print("Introducir");
  lcd.setCursor(0, 1);
  lcd.print("tarjeta");
}

void estadoCerrado() {
  digitalWrite(PIN_LED_ROJO,    HIGH);
  digitalWrite(PIN_LED_NARANJA, LOW);
  digitalWrite(PIN_LED_VERDE,   LOW);
}

void estadoMovimiento() {
  digitalWrite(PIN_LED_ROJO,    LOW);
  digitalWrite(PIN_LED_NARANJA, HIGH);
  digitalWrite(PIN_LED_VERDE,   LOW);
}

void estadoAbierto() {
  digitalWrite(PIN_LED_ROJO,    LOW);
  digitalWrite(PIN_LED_NARANJA, LOW);
  digitalWrite(PIN_LED_VERDE,   HIGH);
}

void abrirPuerta() {
  estadoMovimiento();
  for (int pos = 0; pos <= 90; pos++) {
    servo.write(pos);
    delay(15);
  }
  puertaAbierta = true;
  estadoAbierto();
}

void cerrarPuerta() {
  estadoMovimiento();
  for (int pos = 90; pos >= 0; pos--) {
    servo.write(pos);
    delay(15);
  }
  puertaAbierta = false;
  estadoCerrado();
}

void accesoDenegado() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_LED_ROJO, HIGH);
    delay(200);
    digitalWrite(PIN_LED_ROJO, LOW);
    delay(200);
  }
  estadoCerrado();
}
