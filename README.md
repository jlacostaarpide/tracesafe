# 🔐 TraceSafe

**Sistema IoT de control de accesos con autorización remota en tiempo real**, compuesto por un dispositivo embebido (ESP32 + RFID), una base de datos en la nube (Firebase) y una aplicación móvil (React Native / Expo) que permite a un usuario autorizado **aprobar o denegar el acceso de otra persona desde su teléfono**, en el momento en que ésta presenta su brazalete RFID en la puerta.

El nombre *TraceSafe* resume la idea del proyecto: dejar **traza** (registro y notificación en tiempo real de cada intento de acceso) para mantener el entorno **seguro** (safe), sustituyendo una cerradura tradicional por un flujo de doble verificación: *algo que tienes* (el brazalete RFID) + *alguien que autoriza* (la persona con la app).

> 📚 Proyecto académico desarrollado para la asignatura **TIC II**.

---

## ✨ ¿Qué hace TraceSafe?

1. Una persona acerca su **brazalete RFID** al lector conectado a un ESP32.
2. El sistema identifica al usuario asociado a ese brazalete y publica una **solicitud de acceso pendiente** en Firebase Realtime Database.
3. La app móvil del usuario responsable, que está escuchando esa base de datos en tiempo real, recibe una **notificación push** y muestra un modal con los datos de la solicitud (usuario, brazalete, hora).
4. El usuario pulsa **Autorizar** o **Denegar** desde su móvil.
5. El ESP32, que está esperando la respuesta, reacciona en consecuencia:
   - ✅ **Autorizado** → abre la puerta con un servomotor y enciende el LED verde.
   - ❌ **Denegado / timeout (30 s sin respuesta)** → mantiene la puerta cerrada y parpadea el LED rojo.
6. Además, existe un **botón físico de emergencia** que abre la puerta manualmente sin pasar por el flujo de autorización, para salidas de emergencia.

---

## 🏗️ Arquitectura

El sistema se apoya en tres capas que se comunican entre sí a través de **Firebase Realtime Database**, que actúa como intermediario y única fuente de verdad del estado de la solicitud de acceso:

```
┌──────────────────────┐        HTTPS (PUT/GET)        ┌─────────────────────────────┐
│   ESP32 + RFID +      │ ─────────────────────────────▶│   Firebase Realtime Database│
│   Servo + LEDs        │   escribe "access_requests"   │   nodo: /access_requests    │
│   (control de puerta) │◀───────────────────────────── │   { status, braceletId,     │
│                        │      lee estado (polling)     │     userName, userId, ts }  │
└──────────┬─────────────┘                                └──────────────┬──────────────┘
           │  HTTP local (/id?id=N)                                       │ onValue()
           │  desde un 2º ESP32 lector RFID                               │ (listener en tiempo real)
           ▼                                                              ▼
┌──────────────────────┐                                ┌─────────────────────────────┐
│  ESP32 lector RFID    │                                │   App móvil (React Native)  │
│  identifica brazalete │                                │   Firebase Auth + Expo      │
│  y llama al ESP32     │                                │   Notifications             │
│  controlador          │                                │  - LoginScreen              │
└──────────────────────┘                                │  - HomeScreen (modal        │
                                                          │    Autorizar / Denegar)     │
                                                          └─────────────────────────────┘
```

**Flujo de datos ESP32 → Firebase → App:**

