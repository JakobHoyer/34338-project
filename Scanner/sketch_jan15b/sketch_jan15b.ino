/*
 * --------------------------------------------------------------------------------------------------------------------
 * Setup for NODE MCU used to communicate between RFID chip reader and backend.
 * --------------------------------------------------------------------------------------------------------------------
 * This code is used for sending UID numbers from the RFID reader to the server,
 * that then displays the current temperature and desired temperature on a LCD display.
 *  
 * Pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522                    ESP8266       
 *             Reader/PCD          
 * Signal      Pin                        Pin           
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST                        D3        
 * SPI SS      SDA(SS)                    D8        
 * SPI MOSI    MOSI                       D7       
 * SPI MISO    MISO                       D6      
 * SPI SCK     SCK                        D5       
 *
 * 
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>

#define SS_PIN D8
#define RST_PIN D3
//#define buttonPin D2

MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class


// Init array that will store new NUID
byte nuidPICC[4];
char hexString[9];

// variable for button
//const int buttonPin = D1;

// Søren preset
byte preset1[4] = { 0xA2, 0x9F, 0x8A, 0x3F };

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);




// Definerer id og password til netværksforbindelse som NodeMCU anvender
const char *ssid = "Jakob - iPhone";
const char *password = "kodekodeadad";
//const char* ssid = "ZEPPER";
//const char* password = "12344321";


// Definerer information til mqtt serveren
const char *mqtt_server = "maqiatto.com";          //navn på mqtt-server. Find navnet på cloudmqtt-hjemmesiden
const int mqtt_port = 1883;                        // Definerer porten
const char *mqtt_user = "s183668@student.dtu.dk";  // Definerer mqtt-brugeren
const char *mqtt_pass = "kodekodeadad";            // Definerer koden til mqtt-brugeren
const char *mqtt_topic = "s183668@student.dtu.dk/hub";
const char *mqtt_topic_ST = "s183668@student.dtu.dk/SensorToCloudTemp";
const char *mqtt_topic_SH = "s183668@student.dtu.dk/SensorToCloudHum";
const char *mqtt_topic_CT = "s183668@student.dtu.dk/CloudToSensorTemp";
const char *mqtt_topic_CH = "s183668@student.dtu.dk/CloudToSensorHum";
//

String payload;  // Definerer variablen 'payload' i det globale scope (payload er navnet på besked-variablen)



// Opretter en placeholder for callback-funktionen til brug senere. Den rigtige funktion ses længere nede.
void callback(char *byteArraytopic, byte *byteArrayPayload, unsigned int length);

// Opretter en klient der kan forbinde til en specifik internet IP adresse.
WiFiClient espClient;  // Initialiserer wifi bibloteket ESP8266Wifi, som er inkluderet under "nødvendige bibloteker"

// Opretter forbindelse til mqtt klienten:
PubSubClient client(mqtt_server, mqtt_port, callback, espClient);  // Initialiserer bibloteket for at kunne modtage og sende beskeder til mqtt. Den henter fra definerede mqtt server og port. Den henter fra topic og beskeden payload




void setup() {
  //pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  SPI.begin();         // Init SPI bus
  rfid.PCD_Init();     // Init MFRC522'
  Wire.begin(D4, D1);  // SDA=D4 (GPIO2), SCL=D1 (GPIO5)
  lcd.init();
  lcd.backlight();
  // Print a message to the LCD.
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  lcd.setCursor(0, 1);
  lcd.clear();

  lcd.backlight();  // Turn on backlight
                    /*
  lcd.setCursor(0, 1);
  lcd.print("D11");

  lcd.setCursor(4, 1);
  lcd.print("D21");

  lcd.setCursor(8, 1);
  lcd.print("D31");

  lcd.setCursor(12, 1);
  lcd.print("D41");
*/


  setup_wifi();                              // Kører WiFi loopet og forbinder herved.
  client.setServer(mqtt_server, mqtt_port);  // Forbinder til mqtt serveren (defineret længere oppe)
  client.setCallback(callback);              // Ingangsætter den definerede callback funktion hver gang der er en ny besked på den subscribede "cmd"- topic
}

void loop() {

  ////// LOOP /////////

  //if(checkButtonState(buttonPin)){
  // delay(50);

  // Hvis der opstår problemer med forbindelsen til mqtt broker oprettes forbindelse igen ved at køre client loop
  if (!client.connected()) {
    reconnect();
  }
  client.loop();



  for (int i = 0; i < 4; i++) {
    sprintf(&hexString[i * 2], "%02X", nuidPICC[i]);
  }

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;

  client.publish("s183668@student.dtu.dk/pub", hexString);
  hexString[8] = '\0';

  delay(100);





  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  } else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  //compareByteArrays(nuidPICC, preset1, 4);
}
//else
//{
//  Serial.println("loool");
//}
//}

/*
void compareByteArrays(byte *array1, byte *array2, size_t size) {
  bool areIdentical = true;  // Flag to track if arrays are identical

  for (size_t i = 0; i < size; i++) {
    if (array1[i] != array2[i]) {
      areIdentical = false;  // Set flag to false if a mismatch is found
      break;
    }
  }

  // Print the result
  if (areIdentical) {
    Serial.println("The arrays are identical.");
    lcd.setCursor(0, 0);  // Set the LCD cursor position
    lcd.print("Soerens card");
  } else {
    Serial.println("invalid card");
    lcd.setCursor(0, 0);  // Set the LCD cursor position
    lcd.print("Unknown card");
  }
}
*/
/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    //lcd.setCursor(0, 1);  // Set the LCD cursor position
    //lcd.print("UID: ");
    //for (byte i = 0; i < 4; i++) {

    //  if (nuidPICC[i] < 0x10) lcd.print("0");  // Add a leading zero for single-digit hex values
    //   lcd.print(nuidPICC[i], HEX);             // Print each byte in HEX
    //  if (i < 3) lcd.print("");                // Add a colon between bytes, except the last
    //}
  }
}


/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
    // ');
    // lcd.print(buffer[i], DEC);
  }
}

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


// Definerer callback funktionen der modtager beskeder fra mqtt
// OBS: den her funktion kører hver gang MCU'en modtager en besked via mqtt
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

///////// CALLBACK SLUT /////////

//
//
//
//
//
//

/////// OPSÆTNING AF WIFI-FORBINDELSE  ///////////


// Opretter forbindelse til WiFi
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


// Opretter forbindelse til mqtt server. Dette gentages ved manglende forbindelse til WiFi, server osv.
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