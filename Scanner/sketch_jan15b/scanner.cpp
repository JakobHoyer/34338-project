#include <Arduino.h> // Always include this for Arduino-specific functions
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include "scanner.h"


MFRC522 rfid(D8, D3);  // Instance of the class

// Init array that will store new NUID
byte nuidPICC[4];
char hexString[9];

// variables for LCD display
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);


String payload;  // Definerer variablen 'payload' i det globale scope (payload er navnet på besked-variablen)


const char *ssid = "Jakob - iPhone";
const char *password = "kodekodeadad";

const char *mqtt_server = "maqiatto.com";          //navn på mqtt-server. Find navnet på cloudmqtt-hjemmesiden
const int mqtt_port = 1883;                        // Definerer porten
const char *mqtt_user = "s183668@student.dtu.dk";  // Definerer mqtt-brugeren
const char *mqtt_pass = "kodekodeadad";            // Definerer koden til mqtt-brugeren
const char *mqtt_topic = "s183668@student.dtu.dk/hub";
const char *mqtt_topic_ST = "s183668@student.dtu.dk/SensorToCloudTemp";
const char *mqtt_topic_SH = "s183668@student.dtu.dk/SensorToCloudHum";
const char *mqtt_topic_CT = "s183668@student.dtu.dk/CloudToSensorTemp";
const char *mqtt_topic_CH = "s183668@student.dtu.dk/CloudToSensorHum";



WiFiClient espClient;  // Initialiserer wifi bibloteket ESP8266Wifi, som er inkluderet under "nødvendige bibloteker"
PubSubClient client(mqtt_server, mqtt_port, callback, espClient);  // Initialiserer bibloteket for at kunne modtage og sende beskeder til mqtt. Den henter fra definerede mqtt server og port. Den henter fra topic og beskeden payload




String getValue(String data, char separator, int index) {
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
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
    // ');
    // lcd.print(buffer[i], DEC);
  }
}

void callback(char *byteArraytopic, byte *byteArrayPayload, unsigned int length) {

  // Konverterer indkomne besked (topic) til en string:
  String topic;
  topic = String(byteArraytopic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  // Konverterer den indkomne besked (payload) fra en array til en string:
  // Topic == Temperaturmaaler, Topic == Kraftsensor
  if (topic == mqtt_topic) {  // OBS: der subscribes til et topic nede i reconnect-funktionen. I det her tilfælde er der subscribed til "Test". Man kan subscribe til alle topics ved at bruge "#"
    payload = "";             // Nulstil payload variablen så forloopet ikke appender til en allerede eksisterende payload
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    Serial.println(payload);
    lcd.clear();
    String name = getValue(payload, ';', 0);
    String temp = getValue(payload, ';', 1);
    String hum = getValue(payload, ';', 2);
    Serial.println(hum);
    lcd.setCursor(8, 1);  // Set the LCD cursor position
    lcd.print(hum);
    Serial.println(temp);
    lcd.setCursor(0, 1);  // Set the LCD cursor position
    lcd.print(temp);
    Serial.println(name);
    lcd.setCursor(0, 0);  // Set the LCD cursor position
    lcd.print("                ");
    lcd.setCursor(0, 0);  // Set the LCD cursor position
    lcd.print(name);
  } else if (topic == mqtt_topic_ST) {  // OBS: der subscribes til et topic nede i reconnect-funktionen. I det her tilfælde er der subscribed til "Test". Man kan subscribe til alle topics ved at bruge "#"
    payload = "";                       // Nulstil payload variablen så forloopet ikke appender til en allerede eksisterende payload
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
  float payloadFloat = payload.toFloat();  // Convert string to float
  char paybuffer[10];  // Buffer to store formatted value
  dtostrf(payloadFloat, 4, 1, paybuffer);  // Convert float to string with 1 decimal

  lcd.setCursor(3, 1);  // Set cursor to first row
  lcd.print(paybuffer);
    // store current temp

  } else if (topic == mqtt_topic_SH) {  // OBS: der subscribes til et topic nede i reconnect-funktionen. I det her tilfælde er der subscribed til "Test". Man kan subscribe til alle topics ved at bruge "#"
    payload = "";                       // Nulstil payload variablen så forloopet ikke appender til en allerede eksisterende payload
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    float payloadFloat = payload.toFloat();  // Convert string to float
  char paybuffer[10];  // Buffer to store formatted value
  dtostrf(payloadFloat, 4, 1, paybuffer);  // Convert float to string with 1 decimal

  lcd.setCursor(11, 1);  // Set cursor to first row
  lcd.print(paybuffer);
    // Store current hum
  } else if (topic == mqtt_topic_CT) {  // OBS: der subscribes til et topic nede i reconnect-funktionen. I det her tilfælde er der subscribed til "Test". Man kan subscribe til alle topics ved at bruge "#"
    payload = "";                       // Nulstil payload variablen så forloopet ikke appender til en allerede eksisterende payload
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    lcd.setCursor(0, 1);  // Set the LCD cursor position
    lcd.print(payload);
    // Store pref temp
  }

    else if (topic == mqtt_topic_CH) {  // OBS: der subscribes til et topic nede i reconnect-funktionen. I det her tilfælde er der subscribed til "Test". Man kan subscribe til alle topics ved at bruge "#"
    payload = "";                // Nulstil payload variablen så forloopet ikke appender til en allerede eksisterende payload
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }
    lcd.setCursor(8, 1);  // Set the LCD cursor position
    lcd.print(payload);
    // Store pref hum
  }
}

