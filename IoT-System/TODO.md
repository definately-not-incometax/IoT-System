# Complete IoT Setup

✅ **UI**: Dark Apple theme (web_dashboard)
✅ **Firmware**: WiFi hardcoded STA/AP 'Ruti Torkarii'/'8274808564' (wifi_manager.c, common.h)

**Flash ESP32 COM4**:
1. pip install platformio
2. cd "../esp32_firmware" && pio run --target upload --upload-port COM4
3. pio device monitor (note IP)

**Test**:
- Dashboard localhost:8000 → Enter IP → Connect
- Metrics/charts live
