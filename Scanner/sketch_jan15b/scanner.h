#ifndef SKETCH_JAN15B_H
#define SKETCH_JAN15B_H

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>

//#define SS_PIN D8
//#define RST_PIN D3




extern byte nuidPICC[4];
extern char hexString[9];

// variables for LCD display
extern int lcdColumns;
extern int lcdRows;

extern LiquidCrystal_I2C lcd;




extern String payload;  // Definerer variablen 'payload' i det globale scope (payload er navnet på besked-variablen)


extern const char *ssid;
extern const char *password;

extern const char *mqtt_server;          //navn på mqtt-server. Find navnet på cloudmqtt-hjemmesiden
extern const int mqtt_port;                        // Definerer porten
extern const char *mqtt_user;  // Definerer mqtt-brugeren
extern const char *mqtt_pass;            // Definerer koden til mqtt-brugeren
extern const char *mqtt_topic;
extern const char *mqtt_topic_ST;
extern const char *mqtt_topic_SH;
extern const char *mqtt_topic_CT;
extern const char *mqtt_topic_CH;

extern MFRC522 rfid;       // RFID object
extern WiFiClient espClient;
extern PubSubClient client;




String getValue(String data, char separator, int index);

void setupscanner();

void printHex(byte *buffer, byte bufferSize);

void printDec(byte *buffer, byte bufferSize);

void callback(char *byteArraytopic, byte *byteArrayPayload, unsigned int length);

void setup_wifi();

void reconnect();

bool checkButtonState(int buttonPin);




#endif