import paho.mqtt.client as mqtt
import json
import csv
import os
import psycopg2

# POSTGRESQL SETTINGS

DB_HOST = os.getenv("DB_HOST", "localhost")
DB_PORT = int(os.getenv("DB_PORT", "5432"))
DB_NAME = os.getenv("DB_NAME", "iot_database")
DB_USER = os.getenv("DB_USER", "postgres")
DB_PASSWORD = os.getenv("DB_PASSWORD", "hasaranga")

# CONNECT TO POSTGRESQL
try:

    db = psycopg2.connect(
        host=DB_HOST,
        port=DB_PORT,
        database=DB_NAME,
        user=DB_USER,
        password=DB_PASSWORD
    )

    cursor = db.cursor()

    print("Connected to PostgreSQL!")

except Exception as error:

    print("PostgreSQL connection failed!")
    print(error)

# HIVE MQ SETTINGS

MQTT_BROKER = "342859a9470f4f0ab1d678f5c21fab9d.s1.eu.hivemq.cloud"
MQTT_PORT = 8883
MQTT_USERNAME = "NRF_ESP32"
MQTT_PASSWORD = "20020501"
MQTT_TOPIC = "home/sensors/node1/data"
# CSV file
CSV_FILE = "data/sensor_data.csv"
os.makedirs("data", exist_ok=True)
# CREATE CSV FILE

if not os.path.exists(CSV_FILE):

    with open(CSV_FILE, "w", newline="") as file:

        writer = csv.writer(file)
        writer.writerow([
            "timestamp",
            "node",
            "packet",
            "light",
            "temperature",
            "humidity"
        ])

    print("Created:", CSV_FILE)

# WHEN CONNECTED

def on_connect(client, userdata, flags, reason_code, properties):

    if reason_code == 0:

        print("Connected to HiveMQ Cloud!")
        client.subscribe(MQTT_TOPIC)
        print("Subscribed to:")
        print(MQTT_TOPIC)

    else:

        print("Connection failed!")
        print("Reason code:", reason_code)

# WHEN MESSAGE ARRIVES

def on_message(client, userdata, msg):
    try:
        # Convert MQTT message to string
        message = msg.payload.decode()

        print()
        print("----------------------------------------")
        print("MQTT MESSAGE RECEIVED")
        print("Topic:")
        print(msg.topic)
        print("Message:")
        print(message)

        # Convert JSON string to Python dictionary
        data = json.loads(message)

        # Extract values
        timestamp = data["timestamp"]
        node = data["node"]
        packet = data["packet"]
        light = data["light"]
        temperature = data["temperature"]
        humidity = data["humidity"]

        # Save to CSV
        with open(CSV_FILE, "a", newline="") as file:
            writer = csv.writer(file)
            writer.writerow([
                timestamp,
                node,
                packet,
                light,
                temperature,
                humidity
            ])

        print("Saved to CSV!")
        # SAVE TO POSTGRESQL
        cursor.execute(
            """
            INSERT INTO sensor_readings
            (timestamp, node, packet, light, temperature, humidity)
            VALUES (%s, %s, %s, %s, %s, %s)
            """,
            (
                timestamp,
                node,
                packet,
                light,
                temperature,
                humidity
            )
        )
        db.commit()
        print("Saved to PostgreSQL!")
        

    except json.JSONDecodeError:
        print("ERROR: Invalid JSON received!")
    except KeyError as error:
        print("ERROR: Missing JSON field:", error)
    except Exception as error:
        if "db" in globals() and db is not None:
            db.rollback()
        print("ERROR:", error)
        print("----------------------------------------")

# CREATE MQTT CLIENT

client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2
)

client.username_pw_set(
    MQTT_USERNAME,
    MQTT_PASSWORD
)

client.tls_set()

client.on_connect = on_connect
client.on_message = on_message

# CONNECT

print("Connecting to HiveMQ Cloud...")

client.connect(
    MQTT_BROKER,
    MQTT_PORT,
    60
)


# START MQTT LOOP

client.loop_forever()