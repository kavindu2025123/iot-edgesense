# IoT EdgeSense

### End-to-End IoT Environmental Monitoring & Device Control Platform

IoT EdgeSense is an end-to-end IoT platform designed for **real-time environmental monitoring, wireless sensor communication, data persistence, analytics, and remote device control**.

The system uses an **ESP32-based sensor node** to collect temperature, humidity, and light-level data. The sensor node communicates wirelessly with an **ESP32 gateway using nRF24L01**.

The gateway connects to Wi-Fi and publishes sensor data to **HiveMQ Cloud using MQTT over TLS**. A containerized Python MQTT service receives the data and stores it in PostgreSQL. A **FastAPI backend** exposes REST APIs for sensor data, device status, and device control, while a **React-based web dashboard** provides real-time monitoring and historical visualization.

The complete backend infrastructure is containerized using **Docker and Docker Compose**.

---

## 📸 Project Preview

> Add your dashboard and hardware photos here after capturing them.

### Dashboard

![IoT EdgeSense Dashboard](docs/screenshots/dashboard-overview.png)

### Hardware Setup

![IoT EdgeSense Hardware](docs/screenshots/hardware-setup.jpg)

---

# ✨ Key Features

## 🌡️ Environmental Monitoring

* Real-time temperature monitoring
* Real-time humidity monitoring
* Light-level monitoring using an LDR
* Historical sensor data storage
* 1H / 6H / 24H / All time-range filtering
* Interactive sensor charts
* Dashboard-based sensor visualization
* Efficient chart rendering through data downsampling

---

## 📡 Wireless Communication

* ESP32 sensor node
* ESP32 gateway
* nRF24L01+ wireless communication
* 250 Kbps RF communication
* Automatic acknowledgement
* RF retransmission
* Configurable retry mechanism
* Packet ID tracking
* Packet-loss detection
* Total packet statistics
* Missing packet statistics
* CRC16 error checking

---

## ☁️ IoT & MQTT Communication

* MQTT-based IoT communication
* HiveMQ Cloud MQTT broker
* MQTT over TLS
* QoS-based message publishing
* Sensor data publishing
* Device status publishing
* Remote command handling
* MQTT reconnection
* Wi-Fi reconnection
* Gateway heartbeat monitoring
* Online/offline device detection

---

## 🎛️ Device Control

* AUTO operating mode
* MANUAL operating mode
* Remote LED ON/OFF control
* Automatic light-based LED control
* Dashboard-based device control
* MQTT command communication
* nRF24-based command delivery to the sensor node

---

## 🛡️ Reliability

The system includes several mechanisms designed to improve reliability:

* nRF24 automatic retransmission
* Packet-loss detection
* Packet sequence tracking
* Offline sensor-data buffering
* Wi-Fi reconnection
* MQTT reconnection
* Gateway heartbeat messages
* Device online/offline detection
* Command retry mechanism
* Persistent PostgreSQL storage

The gateway currently supports buffering up to **20 sensor packets** when MQTT connectivity is unavailable.

---

# 🏗️ System Architecture

