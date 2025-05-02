# DHT11 Sensor App for Flipper Zero

A lightweight, interactive sensor reader for the DHT11 temperature and humidity module, designed specifically for the Flipper Zero running Momentum Firmware.

---

## 📦 Version: **v0.1**

### ✨ Features

- Interactive menu UI (Acquire Data, Configuration, About, Exit)
- Configuration for:
  - ✅ GPIO pin (PB3, PB2, PC3)
  - ✅ Units (Celsius or Fahrenheit)
  - ✅ Read interval (1s, 1 min, 5 min)
- Persistent settings saved to `ext/dht11_sensor.cfg`
- Uses DWT-based microsecond timing for stable DHT11 reads
- Graceful app exit with `[Back]`

---

## 📌 Installation

Copy the `.fap` to your `apps/` directory under Momentum Firmware. Example:

```
applications_user/
└── dht11_sensor/
    ├── dht11_sensor_menu_v0_1.c
    ├── dht11_sensor.png
    └── application.fam
```

Build with:

```
ufbt dht11_sensor
```

---

## 📐 Pinout (Flipper GPIO)

```
DHT11     Flipper GPIO
=====     ============
VCC   →   3V
GND   →   Pin 3 (GND)
DATA  →   Pin 5 (PB3) — default
           or Pin 6 (PB2), Pin 7 (PC3)
```



---

## 🧾 About

Designed by: **Javier Canon**\
Platform: **Flipper Zero (Momentum Firmware)**\
License: **GPLv3**\
GitHub: *[https://github.com/canonjc/flipper-dht11-sensor](https://github.com/canonjc/flipper-dht11-sensor)*

---

## 💡 Roadmap

See the `dht11_roadmap` document for planned improvements.

---

## 🎨 ASCII Logo

```
  D . H . T . 1 . 1
```

Stay cool, stay dry! ☁️🌡️