- El **ESP32 controlador** (`esp32/esp32_tracesafe`) expone un endpoint HTTP local (`/id`) que recibe el identificador numérico del brazalete detectado (potencialmente desde un segundo ESP32 dedicado a la lectura RFID) y resuelve a qué usuario corresponde según la tabla `BRACELET_USERS` de `config.h`.
- Escribe (`HTTP PUT`) un objeto JSON en el nodo `/access_requests` de la Realtime Database con `status: "pending"`, el UID del brazalete, el nombre del usuario y su `userId` de Firebase Auth.
- Hace **polling** (`HTTP GET`) cada 1,5 s durante un máximo de 30 s esperando que `status` cambie a `approved` o `denied`.
- La **app móvil**, mediante un listener `onValue()` sobre ese mismo nodo, detecta la solicitud en tiempo real, filtra que sea para el usuario autenticado (`data.userId === user.uid`) y dispara una notificación push local (`expo-notifications`) junto con un modal de confirmación.
- Al pulsar Autorizar/Denegar, la app actualiza (`update()`) el nodo con el nuevo `status`, que el ESP32 lee en su siguiente ciclo de polling y traduce en la apertura del servomotor o en la señal de acceso denegado.
- La autenticación de usuarios (quién puede autorizar accesos) se gestiona con **Firebase Authentication** (email/contraseña), mientras que el estado de las solicitudes vive en **Firebase Realtime Database**.

---

## 🧰 Stack tecnológico

### Hardware / Firmware (ESP32)

| Componente | Detalle |
|---|---|
| Microcontrolador | ESP32 (DevKitV1 / WROOM-32, con variante documentada también para ESP32-S3) |
| Lector RFID | MFRC522 (SPI) |
| Actuador de cerradura | Servomotor SG90 / MG90S |
| Indicadores | LEDs rojo / naranja / verde (estado cerrado / en movimiento / abierto) |
| Entrada | Botón de emergencia (`INPUT_PULLUP`) |
| Pantalla (versión de prototipo) | LCD 16x2 en modo 4 bits |
| Conectividad | WiFi (`WiFi.h`), cliente HTTPS (`WiFiClientSecure`, `HTTPClient`) |
| Servidor local | `WebServer` (Arduino) — endpoint `/id` para recibir el identificador del brazalete |
| Librerías Arduino | `MFRC522`, `ESP32Servo`, `WiFi`, `HTTPClient`, `WiFiClientSecure`, `WebServer` |
| IDE | Arduino IDE con el paquete de placas **esp32** de Espressif (≥ 2.0), placa *ESP32 Dev Module* / *ESP32S3 Dev Module* |

### Cloud

| Servicio | Uso |
|---|---|
| **Firebase Authentication** | Login de los usuarios que pueden autorizar accesos (email/contraseña) |
| **Firebase Realtime Database** | Canal de comunicación en tiempo real entre el ESP32 y la app (`/access_requests`) |
| **Firestore** | Inicializado en el proyecto (`firebaseConfig.js`) para uso futuro / extensión de datos |

### App móvil

