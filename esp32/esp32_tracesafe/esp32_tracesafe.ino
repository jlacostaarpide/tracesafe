#include "config.h"
#include <ESP32Servo.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Servo
Servo servo;

// Estado interno
bool puertaAbierta = false;

// Ruta Firebase
const String RTDB_PATH = String(FIREBASE_RTDB_URL) + "/access_requests.json";

WebServer server(80);

// ======================================================
// Convierte los 4 bytes uid[] a String hex ("019E989D")
// ======================================================
String uidToString(BrazaleteUsuario *usuario) {
  String uid = "";
  for (int i = 0; i < 4; i++) {
    if (usuario->uid[i] < 0x10)
      uid += "0";
    uid += String(usuario->uid[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

// ======================================================
// Recibe el ID numérico del otro ESP y lanza el flujo
// ======================================================
void recibirID() {
  if (!server.hasArg("id")) {
    server.send(400, "text/plain", "Falta parametro id");
    return;
  }

  int idRecibido = server.arg("id").toInt();
  Serial.print("ID recibido: ");
  Serial.println(idRecibido);

  // id=1 → BRACELET_USERS[0] (Arturo)
  // id=2 → BRACELET_USERS[1] (Juan)
  // ... etc.
  if (idRecibido >= 1 && idRecibido <= NUM_USUARIOS) {
    BrazaleteUsuario *usuario = &BRACELET_USERS[idRecibido - 1];
    String uid = uidToString(usuario);

    Serial.print("Usuario: ");
    Serial.println(usuario->nombre);
    Serial.print("UID: ");
    Serial.println(uid);

    server.send(200, "text/plain", "OK");
    esperarAutorizacion(uid, usuario);
  } else {
    Serial.println("ID fuera de rango");
    server.send(400, "text/plain", "ID fuera de rango");
  }
}

void setup() {
  Serial.begin(115200);

  servo.attach(PIN_SERVO);
  servo.write(0);

  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_NARANJA, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_BOTON_EMERG, INPUT_PULLUP);

  estadoCerrado();

  conectarWiFi();

  Serial.println("Iniciando reset Firebase...");
  resetearFirebase();

  Serial.println("Sistema listo.");
  Serial.println("Escribe 'test' para simular acceso de Juan.");
  Serial.println("O pulsa el boton para emergencia.");
  Serial.println(WiFi.localIP());

  server.on("/id", recibirID);
  server.begin();
}

void loop() {
  server.handleClient();

  // Botón de emergencia
  if (digitalRead(PIN_BOTON_EMERG) == LOW && !puertaAbierta) {
    Serial.println("EMERGENCIA - Abriendo puerta");
    abrirPuerta();
    delay(5000);
    Serial.println("Cerrando puerta");
    cerrarPuerta();
    Serial.println("Sistema listo.");
    delay(500);
    return;
  }

  // Test por Serial Monitor — escribe "test" para simular Juan
  static String serialBuf = "";

  while (Serial.available()) {
    char c = (char)Serial.read();

    if (c == '\n' || c == '\r') {
      serialBuf.trim();

      if (serialBuf == "test") {
        Serial.println("[TEST] Simulando tarjeta de Juan...");
        BrazaleteUsuario *juan = &BRACELET_USERS[1]; // id=2 → Juan
        esperarAutorizacion(uidToString(juan), juan);
      }

      serialBuf = "";
    } else {
      serialBuf += c;
    }
  }
}

// ======================================================
// FLUJO PRINCIPAL: escribe solicitud → espera respuesta app
// ======================================================

void esperarAutorizacion(String uid, BrazaleteUsuario *usuario) {
  if (!escribirSolicitud(uid, usuario)) {
    Serial.println("Error Firebase al escribir solicitud");
    return;
  }

  Serial.println("Solicitud enviada a Firebase.");
  Serial.println("Esperando respuesta de la app...");

  unsigned long inicio = millis();
  const unsigned long TIMEOUT_MS = 30000;

  String estado = "pending";

  while (millis() - inicio < TIMEOUT_MS) {
    delay(1500);

    estado = leerEstado();
    Serial.println("Estado Firebase: " + estado);

    if (estado == "approved") {
      Serial.println("Autorizado. Abriendo puerta.");
      abrirPuerta();
      delay(5000);
      Serial.println("Cerrando puerta.");
      cerrarPuerta();
      break;
    }

    if (estado == "denied") {
      Serial.println("Acceso denegado por la app.");
      accesoDenegado();
      break;
    }
  }

  if (estado == "pending") {
    Serial.println("Sin respuesta de la app. Timeout.");
    accesoDenegado();
  }

  resetearFirebase();
  Serial.println("Sistema listo.");
}

// ======================================================
// Firebase
// ======================================================

bool escribirSolicitud(String uid, BrazaleteUsuario *usuario) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sin WiFi. No se puede enviar solicitud.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, RTDB_PATH);
  http.addHeader("Content-Type", "application/json");

  String body = "{\"status\":\"pending\","
                "\"braceletId\":\"" +
                uid +
                "\","
                "\"userName\":\"" +
                String(usuario->nombre) +
                "\","
                "\"userId\":\"" +
                String(usuario->firebaseUid) +
                "\","
                "\"timestamp\":" +
                String(millis()) + "}";

  int code = http.PUT(body);
  http.end();

  Serial.println("PUT Firebase HTTP " + String(code));

  return (code == 200);
}

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

  int idx = payload.indexOf("\"status\":\"");
  if (idx == -1)
    return "error";

  idx += 10;
  int fin = payload.indexOf("\"", idx);

  return payload.substring(idx, fin);
}

void resetearFirebase() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, RTDB_PATH);
  http.addHeader("Content-Type", "application/json");

  String body = "{\"status\":\"idle\","
                "\"braceletId\":\"\","
                "\"userName\":\"\","
                "\"userId\":\"\","
                "\"timestamp\":0}";

  http.PUT(body);
  http.end();
}

// ======================================================
// WiFi
// ======================================================

void conectarWiFi() {
  Serial.println("Conectando WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;

  while (WiFi.status() != WL_CONNECTED && intentos < 40) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado");
    Serial.println(WiFi.localIP());
    delay(3000);
  } else {
    Serial.println("No se pudo conectar a WiFi");
  }
}

// ======================================================
// Hardware
// ======================================================

void estadoCerrado() {
  digitalWrite(PIN_LED_ROJO, HIGH);
  digitalWrite(PIN_LED_NARANJA, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);
}

void estadoMovimiento() {
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_NARANJA, HIGH);
  digitalWrite(PIN_LED_VERDE, LOW);
}

void estadoAbierto() {
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_NARANJA, LOW);
  digitalWrite(PIN_LED_VERDE, HIGH);
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