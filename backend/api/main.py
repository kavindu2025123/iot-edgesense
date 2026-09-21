from fastapi import FastAPI # type: ignore
from fastapi.middleware.cors import CORSMiddleware # type: ignore
import psycopg2
import os
import json
import paho.mqtt.client as mqtt
import threading
from datetime import datetime, timedelta
import time

app = FastAPI(title="IoT Sensor API")
@app.on_event("startup")
def startup_event():
    thread = threading.Thread(
        target=start_status_listener,
        daemon=True
    )

    thread.start()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)
MQTT_BROKER = "342859a9470f4f0ab1d678f5c21fab9d.s1.eu.hivemq.cloud"
MQTT_PORT = 8883
MQTT_USERNAME = "NRF_ESP32"
MQTT_PASSWORD = "20020501"
MQTT_COMMAND_TOPIC = "home/sensors/node1/command"
MQTT_STATUS_TOPIC = "home/sensors/node1/status"

device_status = {
    "online": False,
    "mode": "UNKNOWN",
    "led": "UNKNOWN",
    "light": 0,
    "packet": 0,
    "total_packets": 0,
    "missing_packets": 0
}
last_status_received = 0.0
STATUS_TIMEOUT = 15

def publish_mqtt_command(command):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

    client.username_pw_set(
        MQTT_USERNAME,
        MQTT_PASSWORD
    )

    client.tls_set()

    try:
        print(f"Connecting to MQTT broker...")

        client.connect(
            MQTT_BROKER,
            MQTT_PORT,
            60
        )

        # Start MQTT network loop
        client.loop_start()

        print(f"Publishing command: {command}")

        result = client.publish(
            MQTT_COMMAND_TOPIC,
            command,
            qos=1
        )

        # Wait until the message is actually transmitted
        result.wait_for_publish()

        print(f"MQTT publish result: {result.rc}")

        client.loop_stop()
        client.disconnect()

        return result.rc

    except Exception as error:
        print("MQTT command error:", error)

        try:
            client.loop_stop()
            client.disconnect()
        except:
            pass

        return -1

def on_status_message(client, userdata, msg):
    global device_status, last_status_received

    try:
        payload = msg.payload.decode()
        new_status = json.loads(payload)

        device_status = {
            "online": new_status.get("online", False),
            "mode": new_status.get("mode", "UNKNOWN"),
            "led": new_status.get("led", "UNKNOWN"),
            "light": new_status.get("light", 0),
            "packet": new_status.get("packet", 0),
            "total_packets": new_status.get("total_packets", 0),
            "missing_packets": new_status.get("missing_packets", 0)
        }

        # Record when the latest gateway status was received
        last_status_received = time.monotonic()

        print("Device status updated:")
        print(device_status)
        print("Last status received:", last_status_received)

    except Exception as error:
        print("Status message error:", error)

def on_status_connect(client, userdata, flags, reason_code, properties):
    print("======================================")
    print("MQTT STATUS LISTENER CONNECTED")
    print("Reason code:", reason_code)

    result, mid = client.subscribe(
        MQTT_STATUS_TOPIC,
        qos=1
    )

    if result == mqtt.MQTT_ERR_SUCCESS:
        print("Subscribed to:", MQTT_STATUS_TOPIC)
    else:
        print("MQTT SUBSCRIBE FAILED:", result)

    print("======================================")


def on_status_disconnect(client, userdata, disconnect_flags, reason_code, properties):
    print("======================================")
    print("MQTT STATUS LISTENER DISCONNECTED")
    print("Reason code:", reason_code)
    print("======================================")


def on_status_subscribe(client, userdata, mid, reason_codes, properties):
    print("MQTT STATUS SUBSCRIPTION CONFIRMED")
    print("MID:", mid)
    print("Reason codes:", reason_codes)