| Tecnología | Versión |
|---|---|
| [Expo](https://expo.dev) | ~54 |
| React | 19.1.0 |
| React Native | 0.81.5 |
| React Navigation (native stack) | ^7 |
| Firebase JS SDK | ^12.12.1 |
| expo-notifications | ~0.32 |
| @react-native-async-storage/async-storage | 2.2.0 |

---

## 📱 Interfaz de la app

La app cuenta con dos pantallas principales, con una estética oscura y acentos en cian (`#00d4ff`) y verde (`#00ff88`):

- **`LoginScreen`** — pantalla de acceso con email y contraseña contra Firebase Authentication.
- **`HomeScreen`** — pantalla principal en estado de *escucha permanente*:
  - Cabecera con el usuario autenticado y botón de cerrar sesión.
  - Tarjeta de estado ("Sistema activo — Escuchando solicitudes de acceso…") con indicador animado.
  - Icono de radar con animación de pulso mientras no hay solicitudes.
  - **Modal de solicitud de acceso**, que aparece automáticamente cuando el ESP32 detecta un brazalete asignado al usuario, mostrando el nombre del solicitante, el ID del brazalete y la hora, con botones para **Autorizar** (✓) o **Denegar** (✗).

---

## 🚀 Puesta en marcha

### 1. App móvil (React Native / Expo)

Requisitos: Node.js LTS y la app **Expo Go** en el móvil (o un emulador Android/iOS).

```bash
git clone <url-del-repositorio>
cd tracesafe
npm install
npx expo start
```

Escanea el QR con Expo Go, o pulsa `a` / `i` en la terminal para abrir el emulador Android/iOS.

> El proyecto ya incluye la configuración de Firebase en `firebaseConfig.js`. Si vas a desplegar tu propia instancia, sustituye las credenciales por las de tu propio proyecto de Firebase (Console → Configuración del proyecto → SDK de Firebase).

### 2. Firmware del ESP32 (Arduino IDE)

1. Instala el **Arduino IDE** (2.x recomendado) y añade el soporte de placas **esp32** de Espressif (Gestor de placas ≥ 2.0).
2. Desde *Sketch → Include Library → Manage Libraries*, instala:
   - `MFRC522` (GithubCommunity)
   - `ESP32Servo` (Kevin Harrington / madhephaestus)
3. Abre el sketch principal: `esp32/esp32_tracesafe/esp32_tracesafe.ino`.
4. Edita `esp32/esp32_tracesafe/config.h` con:
   - Tus credenciales WiFi (`WIFI_SSID`, `WIFI_PASSWORD`).
   - La URL de tu Realtime Database (`FIREBASE_RTDB_URL`).
   - La tabla `BRACELET_USERS`, vinculando el UID físico de cada brazalete RFID con el `firebaseUid` (obtenido en Firebase Console → Authentication → Users) del usuario que debe recibir la solicitud.
5. Selecciona la placa correspondiente (*ESP32 Dev Module* o *ESP32S3 Dev Module* según el modelo) y el puerto serie, y sube el sketch.
6. Conecta el hardware según la guía de pines incluida en el repositorio:
   - [`esp32/CONEXIONES.md`](esp32/CONEXIONES.md) — variante ESP32-S3.
   - [`esp32/pin_esp32.txt`](esp32/pin_esp32.txt) — variante ESP32 DevKitV1 (WROOM-32).
7. En Firebase Console, activa **Realtime Database** en modo de prueba (o con reglas restringidas a usuarios autenticados para producción) y copia su URL a `config.h`.
8. Utiliza `esp32/check_UID.ino` como sketch auxiliar para leer el UID de cada brazalete nuevo por el Monitor Serie y así darlo de alta en `BRACELET_USERS`.

> `sketch_apr21a/` contiene el prototipo inicial en Arduino clásico (lectura RFID local + validación de un único UID autorizado, sin conexión a Firebase), previo a la migración a ESP32 + WiFi + backend en la nube.

---

## 📂 Estructura del proyecto

```
tracesafe/
├── App.js                     # Punto de entrada de la app: enrutado según sesión Firebase Auth
├── index.js                   # Registro del componente raíz (Expo)
├── firebaseConfig.js          # Inicialización de Firebase (Auth, Firestore, Realtime Database)
├── screens/
│   ├── LoginScreen.js         # Pantalla de login (email/contraseña)
│   └── HomeScreen.js          # Pantalla principal: escucha de solicitudes y modal de autorización
├── esp32/
│   ├── esp32_tracesafe/
│   │   ├── esp32_tracesafe.ino  # Firmware principal (RFID + servo + LEDs + Firebase HTTP)
│   │   └── config.h             # WiFi, URL de Firebase, usuarios y pines
│   ├── check_UID.ino          # Sketch auxiliar para leer UIDs de brazaletes nuevos
│   ├── CONEXIONES.md          # Guía de cableado (variante ESP32-S3)
│   └── pin_esp32.txt          # Guía de cableado (variante ESP32 DevKitV1)
└── sketch_apr21a/
    └── sketch_apr21a.ino      # Prototipo inicial en Arduino clásico (sin cloud)
```

---

## 🎓 Contexto académico

TraceSafe es un proyecto desarrollado como práctica de la asignatura **TIC II**, con el objetivo de aplicar de forma integrada conceptos de sistemas embebidos, comunicación en red y desarrollo de aplicaciones móviles conectadas a servicios en la nube: un caso de uso real de **IoT de extremo a extremo** que combina hardware (ESP32, RFID, servomotor), backend gestionado (Firebase) y frontend móvil multiplataforma (React Native / Expo).
