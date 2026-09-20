# IoT EdgeSense
### End-to-End Environmental Monitoring & Control Platform

IoT EdgeSense is an end-to-end IoT platform for real-time environmental monitoring and remote device control.

The system collects temperature, humidity, and light-level data using an ESP32-based sensor node. Sensor data is transmitted wirelessly using an nRF24L01 module to an ESP32 gateway, which forwards the data to the cloud through MQTT.

A Python data service stores the incoming data in PostgreSQL, while a FastAPI backend provides REST APIs for the React-based web dashboard.

The dashboard provides real-time monitoring, historical data visualization, gateway status, packet-loss statistics, and remote LED control.

---

## Features

### Environmental Monitoring

- Real-time temperature monitoring
- Real-time humidity monitoring
- Light-level monitoring using an LDR
- Historical sensor data
- Interactive monitoring charts

### Wireless Communication

- ESP32 sensor node
- ESP32 gateway
- nRF24L01 wireless communication
- Automatic acknowledgement and retransmission
- Packet-loss detection
- Packet statistics

### IoT Communication

- MQTT-based communication
- HiveMQ Cloud
- TLS-secured MQTT connection
- MQTT sensor data publishing
- MQTT device command handling
- Automatic Wi-Fi reconnection
- MQTT reconnection handling

### Device Control

- AUTO operating mode
- MANUAL operating mode
- Remote LED ON/OFF control
- Automatic light-based LED control
- React dashboard → FastAPI → MQTT → ESP32 gateway → nRF24L01 → sensor node

### Reliability

- nRF24 automatic retransmission
- Packet-loss monitoring
- Offline sensor-data buffering
- Wi-Fi reconnection
- MQTT reconnection
- Gateway heartbeat monitoring
- Gateway online/offline detection

### Data Pipeline

- MQTT subscriber service
- CSV data storage
- PostgreSQL database
- FastAPI REST API
- Historical data retrieval
- Time-range filtering

### Web Dashboard

- Real-time sensor cards
- Gateway status
- Operating mode status
- LED status
- Packet statistics
- Historical temperature chart
- Historical humidity chart
- Historical light-level chart
- Device control interface
- Responsive dashboard UI

---

# System Architecture

```text
                    ┌──────────────────────┐
                    │    ESP32 Sensor      │
                    │        Node          │
                    │                      │
                    │  DHT11               │
                    │  LDR                 │
                    │  LED                 │
                    └──────────┬───────────┘
                               │
                         nRF24L01
                               │
                               ▼
                    ┌──────────────────────┐
                    │    ESP32 Gateway     │
                    │                      │
                    │ Wi-Fi                │
                    │ MQTT                 │
                    │ Device Control       │
                    │ Offline Buffer       │
                    └──────────┬───────────┘
                               │
                         MQTT / TLS
                               │
                               ▼
                    ┌──────────────────────┐
                    │     HiveMQ Cloud     │
                    │      MQTT Broker     │
                    └──────────┬───────────┘
                               │
                               ▼
                    ┌──────────────────────┐
                    │   MQTT Subscriber    │
                    │      Python          │
                    │                      │
                    │ MQTT → CSV           │
                    │ MQTT → PostgreSQL    │
                    └──────────┬───────────┘
                               │
                               ▼
                    ┌──────────────────────┐
                    │     PostgreSQL       │
                    │       Database       │
                    └──────────┬───────────┘
                               │
                               ▼
                    ┌──────────────────────┐
                    │      FastAPI         │
                    │     REST API         │
                    └──────────┬───────────┘
                               │
                              HTTP
                               │
                               ▼
                    ┌──────────────────────┐
                    │    React Dashboard   │
                    │                      │
                    │ Monitoring            │
                    │ Charts                │
                    │ Device Control        │
                    │ System Status         │
                    └──────────────────────┘
