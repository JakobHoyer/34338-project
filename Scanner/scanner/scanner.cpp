// scanner.cpp
/**
 * @file scanner.cpp
 * @brief Implementation file for the scanner module in our IoT project. All functions that are used in the main file can be seen here.
 * @author Anel Hodza and Søren Zepernick  
 */

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include "scanner.h"

/**
 * @brief RFID reader object initialized with SS and reset pins.
 */
MFRC522 rfid(D8, D3);

/**
 * @brief Array to store the NUID of the scanned RFID card.
 */
byte nuidPICC[4];

/**
 * @brief Character array to store the hexadecimal representation of the RFID UID.
 */
char hexString[9];

/**
 * @brief Number of columns on the LCD display.
 */
int lcdColumns = 16;

/**
 * @brief Number of rows on the LCD display.
 */
int lcdRows = 2;

/**
 * @brief LCD object for handling text display.
 */
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

/**
 * @brief Payload for receiving messages from the cloud.
 */
String payload;

/**
 * @brief WiFi SSID to connect to.
 */
const char *ssid = "Jakob - iPhone";

/**
 * @brief Password for the WiFi network.
 */
const char *password = "kodekodeadad";

/**
 * @brief MQTT server address.
 */
const char *mqtt_server = "maqiatto.com";

/**
 * @brief MQTT server port.
 */
const int mqtt_port = 1883;

/**
 * @brief MQTT username for authentication.
 */
const char *mqtt_user = "s183668@student.dtu.dk";

/**
 * @brief MQTT password for authentication.
 */
const char *mqtt_pass = "kodekodeadad";

/**
 * @brief MQTT topic for cloud-to-sensor communication.
 */
const char *mqtt_topic = "s183668@student.dtu.dk/hub";

/**
 * @brief MQTT topic for sensor-to-cloud temperature updates.
 */
const char *mqtt_topic_ST = "s183668@student.dtu.dk/SensorToCloudTemp";

/**
 * @brief MQTT topic for sensor-to-cloud humidity updates.
 */
const char *mqtt_topic_SH = "s183668@student.dtu.dk/SensorToCloudHum";

/**
 * @brief MQTT topic for cloud-to-sensor temperature commands.
 */
const char *mqtt_topic_CT = "s183668@student.dtu.dk/CloudToSensorTemp";

/**
 * @brief MQTT topic for cloud-to-sensor humidity commands.
 */
const char *mqtt_topic_CH = "s183668@student.dtu.dk/CloudToSensorHum";

/**
 * @brief WiFi client object for connecting to the network.
 */
WiFiClient espClient;

/**
 * @brief PubSubClient object for handling MQTT communication.
 */
PubSubClient client(mqtt_server, mqtt_port, callback, espClient);

String getValue(String data, char separator, int index) {
  /**
   * @brief Extracts a specific value from a string.
   * 
   * This function takes a string as an input and separates it using
   * a seperator element. Outputs the index position of the string.
   * Example: For "name;temp;hum" and index 0, the output is "name".
   * 
   * @param data Input string to parse.
   * @param separator Delimiter used to separate values.
   * @param index Index of the value to extract.
   * @return The extracted value as a string.
   */
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void printHex(byte *buffer, byte bufferSize) {
  /**
   * @brief Prints a byte buffer as a hexadecimal string to the serial monitor.
   * 
   * @param buffer Pointer to the byte array.
   * @param bufferSize Size of the byte array.
   */
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  /**
   * @brief Prints a byte buffer as a decimal string to the serial monitor.
   * 
   * @param buffer Pointer to the byte array.
   * @param bufferSize Size of the byte array.
   */
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}

void callback(char *byteArraytopic, byte *byteArrayPayload, unsigned int length) {
  /**
   * @brief Callback function to handle incoming MQTT messages.
   * 
   * This function processes messages received from the MQTT server
   * and displays relevant data on the LCD display.
   * 
   * @param byteArraytopic Topic of the received message.
   * @param byteArrayPayload Payload of the received message.
   * @param length Length of the payload.
   */
  String topic;
  topic = String(byteArraytopic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Check which topic is received among the subscribed topics (see reconnect function to see all subscribed topics).
  if (topic == mqtt_topic) {
    payload = "";
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    Serial.println(payload);  // print the current payload in the serial monitor for debugging.
    lcd.clear();              // clear display.
    String name = getValue(payload, ';', 0);
    String temp = getValue(payload, ';', 1);
    String hum = getValue(payload, ';', 2);
    Serial.println(hum);
    lcd.setCursor(8, 1);
    lcd.print(hum);
    Serial.println(temp);
    lcd.setCursor(0, 1);
    lcd.print(temp);
    Serial.println(name);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(name);
  }
  // else if the topic is of type ST (sensor temperature), then display the current temperature from the sensor.
  else if (topic == mqtt_topic_ST) {
    payload = "";
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    float payloadFloat = payload.toFloat();  // Convert string to float.
    char paybuffer[10];                      // Buffer to store formatted value.
    dtostrf(payloadFloat, 4, 1, paybuffer);  // Convert float to string with 1 decimal.
    lcd.setCursor(3, 1);
    lcd.print(paybuffer);
  }
  // else if the topic is of type SH (Sensor humidity), then display the current humidity from the sensor.
  else if (topic == mqtt_topic_SH) {
    payload = "";
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    float payloadFloat = payload.toFloat();  // Convert string to float.
    char paybuffer[10];                      // Buffer to store formatted value.
    dtostrf(payloadFloat, 4, 1, paybuffer);  // Convert float to string with 1 decimal.
    lcd.setCursor(11, 1);
    lcd.print(paybuffer);
  }
  // else if the topic is of type CT (Cloud temperature), then display the desired temperature from the cloud.
  else if (topic == mqtt_topic_CT) {
    payload = "";
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    lcd.setCursor(0, 1);
    lcd.print(payload);
  }
  // else if the topic is of type CH (Cloud humidity), then display the desired humidity from the cloud.
  else if (topic == mqtt_topic_CH) {
    payload = "";
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    lcd.setCursor(8, 1);
    lcd.print(payload);
  }
}

void setup_wifi() {
  /**
   * @brief Connects to the specified WiFi network.
   */
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  /**
   * @brief Reconnects to the MQTT server and subscribes to topics if disconnected.
   */
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("HubClient", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
      client.subscribe(mqtt_topic_ST);
      client.subscribe(mqtt_topic_SH);
      client.subscribe(mqtt_topic_CT);
      client.subscribe(mqtt_topic_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setupscanner() {
  /**
   * @brief Initializes the RFID scanner and sets up the LCD display.
   */
  rfid.PCD_Init();
  Wire.begin(D4, D1);
  lcd.init();
  lcd.backlight();

  Serial.println(F("This code scans the MIFARE Classic NUID."));
  lcd.setCursor(0, 1);
  lcd.clear();
  lcd.backlight();

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}
