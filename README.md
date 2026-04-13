# ESP32 IoT Monitoring System

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A production-grade real-time IoT monitoring system built with **ESP32 (ESP-IDF v5+)**. Features modular firmware architecture with WiFi provisioning, mock/real sensors, WebSocket streaming to 100+ clients, OTA updates, and a fully installable **PWA dashboard** with charts, multi-device support, alerts, and offline caching.

## 🚀 Features
- **Firmware**:
  - WiFi AP config portal + NVS storage
  - Sensor registry (mock temp/hum, extensible)
  - Async WebSocket broadcast (multi-client)
  - OTA HTTP updates (no esptool needed)
  - Reliability: Watchdog-safe tasks, heap monitoring
- **Dashboard (PWA)**:
  - Live metrics: Temp, Humidity, Uptime, RSSI, Heap, Timestamp
  - Historical charts (Chart.js)
  - Threshold alerts + browser notifications
  - Multi-device management + reconnect logic
  - Responsive UI, installable, service worker offline
  - Sidebar for saved devices

## 📋 Quick Start

### 1. Firmware (PlatformIO)
```bash
cd esp32_firmware
pio run --target upload --upload-port COM3  # Flash
pio run --target uploadfs  # SPIFFS for OTA
```
- Default AP: `IoT-Config` (no pass)
- Connect → `192.168.4.1` → Save WiFi creds

### 2. Dashboard
- Serve via VSCode Live Server or Python: `cd web_dashboard && python -m http.server 8000`
- Open `http://localhost:8000/index.html`
- Enter ESP32 IP (e.g., `192.168.1.100:80`) → **Connect**
- 💡 Install PWA (Chrome: addr bar + → Install)

### Test Flow
1. Flash firmware → Join AP → Configure WiFi
2. Note IP (serial monitor or nmap)
3. Dashboard → Connect → Live data/charts/alerts
4. OTA: Upload new `main.c` via dashboard `/update`

## 🏗️ Architecture

```
ESP32 Firmware:
[main.c] → Tasks → [WiFiMgr] [SensorMgr] [WSMgr] [OTAMgr]

WebSocket: ws://IP:81/ws → JSON {sensors: {env: {temp, hum}}, system: {uptime, rssi, heap}}

PWA Dashboard:
sw.js (offline) → app.js (WS reconnect) → modules (devices/charts/alerts)
```

## 📱 UI Polish Highlights
- Gradient backgrounds, animated cards w/ emojis
- Responsive grid/mobile sidebar
- Loading spinners, hover transforms
- Accessibility (ARIA), semantic HTML
- Fixed sidebar hides if no saved devices

## 🔧 Troubleshooting
- **No data?** Check IP/port 81 WS open, serial for errors
- **Reconnect fails?** 5 attempts w/ exponential backoff
- **PWA install?** HTTPS or localhost only
- Heap <20KB? Restart (watchdog safe)

## 📂 Structure
```
├── esp32_firmware/     # ESP-IDF + PlatformIO
│   ├── lib/*          # Modules: ConfigPortal, OTAHandler, etc.
│   └── src/main.c
├── web_dashboard/      # PWA static files
│   ├── index.html
│   ├── style.css
│   ├── app.js + modules/
│   └── sw.js
├── docs/              # Design notes
└── README.md          # This!
```

## 🤝 Contributing
Fork → Polish → PR to `main`. Tests: PlatformIO + browser devtools.

## 📄 License
MIT - See [LICENSE](LICENSE)
