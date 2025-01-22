#ifndef SCANNER_H
#define SCANNER_H

// Include all relevant libraries.
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>

// Init array that will store.
extern byte nuidPICC[4];
extern char hexString[9];

// Variables for LCD display.
extern int lcdColumns;
extern int lcdRows;
extern LiquidCrystal_I2C lcd;

// Payload variable for receiving messages from the cloud.
extern String payload;


// Define variables for wifi and mqtt server setup.
extern const char *ssid;
extern const char *password;

extern const char *mqtt_server;          
extern const int mqtt_port;                        
extern const char *mqtt_user;  
extern const char *mqtt_pass;            
extern const char *mqtt_topic;
extern const char *mqtt_topic_ST;
extern const char *mqtt_topic_SH;
extern const char *mqtt_topic_CT;
extern const char *mqtt_topic_CH;

// define RFID object, wificlient and pubsubclient as external variables to not risk declaring them in multiple cpp files.
extern MFRC522 rfid;       
extern WiFiClient espClient;
extern PubSubClient client;

// functions used in the code, see full descriptions in cpp file.
String getValue(String data, char separator, int index);

void setupscanner();

void printHex(byte *buffer, byte bufferSize);

void printDec(byte *buffer, byte bufferSize);

void callback(char *byteArraytopic, byte *byteArrayPayload, unsigned int length);

void setup_wifi();

void reconnect();

#endif