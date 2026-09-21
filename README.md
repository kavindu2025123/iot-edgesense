\# IoT EdgeSense



An end-to-end IoT monitoring and control system using ESP32, nRF24L01 wireless communication, MQTT, PostgreSQL, FastAPI, Docker, and React.



\## Overview



IoT EdgeSense is a smart home/environment monitoring system designed to collect sensor data from an ESP32 sensor node, transmit it wirelessly through an nRF24L01 module, and process the data through an IoT gateway.



The gateway forwards sensor data to an MQTT broker, where the backend stores it in PostgreSQL and provides REST APIs for the web dashboard.



The system also supports remote LED control and automatic light-based control.



\## System Architecture



```text

┌─────────────────────┐

│   ESP32 Sensor Node │

│                     │

│  LDR                │

│  DHT11              │

│  LED                │

└──────────┬──────────┘

&#x20;          │

&#x20;          │ nRF24L01

&#x20;          ▼

┌─────────────────────┐

│   ESP32 Gateway     │

│                     │

│ Wi-Fi + MQTT        │

│ Auto/Manual Control │

│ Offline Buffer      │

└──────────┬──────────┘

&#x20;          │

&#x20;          │ MQTT over TLS

&#x20;          ▼

┌─────────────────────┐

│    HiveMQ Cloud     │

│    MQTT Broker      │

└──────────┬──────────┘

&#x20;          │

&#x20;          ▼

┌─────────────────────┐

│    MQTT Service     │

│      Python         │

└──────────┬──────────┘

&#x20;          │

&#x20;          ▼

┌─────────────────────┐

│     PostgreSQL      │

│   sensor\_readings   │

└──────────┬──────────┘

&#x20;          │

&#x20;          ▼

┌─────────────────────┐

│      FastAPI        │

│     REST API        │

└──────────┬──────────┘

&#x20;          │

&#x20;          ▼

┌─────────────────────┐

│   React Dashboard   │

│                     │

│ Temperature         │

│ Humidity            │

│ Light Level         │

│ Device Status       │

│ Device Control      │

└─────────────────────┘

