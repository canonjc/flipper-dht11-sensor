# DHT11 Sensor App for Flipper Zero

A lightweight, interactive sensor reader for the DHT11 temperature and humidity module, designed specifically for the Flipper Zero running Momentum Firmware.

📦 Version: v0.1

✨ Features

Interactive menu UI (Acquire Data, Configuration, About, Exit)

Configuration for:

✅ GPIO pin (PB3, PB2, PC3)

✅ Units (Celsius or Fahrenheit)

✅ Read interval (1s, 1 min, 5 min)

Persistent settings saved to ext/dht11_sensor.cfg

Uses DWT-based microsecond timing for stable DHT11 reads

Graceful app exit with [Back]

📌 Installation

Copy the .fap to your apps/ directory under Momentum Firmware. Example:

applications_user/
└── dht11_sensor/
    ├── dht11_sensor.c
    ├── dht11_sensor_icon.png
    └── application.fam

Build with:

ufbt dht11_sensor

📐 Pinout (Flipper GPIO)

DHT11     Flipper GPIO
=====     ============
VCC   →   3V
GND   →   Pin 3 (GND)
DATA  →   Pin 5 (PB3) — default
           or Pin 6 (PB2), Pin 7 (PC3)

🧾 About

Designed by: Javier Canon

April 2025

Platform: Flipper Zero (Momentum Firmware)

License: GPLv3

GitHub: https://github.com/canonjc/flipper-dht11-sensor

💡 Roadmap

🔜 Upcoming Features

Show last sensor read status (OK, Timeout, CRC Error)

Display count of read failures

Add optional dark mode

Improve visual contrast in configuration menu

Configuration: allow custom read interval entry

Logging to file (CSV format on SD)

Sensor selection support (DHT11/DHT22)

🧪 Experimental Ideas

Graph view of recent readings (line chart)

QR code export for fast mobile import

Diagnostic test mode

🎨 ASCII Logo

  D . H . T . 1 . 1

Stay cool, stay dry! ☁️🌡️