```text
                    ┌─────────────────────────┐
                    │     ESP32 Sensor Node   │
                    │                         │
                    │  ┌───────┐  ┌────────┐ │
                    │  │ DHT11 │  │  LDR   │ │
                    │  └───┬───┘  └───┬────┘ │
                    │      │           │      │
                    │      └─────┬─────┘      │
                    │            │            │
                    │       Sensor Data       │
                    │            │            │
                    │       nRF24L01+         │
                    └────────────┬────────────┘
                                 │
                                 │  Wireless
                                 ▼
                    ┌─────────────────────────┐
                    │     ESP32 Gateway       │
                    │                         │
                    │      nRF24L01+          │
                    │           │             │
                    │     Packet Processing   │
                    │           │             │
                    │     Device Control      │
                    │           │             │
                    │     Offline Buffer      │
                    │           │             │
                    │          Wi-Fi          │
                    └────────────┬────────────┘
                                 │
                                 │ MQTT / TLS
                                 ▼
                    ┌─────────────────────────┐
                    │      HiveMQ Cloud       │
                    │       MQTT Broker       │
                    └────────────┬────────────┘
                                 │
                                 │ MQTT
                                 ▼
              ┌────────────────────────────────────┐
              │            Docker Host             │
              │                                    │
              │   ┌────────────────────────────┐   │
              │   │      MQTT Service          │   │
              │   │          Python            │   │
              │   │                            │   │
              │   │   MQTT → Data Processing   │   │
              │   └─────────────┬──────────────┘   │
              │                 │                  │
              │                 ▼                  │
              │   ┌────────────────────────────┐   │
              │   │       PostgreSQL           │   │
              │   │         Database           │   │
              │   └─────────────┬──────────────┘   │
              │                 │                  │
              │                 ▼                  │
              │   ┌────────────────────────────┐   │
              │   │         FastAPI            │   │
              │   │        REST API            │   │
              │   └─────────────┬──────────────┘   │
              │                 │                  │
              └─────────────────┼──────────────────┘
                                │
                               HTTP
                                │
                                ▼
                    ┌─────────────────────────┐
                    │     React Dashboard     │
                    │                         │
                    │  • Real-Time Monitoring │
                    │  • Historical Charts    │
                    │  • Device Status        │
                    │  • Device Control       │
                    └─────────────────────────┘
```

---

# 🔄 Data Flow

## Sensor Data Flow

```text
DHT11 + LDR
     │
     ▼
ESP32 Sensor Node
     │
     ▼
nRF24L01+
     │
     ▼
ESP32 Gateway
     │
     ▼
Wi-Fi
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
     ├──────────────► CSV Logging
     │
     ▼
PostgreSQL
     │
     ▼
FastAPI REST API
     │
     ▼
React Dashboard
```

---

## Device Control Flow

The platform also supports two-way communication.

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
nRF24L01+
       │
       ▼
ESP32 Sensor Node
       │
       ▼
LED
```

This allows a command generated from the web dashboard to reach the physical IoT device.

---

# 📊 Web Dashboard

The React dashboard provides a centralized interface for monitoring and controlling the IoT system.

### Dashboard capabilities

* Current temperature
* Current humidity
* Current light level
* Gateway connection status
* Operating mode
* LED status
* Last received packet
* Total packets
* Missing packets
* Temperature history
* Humidity history
* Light-level history
* Time-range filtering
* Remote device control

### Supported History Ranges

```text
1H
6H
24H
All
```

Historical sensor data is retrieved from the FastAPI backend and efficiently processed before visualization.

For larger datasets, the dashboard performs **data downsampling** so that charts remain responsive while preserving the overall sensor trends.

### Dashboard Screenshot

![Dashboard Overview](docs/screenshots/dashboard-overview.png)

---

# 🎛️ Device Control

IoT EdgeSense supports both automatic and manual device control.

## AUTO Mode

In AUTO mode, the ESP32 gateway makes the LED control decision based on the measured light level.

```text
             Light Sensor
                  │
                  ▼
             ESP32 Gateway
                  │
          ┌───────┴────────┐
          │                │
       Dark             Bright
          │                │
          ▼                ▼
       LED ON           LED OFF
```

The light thresholds are configured in the gateway firmware.

---

## MANUAL Mode

In MANUAL mode, the user can control the LED from the web dashboard.

```text
Dashboard
    │
    ├────────► LED ON
    │
    └────────► LED OFF
```

The command is transmitted through:

```text
React
  ↓
FastAPI
  ↓
MQTT
  ↓
HiveMQ Cloud
  ↓
ESP32 Gateway
  ↓
nRF24L01+
  ↓
ESP32 Sensor Node
  ↓
