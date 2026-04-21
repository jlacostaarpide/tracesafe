#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("Acerca una tarjeta al lector...");
}

void loop() {
  // Si no hay tarjeta nueva, no hace nada
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Si no puede leer la tarjeta, no hace nada
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID: ");

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  }

  Serial.println();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(1000);
}