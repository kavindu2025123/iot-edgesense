#include <SPI.h>
#include <RF24.h>
#include <DHT.h>

// Pin Definitions
#define LDR_PIN 34
#define DHT_PIN 4
#define DHT_TYPE DHT11

#define CE_PIN 16
#define CSN_PIN 5

#define LED_PIN 2

// Hardware Objects
RF24 radio(CE_PIN, CSN_PIN);
DHT dht(DHT_PIN, DHT_TYPE);

// NRF24 Pipe Addresses
const byte dataAddress[6] = "NODE1";      // Transmit address to Gateway
const byte commandAddress[6] = "CMD01";    // Receive address from Gateway

// Data Structures
struct SensorPacket {
  uint8_t nodeID;
  uint16_t packetID;
  uint16_t light;
  float temperature;
  float humidity;
};

struct CommandPacket {
  char command[10];
};

// Global Variables
SensorPacket data;
uint16_t packetCounter = 0;

bool sendSensorData() {
  radio.stopListening();
  radio.openWritingPipe(dataAddress);

  bool success = radio.write(&data, sizeof(data));

  if (success) {
    Serial.println("SENSOR DATA SENT + ACK RECEIVED");
  } else {
    Serial.println("SENSOR DATA SEND FAILED");
  }

  return success;
}

void checkCommand() {
  radio.openReadingPipe(1, commandAddress);
  radio.startListening();

  unsigned long startTime = millis();
  CommandPacket command;

  // Listen for command responses
  while (millis() - startTime < 1000) {
    if (radio.available()) {
      radio.read(&command, sizeof(command));

      Serial.println("\n==========================");
      Serial.println("COMMAND RECEIVED");
      Serial.print("Command: ");
      Serial.println(command.command);

      if (strcmp(command.command, "LED_ON") == 0) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED TURNED ON");
      } else if (strcmp(command.command, "LED_OFF") == 0) {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED TURNED OFF");
      } else {
        Serial.println("UNKNOWN COMMAND");
      }

      Serial.println("==========================");
    }
  }

  radio.stopListening();
  radio.openWritingPipe(dataAddress);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  SPI.begin(18, 19, 23, CSN_PIN);

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

  radio.openWritingPipe(dataAddress);
  radio.openReadingPipe(1, commandAddress);

  radio.stopListening();

  Serial.println("\n==========================");
  Serial.println("ESP32 #1 SENSOR NODE");
  Serial.println("READY");
  Serial.println("==========================");
}

void loop() {
  int lightValue = analogRead(LDR_PIN);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT11 ERROR");
    delay(2000);
    return;
  }

  data.nodeID = 1;
  data.packetID = packetCounter++;
  data.light = lightValue;
  data.temperature = temperature;
  data.humidity = humidity;

  Serial.println("\n--------------------------");
  Serial.print("Packet ID: ");
  Serial.println(data.packetID);
  Serial.print("Light: ");
  Serial.println(data.light);
  Serial.print("Temperature: ");
  Serial.print(data.temperature);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(data.humidity);
  Serial.println(" %");

  sendSensorData();
  checkCommand();

  delay(3000);
}