# IoT EdgeSense

### End-to-End Environmental Monitoring & Control Platform

IoT EdgeSense is an end-to-end IoT platform for real-time environmental monitoring and remote device control.

The system collects temperature, humidity, and light-level data using an ESP32-based sensor node. Sensor data is transmitted wirelessly using an nRF24L01 module to an ESP32 gateway.

The gateway connects to Wi-Fi and publishes the sensor data to a cloud MQTT broker using MQTT over TLS. A containerized Python MQTT service receives the data and stores it in PostgreSQL.

A FastAPI backend provides REST APIs for sensor data, device status, and device control. A React-based web dashboard provides real-time monitoring, historical data visualization, system status information, and remote LED control.

The backend infrastructure is containerized using Docker and Docker Compose.

---

## ✨ Features

### Environmental Monitoring

* Real-time temperature monitoring
* Real-time humidity monitoring
* Light-level monitoring using an LDR
* Historical sensor data
* Time-range filtering
* Interactive sensor charts

### Wireless Communication

* ESP32 sensor node
* ESP32 gateway
* nRF24L01 wireless communication
* Automatic acknowledgement
* RF retransmission
* Packet-loss detection
* Packet statistics

### IoT Communication

* MQTT communication
* HiveMQ Cloud MQTT broker
* MQTT over TLS
* Sensor data publishing
* Device command handling
* Wi-Fi reconnection
* MQTT reconnection

### Device Control

* AUTO operating mode
* MANUAL operating mode
* Remote LED ON/OFF control
* Automatic light-based LED control
* Dashboard-based device control

### Reliability

* nRF24 automatic retransmission
* Packet-loss monitoring
* Offline sensor-data buffering
* Wi-Fi reconnection
* MQTT reconnection
* Gateway heartbeat monitoring
* Gateway online/offline detection

### Backend

* Python MQTT subscriber
* CSV data logging
* PostgreSQL data storage
* FastAPI REST API
* Sensor history API
* Device status API
* Device control API

### Web Dashboard

* Real-time sensor cards
* Gateway online/offline indicator
* Operating mode indicator
* LED status
* Packet statistics
* Temperature history
* Humidity history
* Light-level history
* Time-range filtering
* Device control interface
* Responsive UI

### Containerization

The backend services are containerized using Docker.

Docker Compose manages:

* PostgreSQL database
* MQTT data service
* FastAPI API service
* Persistent PostgreSQL storage
* Service networking

---

# 🏗️ System Architecture

```text
                    ┌──────────────────────┐
                    │    ESP32 Sensor      │
                    │        Node          │
                    │                      │
                    │  DHT11               │
                    │  LDR                 │
                    │  LED                 │
                    │  nRF24L01            │
                    └──────────┬───────────┘
                               │
                         nRF24L01
                               │
                               ▼
                    ┌──────────────────────┐
                    │    ESP32 Gateway     │
                    │                      │
                    │  Wi-Fi               │
                    │  MQTT                │
                    │  Device Control      │
                    │  Offline Buffer      │
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
              ┌────────────────────────────────┐
              │          Docker Host            │
              │                                │
              │  ┌──────────────────────────┐  │
              │  │    MQTT Service          │  │
              │  │       Python             │  │
              │  └────────────┬─────────────┘  │
              │               │                │
              │               ▼                │
              │  ┌──────────────────────────┐  │
              │  │      PostgreSQL          │  │
              │  │        Database          │  │
              │  └────────────┬─────────────┘  │
              │               │                │
              │               ▼                │
              │  ┌──────────────────────────┐  │
              │  │        FastAPI           │  │
              │  │        REST API          │  │
              │  └────────────┬─────────────┘  │
              │               │                │
              └───────────────┼────────────────┘
                              │
                             HTTP
                              │
                              ▼
                    ┌──────────────────────┐
                    │    React Dashboard   │
                    │                      │
                    │  Monitoring          │
                    │  Charts              │
                    │  Device Control      │
                    │  System Status       │
                    └──────────────────────┘
```

---

# 🔄 Data Flow

## Sensor Data

```text
DHT11 + LDR
     │
     ▼
ESP32 Sensor Node
     │
     ▼
nRF24L01
     │
     ▼
ESP32 Gateway
     │
     ▼
MQTT / TLS
     │
     ▼
HiveMQ Cloud
     │
     ▼
Python MQTT Service
     │
     ├──────────────► CSV
     │
     ▼
PostgreSQL
     │
     ▼
FastAPI
     │
     ▼
React Dashboard
```

## Device Control

```text
React Dashboard
       │
       ▼
FastAPI
       │
       ▼
MQTT Command
       │
       ▼
HiveMQ Cloud
       │
       ▼
ESP32 Gateway
       │
       ▼
nRF24L01
       │
       ▼
ESP32 Sensor Node
       │
       ▼
LED
```

---

# 🐳 Docker Architecture

The backend infrastructure uses Docker Compose.

