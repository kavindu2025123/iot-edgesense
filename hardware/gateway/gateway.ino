#include <SPI.h>
#include <RF24.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
#include "../config.h"

#define CE_PIN 16
#define CSN_PIN 5

const char* ssid = WIFI_SSID;
const char* wifiPassword = WIFI_PASSWORD;

const char* mqttServer = MQTT_SERVER;
const int mqttPort = MQTT_PORT;

const char* mqttUsername = MQTT_USERNAME;
const char* mqttPassword = MQTT_PASSWORD;

const char* dataTopic = "home/sensors/node1/data";
const char* statusTopic = "home/sensors/node1/status";
const char* commandTopic = "home/sensors/node1/command";

const long GMT_OFFSET_SEC = 5 * 3600 + 30 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;

RF24 radio(CE_PIN, CSN_PIN);

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

const byte dataAddress[6] = "NODE1";
const byte commandAddress[6] = "CMD01";

struct SensorPacket {
  uint8_t nodeID;
  uint16_t packetID;
  uint16_t light;
  float temperature;
  float humidity;
};

struct BufferedPacket {
  SensorPacket data;
  char timestamp[25];
};

struct CommandPacket {
  char command[10];
};

SensorPacket data;

#define BUFFER_SIZE 20
BufferedPacket sensorBuffer[BUFFER_SIZE];
int bufferCount = 0;

char pendingCommand[10];
bool commandPending = false;
unsigned long lastCommandAttempt = 0;
int commandAttempts = 0;
const int MAX_COMMAND_ATTEMPTS = 20;
const unsigned long COMMAND_RETRY_INTERVAL = 300;

#define DARK_THRESHOLD 800
#define LIGHT_THRESHOLD 1200

bool ledState = false;
bool autoMode = true;

unsigned long lastMQTTAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL = 5000;

unsigned long lastStatusCheck = 0;
const unsigned long STATUS_INTERVAL = 5000;
unsigned long lastStatusPublish = 0;
const unsigned long STATUS_PUBLISH_INTERVAL = 5000;
unsigned long lastBufferFlush = 0;
const unsigned long BUFFER_FLUSH_INTERVAL = 200;

bool haveReceivedPacket = false;
uint16_t lastReceivedPacketID = 0;
unsigned long totalReceivedPackets = 0;
unsigned long missingPacketCount = 0;

void publishDeviceStatus();
bool sendRFCommand(const char* command);
void processPendingCommand();
void receiveRFPackets();
void processSensorPacket(const SensorPacket &packet);
void bufferSensorData(const SensorPacket &packet, const char* timestamp);
void processOneBufferedPacket();

void connectWiFi() {
  Serial.println("\nConnecting to Wi-Fi...");
  WiFi.begin(ssid, wifiPassword);

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed.");
    Serial.println("Will retry later...");
  }
}

void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("\nWi-Fi connection lost!");
  connectWiFi();
}

void printConnectionStatus() {
  if (millis() - lastStatusCheck < STATUS_INTERVAL) {
    return;
  }

  lastStatusCheck = millis();

  Serial.println("\n========== CONNECTION STATUS ==========");
  Serial.print("Wi-Fi: ");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("CONNECTED");
  } else {
    Serial.println("DISCONNECTED");
  }

  Serial.print("MQTT: ");
  if (mqttClient.connected()) {
    Serial.println("CONNECTED");
  } else {
    Serial.println("DISCONNECTED");
  }

  Serial.print("Offline Buffer: ");
  Serial.print(bufferCount);
  Serial.print(" / ");
  Serial.println(BUFFER_SIZE);

  Serial.print("Total RF packets received: ");
  Serial.println(totalReceivedPackets);

  Serial.print("Missing packet count: ");
  Serial.println(missingPacketCount);
  Serial.println("=======================================");
}

void setupTime() {
  Serial.println("\nSynchronizing time...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10000)) {
    Serial.println("Time synchronized!");
    Serial.print("Current time: ");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
  } else {
    Serial.println("NTP synchronization failed!");
  }
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "TIME_ERROR";
  }

  char timestamp[25];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timestamp);
}