LED
```

### Device Control Screenshot

![Device Control](docs/screenshots/device-control.png)

---

# 📡 MQTT Architecture

The gateway communicates with HiveMQ Cloud using MQTT over TLS.

## MQTT Topics

```text
home/sensors/node1/data
home/sensors/node1/status
home/sensors/node1/command
```

### Sensor Data Topic

```text
home/sensors/node1/data
```

Example:

```json
{
  "node": 1,
  "packet": 183,
  "light": 2147,
  "temperature": 26.5,
  "humidity": 80,
  "timestamp": "2026-09-20T22:00:00"
}
```

### Device Status Topic

```text
home/sensors/node1/status
```

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

### Command Topic

```text
home/sensors/node1/command
```

Example commands:

```text
AUTO
MANUAL
LED_ON
LED_OFF
```

---

# 💓 Gateway Heartbeat & Device Status

The ESP32 gateway periodically publishes its current status.

The status information includes:

```text
Online / Offline
Operating Mode
LED State
Current Light Level
Last Packet
Total Packets
Missing Packets
```

The FastAPI backend listens to the status topic and maintains the latest device state.

If status updates are not received within the configured timeout period, the backend reports the gateway as offline.

This allows the React dashboard to display the current connection state of the IoT system.

---

# 📦 Offline Data Buffering

The gateway includes an offline buffering mechanism to prevent immediate data loss during MQTT connectivity problems.

### When MQTT is unavailable

```text
Sensor Node
     │
     ▼
ESP32 Gateway
     │
     X
 MQTT Unavailable
     │
     ▼
Offline Buffer
```

The gateway temporarily stores sensor packets.

### When MQTT reconnects

```text
Offline Buffer
      │
      ▼
MQTT Connection Restored
      │
      ▼
HiveMQ Cloud
      │
      ▼
Backend
      │
      ▼
PostgreSQL
```

The current implementation supports a buffer of up to:

```text
20 packets
```

---

# 📡 Packet Loss Detection

Each sensor packet contains a sequential packet ID.

For example:

```text
Packet 101
Packet 102
Packet 103
Packet 105
```

The gateway detects that packet `104` was not received.

The packet-loss counter is then updated.

Example dashboard information:

```text
Total Packets:     183
Missing Packets:     1
```

This provides basic wireless communication reliability monitoring.

---

# 🐳 Docker Architecture

The backend infrastructure is containerized using Docker Compose.

```text
                     Docker Compose
                           │
          ┌────────────────┼────────────────┐
          │                │                │
          ▼                ▼                ▼
 ┌────────────────┐ ┌───────────────┐ ┌───────────────┐
 │  MQTT Service  │ │  PostgreSQL   │ │    FastAPI    │
 │                │ │               │ │      API      │
 │ Python         │ │ Sensor Data   │ │ REST API      │
 │ MQTT Subscriber│ │ Storage       │ │ Device Control│
 └────────────────┘ └───────────────┘ └───────────────┘
          │                │                │
          └────────────────┼────────────────┘
                           │
                    Docker Network