```text
                 Docker Compose
                       │
       ┌───────────────┼────────────────┐
       │               │                │
       ▼               ▼                ▼
┌─────────────┐ ┌─────────────┐ ┌─────────────┐
│ MQTT Service│ │ PostgreSQL  │ │   FastAPI   │
│             │ │             │ │     API     │
│ Python      │ │ Database    │ │ REST API    │
│ MQTT        │ │             │ │             │
└──────┬──────┘ └──────┬──────┘ └──────┬──────┘
       │               │                │
       └───────────────┼────────────────┘
                       │
                  Docker Network
```

### Docker Services

| Service        | Purpose              | Container          |
| -------------- | -------------------- | ------------------ |
| `postgres`     | Sensor database      | `iot-postgres`     |
| `mqtt-service` | MQTT data processing | `iot-mqtt-service` |
| `api`          | REST API             | `iot-api`          |

PostgreSQL data is stored using a Docker volume so that database data remains available when containers are recreated.

---

# 🗄️ Database

The project uses PostgreSQL for persistent sensor data storage.

The main sensor table stores:

| Field         | Description              |
| ------------- | ------------------------ |
| `id`          | Database record ID       |
| `node`        | Sensor node identifier   |
| `packet`      | Packet ID                |
| `light`       | LDR light reading        |
| `temperature` | Temperature              |
| `humidity`    | Humidity                 |
| `timestamp`   | Sensor reading timestamp |

---

# 📡 MQTT Topics

The project uses the following MQTT topics:

```text
home/sensors/node1/data
home/sensors/node1/status
home/sensors/node1/command
```

### Sensor Data

Example:

```json
{
  "node": "NODE1",
  "packet": 183,
  "light": 2147,
  "temperature": 26.5,
  "humidity": 80,
  "timestamp": "2026-09-20T22:00:00"
}
```

### Device Status

Example:

```json
{
  "online": true,
  "mode": "AUTO",
  "led": "OFF",
  "light": 1958,
  "packet": 183,
  "total_packets": 183,
  "missing_packets": 0
}
```

---

# ⚙️ Operating Modes

## AUTO Mode

In AUTO mode, the gateway automatically controls the LED based on the measured light level.

```text
Light Level
     │
     ▼
Gateway
     │
     ├── Dark  ──────► LED ON
     │
     └── Bright ─────► LED OFF
```

The light thresholds are configured in the ESP32 gateway firmware.

---

## MANUAL Mode

In MANUAL mode, the user controls the LED from the web dashboard.

```text
React Dashboard
       │
       ├── LED ON
       │
       └── LED OFF
```

The command travels through FastAPI, MQTT, the ESP32 gateway, and nRF24L01 before reaching the sensor node.

---

# 📦 Offline Data Buffer

The ESP32 gateway contains an offline sensor-data buffer.

When MQTT connectivity is unavailable:

```text
Sensor
   │
   ▼
ESP32 Gateway
   │
   X MQTT unavailable
   │
   ▼
Offline Buffer
```

When the connection is restored:

```text
Offline Buffer
      │
      ▼
MQTT
      │
      ▼
HiveMQ Cloud
```

The current gateway implementation supports buffering up to 20 sensor packets.

---

# 📊 Packet Loss Detection

Each wireless sensor packet contains a packet ID.

Example:

```text
Packet 101
Packet 102
Packet 103
Packet 105
```

Packet `104` was not received.

The gateway detects the missing packet and updates the packet-loss counter.

The dashboard can therefore display:

```text
Total Packets:    183
Missing Packets:    1
```

---

# 🔌 Hardware

## Sensor Node

| Component | Purpose                |
| --------- | ---------------------- |
| ESP32     | Main microcontroller   |
| DHT11     | Temperature & humidity |
| LDR       | Light sensing          |
| LED       | Controlled output      |
| nRF24L01+ | Wireless communication |

## Gateway

| Component | Purpose                |
| --------- | ---------------------- |
| ESP32     | Gateway controller     |
| nRF24L01+ | Wireless communication |
| Wi-Fi     | Internet connectivity  |
| MQTT      | Cloud communication    |

---

# 📍 Pin Configuration

## ESP32 Sensor Node

| Component | ESP32 Pin |
| --------- | --------- |
| LDR       | GPIO 34   |
| DHT11     | GPIO 4    |
| LED       | GPIO 2    |
| nRF24 CE  | GPIO 16   |
| nRF24 CSN | GPIO 5    |

---

# 🧰 Technology Stack

### Embedded Systems

* ESP32
* Arduino Framework
* C/C++
* nRF24L01+
* DHT11
* LDR

### Communication

* Wi-Fi
* MQTT
* HiveMQ Cloud
* MQTT over TLS
* nRF24L01 wireless communication

### Backend

* Python
* FastAPI
* Paho MQTT
* PostgreSQL

### Frontend

* React
* Vite
* JavaScript
* Recharts
* CSS

