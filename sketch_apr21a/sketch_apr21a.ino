#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal.h>

// RFID
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// LCD
LiquidCrystal lcd(A1, A2, A3, A4, A5, 5);

// Servo
Servo servo;

// LEDs
int ledRojo = 8;
int ledNaranja = 7;
int ledVerde = 6;

// Botón
int botonEmergencia = 2;

// UID autorizado
byte uidAutorizado[] = {0x11, 0xEF, 0xD4, 0x5D};

// Estados
bool puertaAbierta = false;
bool puertaEnMovimiento = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  servo.attach(3);
  servo.write(0);

  pinMode(ledRojo, OUTPUT);
  pinMode(ledNaranja, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  pinMode(botonEmergencia, INPUT_PULLUP);

  lcd.begin(16, 2);

  estadoCerrado();
  pantallaInicial();
}

void loop() {

  // BOTÓN EMERGENCIA
  if (digitalRead(botonEmergencia) == LOW && !puertaAbierta && !puertaEnMovimiento) {
    lcd.clear();
    lcd.print("EMERGENCIA");
    lcd.setCursor(0,1);
    lcd.print("Abriendo...");

    abrirPuerta();
    delay(4000);

    lcd.clear();
    lcd.print("Cerrando...");
    cerrarPuerta();

    pantallaInicial();
    return;
  }

  // RFID
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  if (tarjetaValida()) {

    lcd.clear();
    lcd.print("Abriendo puerta");
    abrirPuerta();

    lcd.clear();
    lcd.print("Bienvenido ID:");
    lcd.setCursor(0,1);
    mostrarUID_LCD();

    delay(4000);

    lcd.clear();
    lcd.print("Cerrando puerta");
    cerrarPuerta();

    pantallaInicial();

  } else {
    lcd.clear();
    lcd.print("Acceso");
    lcd.setCursor(0,1);
    lcd.print("denegado");

    accesoDenegado();
    pantallaInicial();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// -------- FUNCIONES --------

void pantallaInicial() {
  lcd.clear();
  lcd.print("Introducir");
  lcd.setCursor(0,1);
  lcd.print("tarjeta");
}

bool tarjetaValida() {
  if (rfid.uid.size != 4) return false;

  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != uidAutorizado[i]) return false;
  }
  return true;
}

void mostrarUID_LCD() {
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) lcd.print("0");
    lcd.print(rfid.uid.uidByte[i], HEX);
    lcd.print(" ");
  }
}

void estadoCerrado() {
  digitalWrite(ledRojo, HIGH);
  digitalWrite(ledNaranja, LOW);
  digitalWrite(ledVerde, LOW);
}

void estadoMovimiento() {
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledNaranja, HIGH);
  digitalWrite(ledVerde, LOW);
}

void estadoAbierto() {
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledNaranja, LOW);
  digitalWrite(ledVerde, HIGH);
}

void abrirPuerta() {
  puertaEnMovimiento = true;
  estadoMovimiento();

  for (int pos = 0; pos <= 90; pos++) {
    servo.write(pos);
    delay(15);
  }

  puertaEnMovimiento = false;
  puertaAbierta = true;
  estadoAbierto();
}

void cerrarPuerta() {
  puertaEnMovimiento = true;
  estadoMovimiento();

  for (int pos = 90; pos >= 0; pos--) {
    servo.write(pos);
    delay(15);
  }

  puertaEnMovimiento = false;
  puertaAbierta = false;
  estadoCerrado();
}

void accesoDenegado() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledRojo, HIGH);
    delay(200);
    digitalWrite(ledRojo, LOW);
    delay(200);
  }
  estadoCerrado();
}