```

## Docker Services

| Service        | Purpose                     | Container          |
| -------------- | --------------------------- | ------------------ |
| `postgres`     | Persistent sensor database  | `iot-postgres`     |
| `mqtt-service` | MQTT data processing        | `iot-mqtt-service` |
| `api`          | REST API and device control | `iot-api`          |

PostgreSQL uses a Docker volume for persistent database storage.

---

# 🗄️ Database

PostgreSQL is used for persistent storage of sensor readings.

## Sensor Table

The main `sensor_readings` table contains:

| Field         | Description              |
| ------------- | ------------------------ |
| `id`          | Database record ID       |
| `timestamp`   | Sensor reading timestamp |
| `node`        | Sensor node identifier   |
| `packet`      | Packet ID                |
| `light`       | LDR reading              |
| `temperature` | Temperature reading      |
| `humidity`    | Humidity reading         |

The database allows the system to maintain historical sensor data for analysis and visualization.

---

# 🔌 Hardware

## Sensor Node

| Component | Purpose                          |
| --------- | -------------------------------- |
| ESP32     | Main microcontroller             |
| DHT11     | Temperature and humidity sensing |
| LDR       | Light-level sensing              |
| LED       | Controlled output                |
| nRF24L01+ | Wireless communication           |

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
| --------- | --------: |
| LDR       |   GPIO 34 |
| DHT11     |    GPIO 4 |
| LED       |    GPIO 2 |
| nRF24 CE  |   GPIO 16 |
| nRF24 CSN |    GPIO 5 |

## ESP32 Gateway

| Component | ESP32 Pin |
| --------- | --------: |
| nRF24 CE  |   GPIO 16 |
| nRF24 CSN |    GPIO 5 |
| SPI SCK   |   GPIO 18 |
| SPI MISO  |   GPIO 19 |
| SPI MOSI  |   GPIO 23 |

---

# 🧰 Technology Stack

## Embedded Systems

* ESP32
* Arduino Framework
* C/C++
* nRF24L01+
* DHT11
* LDR

## Wireless & IoT Communication

* nRF24L01+
* SPI
* Wi-Fi
* MQTT
* HiveMQ Cloud
* MQTT over TLS

## Backend

* Python
* FastAPI
* Paho MQTT
* PostgreSQL
* REST APIs

## Frontend

* React
* Vite
* JavaScript
* Recharts
* CSS

## DevOps & Infrastructure

* Docker
* Docker Compose
* Docker Volumes
* Docker Networking
* Environment Variables

---

# 🔗 REST API

The FastAPI backend provides endpoints for sensor data and device control.

## Health Check

```http
GET /
```

## Latest Sensor Reading

```http
GET /api/sensors/latest
```

Returns the latest stored sensor reading.

## Sensor History

```http
GET /api/sensors/history?hours=1
```

Supported examples:

```text
/api/sensors/history?hours=1
/api/sensors/history?hours=6
/api/sensors/history?hours=24
```

For all available stored data:

```http
GET /api/sensors/history?hours=0
```

## Device Status

```http
GET /api/device/status
```

Returns the latest gateway/device status.

## LED Control

Turn LED on:

```http
POST /api/device/led/ON
```

Turn LED off:

```http
POST /api/device/led/OFF
```

## Operating Mode

AUTO:

```http
POST /api/device/mode/AUTO
```

MANUAL:

```http
POST /api/device/mode/MANUAL
```

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
│       ├── dashboard-overview.png
│       ├── hardware-setup.jpg
│       ├── sensor-node.jpg
│       ├── gateway.jpg
│       ├── device-control.png
│       └── api-response.png
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

> Update the structure above if your actual repository uses different folder names.

---

# 🚀 Installation & Setup

## Prerequisites

Install:

* Git
* Docker Desktop
* Node.js
* npm
* Arduino IDE
* ESP32 board support for Arduino IDE

---

## 1. Clone the Repository

```bash
git clone https://github.com/kavindu2025123/iot-edgesense.git
cd iot-edgesense
```

---

## 2. Configure Environment Variables

Create a local `.env` file from `.env.example`.

### Windows PowerShell

```powershell
Copy-Item .env.example .env
```

Configure the required values:

```env
MQTT_BROKER=your-hivemq-broker
MQTT_PORT=8883
MQTT_USERNAME=your-username
MQTT_PASSWORD=your-password