### DevOps / Infrastructure

* Docker
* Docker Compose
* Docker Volumes
* Container Networking

---

# 📁 Project Structure

```text
iot-edgesense/
│
├── hardware/
│   ├── sensor-node/
│   │   └── sensor-node.ino
│   │
│   └── gateway/
│       └── gateway.ino
│
├── backend/
│   ├── mqtt-service/
│   │   ├── mqtt_subscriber.py
│   │   ├── requirements.txt
│   │   └── Dockerfile
│   │
│   └── api/
│       ├── main.py
│       ├── requirements.txt
│       └── Dockerfile
│
├── database/
│   └── schema.sql
│
├── dashboard/
│   ├── src/
│   │   ├── App.jsx
│   │   ├── App.css
│   │   └── ...
│   ├── package.json
│   └── vite.config.js
│
├── docs/
│   ├── architecture.png
│   ├── system-flow.png
│   └── screenshots/
│
├── data/
│   └── .gitkeep
│
├── docker-compose.yml
├── .env.example
├── .gitignore
├── LICENSE
└── README.md
```

---

# 🚀 Installation & Setup

## Prerequisites

Install the following:

* Git
* Docker Desktop
* Node.js
* Arduino IDE
* ESP32 board support for Arduino IDE

---

## 1. Clone the Repository

```bash
git clone https://github.com/YOUR_USERNAME/iot-edgesense.git

cd iot-edgesense
```

---

## 2. Configure Environment Variables

Create a `.env` file from `.env.example`.

```bash
copy .env.example .env
```

Then configure:

```env
MQTT_BROKER=your-hivemq-broker
MQTT_PORT=8883
MQTT_USERNAME=your-username
MQTT_PASSWORD=your-password

POSTGRES_HOST=postgres
POSTGRES_PORT=5432
POSTGRES_DB=iot_database
POSTGRES_USER=postgres
POSTGRES_PASSWORD=your-password
```

Never commit the real `.env` file.

---

# 🐳 3. Start the Docker Infrastructure

From the project root:

```bash
docker compose up --build
```

This starts the backend infrastructure.

To run in the background:

```bash
docker compose up --build -d
```

Check the containers:

```bash
docker compose ps
```

View logs:

```bash
docker compose logs
```

View individual service logs:

```bash
docker compose logs mqtt-service
docker compose logs api
docker compose logs postgres
```

Stop the system:

```bash
docker compose down
```

Stop containers without removing persistent database data:

```bash
docker compose down
```

---

# 🌐 4. Start the React Dashboard

Open another terminal:

```bash
cd dashboard
```

Install dependencies:

```bash
npm install
```

Start the development server:

```bash
npm run dev
```

The dashboard will normally be available at:

```text
http://localhost:5173
```

---

# 🔌 5. Configure the ESP32 Devices

Open the firmware projects:

```text
hardware/sensor-node/
hardware/gateway/
```

Configure the required:

* Wi-Fi credentials
* MQTT broker
* MQTT username
* MQTT password
* MQTT topics

Upload the firmware to the corresponding ESP32 boards.

---

# 🔐 Security

Never commit sensitive credentials to GitHub.

Do NOT upload:

```text
.env
Wi-Fi passwords
MQTT passwords
API keys
Private keys
TLS certificates containing secrets
Database passwords
```

Use environment variables whenever possible.

For example:

```text
.env
.env.example
```

`.env.example` should contain only placeholder values.

> **Important:** If real credentials have previously been placed directly in the source code, rotate those credentials before publishing the repository.

---

# 🧪 Development

The project can be developed and tested in separate layers.

```text
Hardware
   │
   ▼
Wireless Communication
   │
   ▼
MQTT
   │
   ▼
Docker Backend
   │
   ▼
Database
   │
   ▼
FastAPI
   │
   ▼
React Dashboard
```

This makes it possible to troubleshoot each layer independently.

---

# 📈 Future Improvements

* [ ] Real-time chart updates
* [ ] Min/Max/Average analytics
* [ ] Alert and notification system
* [ ] CSV data export
* [ ] Multiple sensor nodes
* [ ] User authentication
* [ ] Role-based device control
* [ ] Additional environmental sensors
* [ ] Mobile dashboard
* [ ] Cloud deployment
* [ ] Advanced device management
* [ ] Sensor anomaly detection

---

# 🎓 Learning Objectives

This project was developed to gain practical experience in:

* Embedded systems
* ESP32 development
* Wireless communication
* MQTT
* IoT architecture
* Cloud-connected devices
* REST API development
* Database design
* Docker
* Docker Compose
* React
* Real-time monitoring
* Remote device control
* Fault detection
* Network reliability

---

# 👨‍💻 Author

**Kavindu Gamhatha**

BSc Honours in Electronics & Information Technology
Faculty of Science, University of Colombo

---

# 📄 License

This project is licensed under the MIT License.

See the `LICENSE` file for details.