bool sendRFCommand(const char* command) {
  CommandPacket packet;
  memset(&packet, 0, sizeof(packet));
  strncpy(packet.command, command, sizeof(packet.command) - 1);

  radio.stopListening();
  radio.openWritingPipe(commandAddress);

  Serial.println("\n==========================");
  Serial.println("NRF24 COMMAND TRANSMISSION");
  Serial.print("Command: ");
  Serial.println(packet.command);

  bool success = radio.write(&packet, sizeof(packet));

  if (success) {
    Serial.println("RF COMMAND SENT + ACK RECEIVED");
  } else {
    Serial.println("RF COMMAND FAILED / NO ACK");
  }

  radio.openReadingPipe(1, dataAddress);
  radio.startListening();

  return success;
}

void automaticLightControl(uint16_t lightValue) {
  if (!autoMode) {
    return;
  }

  if (lightValue <= DARK_THRESHOLD) {
    if (!ledState) {
      Serial.println("\n*** AUTOMATIC LIGHT CONTROL ***");
      Serial.print("Light value: ");
      Serial.println(lightValue);
      Serial.println("Condition: DARK");
      Serial.println("Action: Turning LED ON");

      bool success = sendRFCommand("LED_ON");
      if (success) {
        ledState = true;
        Serial.println("Automatic LED_ON delivered");
        publishDeviceStatus();
      }
    } else {
      Serial.println("Dark detected - LED already ON");
    }
  } else if (lightValue >= LIGHT_THRESHOLD) {
    if (ledState) {
      Serial.println("\n*** AUTOMATIC LIGHT CONTROL ***");
      Serial.print("Light value: ");
      Serial.println(lightValue);
      Serial.println("Condition: LIGHT");
      Serial.println("Action: Turning LED OFF");

      bool success = sendRFCommand("LED_OFF");
      if (success) {
        ledState = false;
        Serial.println("Automatic LED_OFF delivered");
        publishDeviceStatus();
      }
    } else {
      Serial.println("Light detected - LED already OFF");
    }
  } else {
    Serial.print("Light value ");
    Serial.print(lightValue);
    Serial.println(" is in transition zone - keeping current LED state");
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n==========================");
  Serial.println("MQTT MESSAGE RECEIVED");
  Serial.print("Topic: ");
  Serial.println(topic);

  char command[10];
  if (length >= sizeof(command)) {
    length = sizeof(command) - 1;
  }

  memset(command, 0, sizeof(command));
  memcpy(command, payload, length);
  command[length] = '\0';

  Serial.print("Command: ");
  Serial.println(command);

  if (strcmp(command, "AUTO") == 0) {
    autoMode = true;
    publishDeviceStatus();
    Serial.println("MODE CHANGED: AUTO");
    Serial.println("LDR will now control the LED");
  } else if (strcmp(command, "MANUAL") == 0) {
    autoMode = false;
    publishDeviceStatus();
    Serial.println("MODE CHANGED: MANUAL");
    Serial.println("MQTT commands will control the LED");
  } else if (strcmp(command, "LED_ON") == 0 || strcmp(command, "LED_OFF") == 0) {
    if (!autoMode) {
      memset(pendingCommand, 0, sizeof(pendingCommand));
      strncpy(pendingCommand, command, sizeof(pendingCommand) - 1);

      commandPending = true;
      commandAttempts = 0;
      lastCommandAttempt = 0;

      Serial.println("MANUAL COMMAND STORED");
    } else {
      Serial.println("LED COMMAND IGNORED");
      Serial.println("System is currently in AUTO mode.");
      Serial.println("Send MANUAL first.");
    }
  } else {
    Serial.println("UNKNOWN COMMAND");
    Serial.println("Use AUTO, MANUAL, LED_ON or LED_OFF");
  }

  Serial.println("==========================");
}

void processPendingCommand() {
  if (!commandPending) {
    return;
  }

  if (millis() - lastCommandAttempt < COMMAND_RETRY_INTERVAL) {
    return;
  }

  if (commandAttempts >= MAX_COMMAND_ATTEMPTS) {
    Serial.println("\nCOMMAND TRANSMISSION FAILED");
    commandPending = false;
    return;
  }

  commandAttempts++;
  lastCommandAttempt = millis();

  Serial.print("\nCommand attempt ");
  Serial.print(commandAttempts);
  Serial.print(" / ");
  Serial.println(MAX_COMMAND_ATTEMPTS);

  bool success = sendRFCommand(pendingCommand);

  if (success) {
    Serial.println("COMMAND SUCCESSFULLY DELIVERED");

    if (strcmp(pendingCommand, "LED_ON") == 0) {
      ledState = true;
    } else if (strcmp(pendingCommand, "LED_OFF") == 0) {
      ledState = false;
    }

    publishDeviceStatus();
    commandPending = false;
  }
}

bool publishSensorPacket(const SensorPacket &packet, const char* timestamp) {
  if (!mqttClient.connected()) {
    return false;
  }

  char message[250];
  snprintf(
    message,
    sizeof(message),
    "{\"node\":%d,\"packet\":%d,\"light\":%d,\"temperature\":%.2f,\"humidity\":%.2f,\"timestamp\":\"%s\"}",
    packet.nodeID,
    packet.packetID,
    packet.light,
    packet.temperature,
    packet.humidity,
    timestamp
  );

  bool success = mqttClient.publish(dataTopic, message);

  if (success) {
    Serial.println("MQTT DATA PUBLISHED");
    Serial.print("Packet ID: ");
    Serial.println(packet.packetID);
    return true;
  } else {
    Serial.println("MQTT DATA PUBLISH FAILED");
    return false;
  }
}

void bufferSensorData(const SensorPacket &packet, const char* timestamp) {
  if (bufferCount >= BUFFER_SIZE) {
    Serial.println("\n!!! SENSOR BUFFER FULL !!!");
    Serial.print("Packet ");
    Serial.print(packet.packetID);
    Serial.println(" discarded.");
    return;
  }

  sensorBuffer[bufferCount].data = packet;
  strncpy(sensorBuffer[bufferCount].timestamp, timestamp, sizeof(sensorBuffer[bufferCount].timestamp) - 1);
  sensorBuffer[bufferCount].timestamp[sizeof(sensorBuffer[bufferCount].timestamp) - 1] = '\0';

  bufferCount++;

  Serial.println("\n*** SENSOR DATA BUFFERED ***");
  Serial.print("Packet ID: ");
  Serial.println(packet.packetID);
  Serial.print("Buffer: ");
  Serial.print(bufferCount);
  Serial.print(" / ");
  Serial.println(BUFFER_SIZE);
}

void processOneBufferedPacket() {
  if (!mqttClient.connected()) {
    return;
  }

  if (bufferCount == 0) {
    return;
  }

  if (millis() - lastBufferFlush < BUFFER_FLUSH_INTERVAL) {
    return;
  }

  lastBufferFlush = millis();

  BufferedPacket packet = sensorBuffer[0];

  Serial.print("\nSending buffered packet: ");
  Serial.println(packet.data.packetID);

  bool success = publishSensorPacket(packet.data, packet.timestamp);

  if (success) {
    for (int i = 1; i < bufferCount; i++) {
      sensorBuffer[i - 1] = sensorBuffer[i];
    }

    bufferCount--;

    Serial.print("Buffered packets remaining: ");
    Serial.println(bufferCount);

    if (bufferCount == 0) {
      Serial.println("\n*** BUFFER FLUSH COMPLETE ***");
      Serial.println("All offline packets uploaded.");
    }
  } else {
    Serial.println("Buffered packet publish failed.");
    Serial.println("Will retry later.");
  }
}

void publishDeviceStatus() {
  if (!mqttClient.connected()) {
    Serial.println("DEVICE STATUS NOT PUBLISHED - MQTT OFFLINE");
    return;
  }

  char statusMessage[250];
  const char* modeText = autoMode ? "AUTO" : "MANUAL";
  const char* ledText = ledState ? "ON" : "OFF";

  snprintf(
    statusMessage,
    sizeof(statusMessage),
    "{\"online\":true,\"mode\":\"%s\",\"led\":\"%s\",\"light\":%d,\"packet\":%d,\"total_packets\":%lu,\"missing_packets\":%lu}",
    modeText,
    ledText,
    data.light,
    data.packetID,
    totalReceivedPackets,
    missingPacketCount
  );

  if (mqttClient.publish(statusTopic, statusMessage, true)) {

    Serial.println("DEVICE STATUS PUBLISHED");
    Serial.print("Status JSON: ");
    Serial.println(statusMessage);

  } else {

    Serial.println("DEVICE STATUS PUBLISH FAILED");

  }
}

void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (mqttClient.connected()) {
    return;
  }

  Serial.println("\nAttempting MQTT connection...");

  String clientID =
    "ESP32_Gateway_" +
    String((uint32_t)ESP.getEfuseMac(), HEX);

  if (mqttClient.connect(
        clientID.c_str(),
        mqttUsername,
        mqttPassword
      )) {

    Serial.println("MQTT CONNECTED!");

    if (mqttClient.subscribe(commandTopic)) {
      Serial.println("MQTT COMMAND SUBSCRIBED");
    } else {
      Serial.println("MQTT SUBSCRIBE FAILED");
    }

    publishDeviceStatus();

    Serial.println("MQTT recovery complete.");

  } else {

    Serial.print("MQTT connection FAILED. State = ");
    Serial.println(mqttClient.state());
  }
}