void setup_wifi() {
  // Forbinder til et WiFi network
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
  // Fortsætter til forbindelsen er oprettet
  while (!client.connected()) {
    Serial.print("Forsøger at oprette MQTT forbindelse...");

    if (client.connect("HubClient", mqtt_user, mqtt_pass)) {  // Forbinder til klient med mqtt bruger og password
      Serial.println("connected");
      // Derudover subsribes til topic "Test" hvor NodeMCU modtager payload beskeder fra
      client.subscribe(mqtt_topic);
      client.subscribe(mqtt_topic_ST);
      client.subscribe(mqtt_topic_SH);
      client.subscribe(mqtt_topic_CT);
      client.subscribe(mqtt_topic_CH);
      // Der kan subscribes til flere specifikke topics
      //client.subscribe("Test1");
      // Eller til samtlige topics ved at bruge '#' (Se Power Point fra d. 18. marts)
      // client.subscribe("#");

      // Hvis forbindelsen fejler køres loopet igen efter 5 sekunder indtil forbindelse er oprettet
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Venter 5 sekunder før den prøver igen
      delay(5000);
    }
  }
}


bool checkButtonState(int buttonPin) {
  static bool buttonState = false;        // Current toggle state (true/false)
  static bool lastButtonPressed = false;  // Tracks if the button was previously pressed

  bool currentButtonPressed = digitalRead(buttonPin) == LOW;  // Read current button state

  // Detect a rising edge: button goes from unpressed to pressed
  if (currentButtonPressed && !lastButtonPressed) {
    buttonState = !buttonState;  // Toggle the state
  }

  lastButtonPressed = currentButtonPressed;  // Update the last button state

  return buttonState;  // Return the new button state
}

void setupscanner(){
  rfid.PCD_Init();     // Init MFRC522'
  Wire.begin(D4, D1);  // SDA=D4 (GPIO2), SCL=D1 (GPIO5)
  lcd.init();
  lcd.backlight();
  // Print a message to the LCD.
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  lcd.setCursor(0, 1);
  lcd.clear();
  lcd.backlight();
  setup_wifi();   
  client.setServer(mqtt_server, mqtt_port);  // Forbinder til mqtt serveren (defineret længere oppe)
  client.setCallback(callback);  
}
