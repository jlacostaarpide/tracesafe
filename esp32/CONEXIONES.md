# Conexiones ESP32-S3 — TraceSafe

## MFRC522 (RFID)

| MFRC522 | ESP32-S3 | Notas |
|---------|----------|-------|
| VCC     | 3.3V     | ⚠️ 3.3V, NO 5V |
| GND     | GND      | |
| SCK     | GPIO 36  | FSPI_CLK |
| MISO    | GPIO 37  | FSPI_MISO |
| MOSI    | GPIO 35  | FSPI_MOSI |
| SDA/SS  | GPIO 10  | Chip Select |
| RST     | GPIO 9   | Reset |

## Servo motor (SG90 / MG90S)

| Cable servo | ESP32-S3 | Notas |
|-------------|----------|-------|
| Señal (naranja) | GPIO 4 | PWM |
| VCC (rojo)      | 5V (VBUS o externo) | No usar 3.3V del ESP32 |
| GND (marrón)    | GND | GND común con ESP32 |

> Si el servo causa reinicios del ESP32, aliméntalo desde una fuente externa de 5V
> con GND compartido con el ESP32.

## LEDs (con resistencia de 220 Ω en serie cada uno)

| LED      | ESP32-S3 |
|----------|----------|
| Rojo     | GPIO 5   |
| Naranja  | GPIO 6   |
| Verde    | GPIO 7   |

Esquema por LED:
```
GPIO → [220 Ω] → Ánodo LED → Cátodo LED → GND
```

## Botón de emergencia

```
GPIO 2 ──────┬──── Botón ──── GND
             │
           (INPUT_PULLUP: el código ya lo activa)
```

## LCD 16x2 (modo 4 bits, sin módulo I2C)

| LCD pin | Nombre | ESP32-S3 / otro |
|---------|--------|-----------------|
| 1  VSS  | GND    | GND |
| 2  VDD  | 5V     | 5V |
| 3  V0   | Contraste | Pin central de potenciómetro 10 kΩ (extremos a 5V y GND) |
| 4  RS   | Register Select | GPIO 15 |
| 5  RW   | Read/Write | GND (siempre escritura) |
| 6  E    | Enable | GPIO 16 |
| 7–10 D0–D3 | — | Sin conectar (modo 4 bits) |
| 11 D4   | Data 4 | GPIO 17 |
| 12 D5   | Data 5 | GPIO 18 |
| 13 D6   | Data 6 | GPIO 21 |
| 14 D7   | Data 7 | GPIO 38 |
| 15 A    | Backlight + | 5V (con resistencia 220 Ω) |
| 16 K    | Backlight − | GND |

---

## Resumen de pines ESP32-S3

```
GPIO  2  →  Botón emergencia (INPUT_PULLUP)
GPIO  4  →  Servo (señal PWM)
GPIO  5  →  LED Rojo
GPIO  6  →  LED Naranja
GPIO  7  →  LED Verde
GPIO  9  →  RFID RST
GPIO 10  →  RFID SS/SDA
GPIO 15  →  LCD RS
GPIO 16  →  LCD EN
GPIO 17  →  LCD D4
GPIO 18  →  LCD D5
GPIO 21  →  LCD D6
GPIO 35  →  RFID MOSI
GPIO 36  →  RFID SCK
GPIO 37  →  RFID MISO
GPIO 38  →  LCD D7
```

---

## Setup en Firebase Console (pasos obligatorios)

1. **Activar Realtime Database**
   - Firebase Console → Build → Realtime Database → Crear base de datos
   - Región: Europe-West1 (o la más cercana)
   - Empezar en **modo de prueba** (permite leer y escribir sin auth)

2. **Copiar la URL de la RTDB**
   - Aparece en el panel: `https://tracesafe-7f238-default-rtdb.firebaseio.com`
   - Pégala en `config.h` → `FIREBASE_RTDB_URL`

3. **Reglas de seguridad** (para pruebas)
   ```json
   {
     "rules": {
       ".read": true,
       ".write": true
     }
   }
   ```
   > Para producción, restringir a usuarios autenticados.

4. **Instalar dependencia en la app React Native**
   ```bash
   npx expo install firebase
   ```
   (si ya está instalado, no hace falta)

---

## Librerías Arduino a instalar

Arduino IDE → Sketch → Include Library → Manage Libraries:

| Librería | Autor |
|----------|-------|
| MFRC522  | GithubCommunity |
| ESP32Servo | Kevin Harrington / madhephaestus |

Board: **ESP32S3 Dev Module** (paquete "esp32" de Espressif ≥ 2.0)
