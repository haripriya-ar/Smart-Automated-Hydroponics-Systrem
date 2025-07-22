# 🌱 Smart Hydroponics Monitoring & Control System (ESP32)

A fully automated **hydroponics system** using an **ESP32 DevKit v1**, designed to monitor and control essential parameters such as **temperature**, **humidity**, **pH level**, **water level**, and **nutrient dosing**. The system is integrated with **Firebase** for cloud logging and uses a **Telegram bot** for real-time alerts.

---

## 📸 System Overview

This project uses the following components:
- 🟦 ESP32 DevKit v1
- 🌡️ DHT11 (Temperature + Humidity)
- 💧 Ultrasonic Sensor (Water Level)
- 🔬 Analog pH Sensor + Temp Probe
- 🧠 Servo Motor (Nutrient Valve Control)
- ⚡ Relay Module (Pump Control)
- ☁️ Firebase Realtime Database
- 📲 Telegram Bot Integration

---

## 🚀 Features

- ✅ **Real-time sensor data** logging to Firebase
- 🔁 **Automatic water pumping** every 2 minutes
- 📏 **Ultrasonic sensor** for water level detection
- 🔬 **pH monitoring** with temperature correction
- 🧪 **Servo-based dosing** mechanism every 5 minutes
- 📡 **Telegram alerts** for low water or abnormal pH
- 🔌 Auto reconnection to Wi-Fi and Firebase
- 🔍 Serial debug logs for every operation

---

## 📡 Hardware Pin Connections (ESP32 DevKit v1)

| Component              | Pin (GPIO) |
|------------------------|------------|
| DHT11 Sensor           | 4          |
| Relay (Pump Control)   | 5 (D5)     |
| Ultrasonic TRIG        | 25         |
| Ultrasonic ECHO        | 26         |
| pH Sensor (Analog)     | 34 (ADC1)  |
| pH Temp Sensor (Analog)| 35 (ADC1)  |
| Servo Motor            | 13         |

> ⚠️ Note: GPIO34 and GPIO35 are **input-only** and used for analog reading.

---

## 🔧 Setup Instructions

1. **Clone the repo** or copy the code into Arduino IDE.
2. Install required libraries:
   - `Firebase_ESP_Client`
   - `UniversalTelegramBot`
   - `DHT sensor library`
   - `ESP32Servo`
3. Replace credentials in the code:
   - `Wi-Fi SSID` & `Password`
   - `Firebase API key & URL`
   - `Telegram Bot Token & Chat ID`
4. Upload the code to ESP32 using Arduino IDE.
5. Monitor output via **Serial Monitor** at **115200 baud**.

---

## 📊 Firebase Structure

```
/readings/
  └── timestamp/
        ├── temperature: 24.6
        ├── humidity: 45.2
        ├── ph: 6.3
        ├── water_level: 3.2
        └── ph_sensor_temp: 25.1
```

---

## 📲 Telegram Alerts

- 🔔 Alert when **water level < 2 cm**
- ⚠️ Alert when **pH < 6.0 or > 8.0**
- Messages are formatted in **HTML** with emojis

---

## 📷 Preview (Optional)
_Add system photos or screenshots here_

---

## 📘 License

This project is licensed under the **MIT License**.

```
MIT License

Copyright (c) 2025 [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy...
```

---

## 🙌 Credits

Developed by **[Your Name]** as part of a smart agriculture/hydroponics automation initiative.  
Special thanks to the open-source community.

---

## 🤝 Contributing

Pull requests and feedback are welcome. If you find bugs or have feature requests, open an issue or fork the repo!

---
