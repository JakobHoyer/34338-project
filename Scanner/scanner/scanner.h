/**
 * @file scanner.h
 * @brief Header file for the scanner module in our smart home project.
 */

#ifndef SCANNER_H
#define SCANNER_H

// Include all relevant libraries.
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>

/**
 * @brief Array to store the NUID of the scanned RFID card.
 */
extern byte nuidPICC[4];

/**
 * @brief Character array to store the hexadecimal representation of the RFID UID.
 */
extern char hexString[9];

/**
 * @brief Number of columns on the LCD display.
 */
extern int lcdColumns;

/**
 * @brief Number of rows on the LCD display.
 */
extern int lcdRows;

/**
 * @brief LCD object for handling text display.
 */
extern LiquidCrystal_I2C lcd;

/**
 * @brief Payload for receiving messages from the cloud.
 */
extern String payload;

/**
 * @brief WiFi SSID to connect to.
 */
extern const char *ssid;

/**
 * @brief Password for the WiFi network.
 */
extern const char *password;

/**
 * @brief MQTT server address.
 */
extern const char *mqtt_server;

/**
 * @brief MQTT server port.
 */
extern const int mqtt_port;

/**
 * @brief MQTT username for authentication.
 */
extern const char *mqtt_user;

/**
 * @brief MQTT password for authentication.
 */
extern const char *mqtt_pass;

/**
 * @brief MQTT topic for cloud-to-sensor communication.
 */
extern const char *mqtt_topic;

/**
 * @brief MQTT topic for sensor-to-cloud temperature updates.
 */
extern const char *mqtt_topic_ST;

/**
 * @brief MQTT topic for sensor-to-cloud humidity updates.
 */
extern const char *mqtt_topic_SH;

/**
 * @brief MQTT topic for cloud-to-sensor temperature commands.
 */
extern const char *mqtt_topic_CT;

/**
 * @brief MQTT topic for cloud-to-sensor humidity commands.
 */
extern const char *mqtt_topic_CH;

/**
 * @brief RFID reader object for handling RFID card operations.
 */
extern MFRC522 rfid;

/**
 * @brief WiFi client object for connecting to the network.
 */
extern WiFiClient espClient;

/**
 * @brief PubSubClient object for handling MQTT communication.
 */
extern PubSubClient client;

/**
 * @brief Extracts a specific value from a delimited string.
 * 
 * @param data The input string to parse.
 * @param separator The delimiter used to separate values.
 * @param index The index of the value to extract.
 * @return The extracted value as a string.
 */
String getValue(String data, char separator, int index);

/**
 * @brief Initializes the RFID scanner and sets up the LCD display.
 */
void setupscanner();

/**
 * @brief Prints a byte buffer as a hexadecimal string.
 * 
 * @param buffer Pointer to the byte array.
 * @param bufferSize Size of the byte array.
 */
void printHex(byte *buffer, byte bufferSize);

/**
 * @brief Prints a byte buffer as a decimal string.
 * 
 * @param buffer Pointer to the byte array.
 * @param bufferSize Size of the byte array.
 */
void printDec(byte *buffer, byte bufferSize);

/**
 * @brief Callback function for handling incoming MQTT messages.
 * 
 * @param byteArraytopic Topic of the received message.
 * @param byteArrayPayload Payload of the received message.
 * @param length Length of the payload.
 */
void callback(char *byteArraytopic, byte *byteArrayPayload, unsigned int length);

/**
 * @brief Connects to the specified WiFi network.
 */
void setup_wifi();

/**
 * @brief Reconnects to the MQTT server and subscribes to topics if disconnected.
 */
void reconnect();

#endif