def start_status_listener():
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

    client.username_pw_set(
        MQTT_USERNAME,
        MQTT_PASSWORD
    )

    client.tls_set()

    client.on_connect = on_status_connect
    client.on_disconnect = on_status_disconnect
    client.on_subscribe = on_status_subscribe
    client.on_message = on_status_message

    while True:
        try:
            print("\nConnecting MQTT status listener...")

            client.connect(
                MQTT_BROKER,
                MQTT_PORT,
                60
            )

            print("MQTT status listener connected")

            client.loop_forever()

        except Exception as error:
            print("MQTT status listener error:", error)

            print("Retrying MQTT status listener in 5 seconds...")

            time.sleep(5)

def get_db_connection():
    return psycopg2.connect(
        host=os.getenv("DB_HOST", "localhost"),
        port=os.getenv("DB_PORT", "5432"),
        database=os.getenv("DB_NAME", "iot_database"),
        user=os.getenv("DB_USER", "postgres"),
        password=os.getenv("DB_PASSWORD", "hasaranga")
    )


@app.get("/")
def root():
    return {
        "message": "IoT Sensor API is running"
    }


@app.get("/api/sensors/latest")
def get_latest_sensor():

    conn = get_db_connection()
    cursor = conn.cursor()

    cursor.execute("""
        SELECT node, packet, light, temperature, humidity, timestamp
        FROM sensor_readings
        ORDER BY id DESC
        LIMIT 1
    """)

    row = cursor.fetchone()

    cursor.close()
    conn.close()

    if row is None:
        return {"message": "No sensor data available"}

    return {
        "node": row[0],
        "packet": row[1],
        "light": row[2],
        "temperature": row[3],
        "humidity": row[4],
        "timestamp": row[5]
    }


@app.get("/api/sensors/history")
def get_sensor_history(limit: int = 100, hours: int = 1):
    conn = get_db_connection()
    cursor = conn.cursor()

    if hours > 0:
        cursor.execute("""
            SELECT node, packet, light, temperature, humidity, timestamp
            FROM (
                SELECT node, packet, light, temperature, humidity, timestamp, id
                FROM sensor_readings
                WHERE timestamp >= NOW() - (%s * INTERVAL '1 hour')
                ORDER BY id DESC
                LIMIT %s
            ) AS recent
            ORDER BY id ASC
        """, (hours, limit))

    else:
        cursor.execute("""
            SELECT node, packet, light, temperature, humidity, timestamp
            FROM (
                SELECT node, packet, light, temperature, humidity, timestamp, id
                FROM sensor_readings
                ORDER BY id DESC
                LIMIT %s
            ) AS recent
            ORDER BY id ASC
        """, (limit,))

    rows = cursor.fetchall()

    cursor.close()
    conn.close()

    return [
        {
            "node": row[0],
            "packet": row[1],
            "light": row[2],
            "temperature": row[3],
            "humidity": row[4],
            "timestamp": row[5]
        }
        for row in rows
    ]

@app.post("/api/device/led/{state}")
def control_led(state: str):

    state = state.upper()

    if state == "ON":
        command = "LED_ON"

    elif state == "OFF":
        command = "LED_OFF"

    else:
        return {
            "success": False,
            "message": "Invalid LED state"
        }

    result = publish_mqtt_command(command)

    return {
        "success": result == 0,
        "command": command
    }

@app.post("/api/device/mode/{mode}")
def control_mode(mode: str):

    mode = mode.upper()

    if mode not in ["AUTO", "MANUAL"]:
        return {
            "success": False,
            "message": "Invalid mode"
        }

    result = publish_mqtt_command(mode)

    return {
        "success": result == 0,
        "command": mode
    }

@app.get("/api/device/status")
def get_device_status():
    global device_status, last_status_received

    status = device_status.copy()

    # Check how long it has been since the gateway
    # last sent a status heartbeat.
    if (
        last_status_received == 0.0
        or time.monotonic() - last_status_received > STATUS_TIMEOUT
    ):
        status["online"] = False

    return status