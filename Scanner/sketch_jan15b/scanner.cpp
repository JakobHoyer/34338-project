#include <Arduino.h> // Always include this for Arduino-specific functions
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include "scanner.h"


MFRC522 rfid(D8, D3);  // Instance of the class, D8 is SS pin and D3 is reset pin.

// Init array that will store new NUID.
byte nuidPICC[4];
char hexString[9];

// variables for LCD display.
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// variable to receive messsages from the cloud.
String payload; 

// define variables for wifi and mqtt server setup. 
const char *ssid = "Jakob - iPhone";
const char *password = "kodekodeadad";

const char *mqtt_server = "maqiatto.com";          
const int mqtt_port = 1883;                        
const char *mqtt_user = "s183668@student.dtu.dk";  
const char *mqtt_pass = "kodekodeadad";            
const char *mqtt_topic = "s183668@student.dtu.dk/hub";
const char *mqtt_topic_ST = "s183668@student.dtu.dk/SensorToCloudTemp";
const char *mqtt_topic_SH = "s183668@student.dtu.dk/SensorToCloudHum";
const char *mqtt_topic_CT = "s183668@student.dtu.dk/CloudToSensorTemp";
const char *mqtt_topic_CH = "s183668@student.dtu.dk/CloudToSensorHum";

// initialize wifi client from ESP8266wifi library.
WiFiClient espClient;  
PubSubClient client(mqtt_server, mqtt_port, callback, espClient);  


/*
** function that takes a string as an input and can seperate the string using
** a seperator element. In our case the data is received as "name;temp;hum".
** This means that the function will take ";" as a seperator
** and outputs the index position of the string.
** An example would be if the data is "name;temp;hum", and you use index 0,
** the output would be "name".
*/
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


/*
** This function takes a buffer and a buffersize as input of type byte,
** and outputs the byte as a hexidecimal number in the serial monitor.
** This is mainly used for debugging the RFID chip readers UID numbers,
** and makes sure we receive the correct information.
*/
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/*
** This is the same as the previous function, but prints the byte as a 
** decimal instead of hexidecimal. This is also mainly used for debugging.
*/
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}


/*
** Callback function to receive the messages sent from the MQTT server. 
** All received messages are then displayed on our LCD display. There's
** a constant stream of data sent from our server that include the current temperature
** and humidity. These sensors are programmed on another ESP8266 board, and send their 
** data every 2 seconds.
*/
void callback(char *byteArraytopic, byte *byteArrayPayload, unsigned int length) {

  String topic;
  topic = String(byteArraytopic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

// Check which topic is received among the subscribed topics (see reconnect function to see all subscribed topics).
  if (topic == mqtt_topic) {  
    payload = "";             // clear the payload to not include any previously loaded messages in the for loop.
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i]; // inputs the received message into payload.
    }
    Serial.println(payload); // print the current payload in the serial monitor for debugging.
    lcd.clear(); // clear display.

    // variables for the messages received from the cloud that can be displayed on the LCD later on.
    String name = getValue(payload, ';', 0); 
    String temp = getValue(payload, ';', 1);
    String hum = getValue(payload, ';', 2);
    
    // display all temperature, humidity and name readings on the LCD display and serial monitor.
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
  char paybuffer[10];  // Buffer to store formatted value.
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
  char paybuffer[10];  // Buffer to store formatted value.
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

// Setup for the wifi. This function makes sure that the ESP8266 is connected to wifi.
void setup_wifi() {
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

/* 
** reconnect function that makes sure to reconnect to the server in case of disconnect. 
** Also makes sure to subscribe to all relevant topics
*/

void reconnect() {
  // Continues until client is connected succesfully.
  while (!client.connected()) {
    Serial.print("Forsøger at oprette MQTT forbindelse...");

    if (client.connect("HubClient", mqtt_user, mqtt_pass)) {  // connects to MQTT client with user and password.
      Serial.println("connected");
      client.subscribe(mqtt_topic);
      client.subscribe(mqtt_topic_ST);
      client.subscribe(mqtt_topic_SH);
      client.subscribe(mqtt_topic_CT);
      client.subscribe(mqtt_topic_CH);
    } 
    // if connections fails then warn the user and try again.
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Waits 5 seconds before trying again.
      delay(5000);
    }
  }
}

// setup for the RFID scanner.
void setupscanner(){
  rfid.PCD_Init();     // Init MFRC522

  Wire.begin(D4, D1);  
  lcd.init();
  lcd.backlight();

  Serial.println(F("This code scan the MIFARE Classsic NUID."));

  lcd.setCursor(0, 1);
  lcd.clear();
  lcd.backlight();

  setup_wifi();   

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  
}