void receiveRFPackets() {
  // Drain ALL packets currently waiting in the nRF24 FIFO.
  while (radio.available()) {
    SensorPacket receivedPacket;
    radio.read(&receivedPacket, sizeof(receivedPacket));

    totalReceivedPackets++;

    if (haveReceivedPacket) {
      uint16_t expectedPacket = lastReceivedPacketID + 1;

      if (receivedPacket.packetID != expectedPacket) {
        Serial.println("\n!!! PACKET SEQUENCE WARNING !!!");
        Serial.print("Expected packet: ");
        Serial.println(expectedPacket);
        Serial.print("Received packet: ");
        Serial.println(receivedPacket.packetID);

        // Only count forward gaps to avoid treating duplicates as missing packets.
        uint16_t difference = receivedPacket.packetID - expectedPacket;

        if (difference < 1000) {
          missingPacketCount += difference;
        }
      }
    }

    lastReceivedPacketID = receivedPacket.packetID;
    haveReceivedPacket = true;

    processSensorPacket(receivedPacket);
  }
}

void processSensorPacket(const SensorPacket &packet) {
  data = packet;
  String timestamp = getTimestamp();

  Serial.println("\n--------------------------");
  Serial.println("SENSOR DATA RECEIVED");
  Serial.print("Time: ");
  Serial.println(timestamp);
  Serial.print("Node: ");
  Serial.println(packet.nodeID);
  Serial.print("Packet: ");
  Serial.println(packet.packetID);
  Serial.print("Light: ");
  Serial.println(packet.light);
  Serial.print("Temperature: ");
  Serial.print(packet.temperature);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(packet.humidity);
  Serial.println(" %");

  automaticLightControl(packet.light);

  if (mqttClient.connected()) {
    bool published = publishSensorPacket(packet, timestamp.c_str());

    if (!published) {
      bufferSensorData(packet, timestamp.c_str());
    }
  } else {
    Serial.println("MQTT OFFLINE - STORING SENSOR DATA");
    bufferSensorData(packet, timestamp.c_str());
  }

  Serial.println("--------------------------");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWiFi();
  setupTime();

  secureClient.setInsecure();
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);

  connectMQTT();

  SPI.begin(18, 19, 23, CSN_PIN);

  Serial.println("\nStarting nRF24...");

  if (!radio.begin()) {
    Serial.println("nRF24 NOT DETECTED!");
    while (1) {
      delay(1000);
    }
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(true);
  radio.setRetries(5, 15);
  radio.setCRCLength(RF24_CRC_16);

  radio.openReadingPipe(1, dataAddress);
  radio.startListening();

  Serial.println("\n==========================");
  Serial.println("ESP32 #2 GATEWAY");
  Serial.println("READY");
  Serial.println("RF packet receiver: ACTIVE");
  Serial.println("MQTT: READY");
  Serial.println("Automatic light control: ENABLED");
  Serial.println("Offline buffer: 20 packets");
  Serial.println("==========================");
}

void loop() {
  maintainWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      if (millis() - lastMQTTAttempt >= MQTT_RETRY_INTERVAL) {
        lastMQTTAttempt = millis();
        Serial.println("\nMQTT is disconnected.");
        Serial.println("Trying to reconnect...");
        connectMQTT();
      }
    } else {
      mqttClient.loop();
    }
  }
  // RF reception gets priority.
  receiveRFPackets();
  processPendingCommand();
  // Non-blocking offline packet flushing.
  processOneBufferedPacket();
  printConnectionStatus();

  if (mqttClient.connected()) {
  if (millis() - lastStatusPublish >= STATUS_PUBLISH_INTERVAL) {
    lastStatusPublish = millis();
    publishDeviceStatus();
    }
}
}