DB_HOST=postgres
DB_PORT=5432
DB_NAME=iot_database
DB_USER=postgres
DB_PASSWORD=your-database-password
```

Use your actual credentials only in the local `.env` file.

**Never commit the real `.env` file to GitHub.**

---

# 🐳 3. Start the Backend

From the project root:

```bash
docker compose up --build
```

Or run in detached mode:

```bash
docker compose up --build -d
```

Check running services:

```bash
docker compose ps
```

View all logs:

```bash
docker compose logs
```

View MQTT service logs:

```bash
docker compose logs mqtt-service
```

View API logs:

```bash
docker compose logs api
```

View PostgreSQL logs:

```bash
docker compose logs postgres
```

Stop the services:

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

Open the corresponding firmware projects:

```text
hardware/sensor-node/
hardware/gateway/
```

Configure the required device/network settings.

The gateway requires:

* Wi-Fi configuration
* MQTT broker configuration
* MQTT username
* MQTT password

Upload the appropriate firmware to each ESP32.

---

# 🧪 Testing the System

The complete system can be tested layer by layer.

### Hardware Layer

Verify:

```text
DHT11 → Temperature/Humidity
LDR  → Light
LED  → Device Control
```

### Wireless Layer

Verify:

```text
ESP32 Sensor → nRF24L01 → ESP32 Gateway
```

### MQTT Layer

Verify:

```text
ESP32 Gateway → HiveMQ Cloud
```

### Backend Layer

Verify:

```text
MQTT → Python Service → PostgreSQL
```

### API Layer

Verify:

```text
PostgreSQL → FastAPI
```

### Dashboard Layer

Verify:

```text
FastAPI → React Dashboard
```

### Control Layer

Finally verify:

```text
React
 ↓
FastAPI
 ↓
MQTT
 ↓
Gateway
 ↓
nRF24
 ↓
Sensor Node
 ↓
LED
```

---

# 🔐 Security

Security is an important part of the deployment process.

Never commit:

```text
.env
Wi-Fi passwords
MQTT passwords
Database passwords
API keys
Private keys
Sensitive certificates
```

Use environment variables for credentials.

The repository should contain:

```text
.env.example
```

rather than the real:

```text
.env
```

If credentials have ever been exposed in source code or Git history, **rotate those credentials before public deployment**.

---

# 📈 Future Improvements

Potential future development includes:

* [ ] Real-time WebSocket dashboard updates
* [ ] Min / Max / Average analytics
* [ ] Sensor trend analysis
* [ ] Alert and notification system
* [ ] CSV data export
* [ ] Multiple sensor nodes
* [ ] Multi-device management
* [ ] User authentication
* [ ] Role-based access control
* [ ] Additional environmental sensors
* [ ] Mobile application
* [ ] Cloud deployment
* [ ] Advanced device configuration
* [ ] Sensor anomaly detection
* [ ] Predictive analytics
* [ ] Improved MQTT TLS certificate validation
* [ ] Automated testing and CI/CD

---

# 🎯 Project Objectives

The main objectives of IoT EdgeSense are to demonstrate practical implementation of:

* Embedded systems
* Wireless sensor networks
* IoT communication
* MQTT
* Cloud-connected devices
* REST API development
* Database integration
* Web-based monitoring
* Remote device control
* Fault detection
* Offline data handling
* Containerized backend services
* Full-stack IoT application development

---

# 💡 What This Project Demonstrates

IoT EdgeSense combines multiple engineering domains into a single working system:

```text
Embedded Systems
        +
Wireless Communication
        +
IoT / MQTT
        +
Cloud Connectivity
        +
Backend Development
        +
Database Engineering
        +
Docker
        +
Frontend Development
        =
End-to-End IoT Platform
```

The project demonstrates how sensor data can travel from a physical device through a wireless gateway and cloud MQTT infrastructure into a database and web application, while also supporting commands flowing in the opposite direction back to the physical device.

---

# 📚 Learning Outcomes

Through this project, practical experience was gained in:

* ESP32 firmware development
* Sensor interfacing
* SPI communication
* nRF24L01 wireless communication
* MQTT architecture
* MQTT over TLS
* Wi-Fi connectivity and recovery
* Packet-loss detection
* Offline buffering
* Python development
* FastAPI REST API development
* PostgreSQL database integration
* React dashboard development
* Data visualization with Recharts
* Docker containerization
* Docker Compose
* Environment-based configuration
* Full-stack IoT architecture

---

# 👨‍💻 Author

**Kavindu Gamhatha**

BSc Honours in Electronics & Information Technology
Faculty of Science, University of Colombo

---

# 📄 License

This project is licensed under the MIT License.

See the [`LICENSE`](LICENSE) file for details.

---

## ⭐ Project

If you find this project useful or interesting, feel free to explore the repository and follow the development of IoT EdgeSense.
