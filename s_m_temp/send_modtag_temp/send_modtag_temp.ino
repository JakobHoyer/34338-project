//////////// Initiering ///////////

//inkluderer nødvendige bibloteker
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

#define DHT_PIN D2
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// Definerer id og password til netværksforbindelse som NodeMCU anvender
const char* ssid = "Jakob - iPhone";    //Indsæt navnet på jeres netværk her
const char* password = "kodekodeadad";  //Indsæt password her

//const char* ssid = "Abisu";         //Indsæt navnet på jeres netværk her
//const char* password = "00000000";  //Indsæt password her

// Definerer information til mqtt serveren
const char* mqtt_server = "maqiatto.com";          //navn på mqtt-server. Find navnet på cloudmqtt-hjemmesiden
const int mqtt_port = 1883;                        // Definerer porten
const char* mqtt_user = "s183668@student.dtu.dk";  // Definerer mqtt-brugeren
const char* mqtt_pass = "kodekodeadad";            // Definerer koden til mqtt-brugeren
const char* mqtt_topic_temp = "s183668@student.dtu.dk/CloudToSensorTemp";
const char* mqtt_topic_hum = "s183668@student.dtu.dk/CloudToSensorHum";
//

String payload;  // Definerer variablen 'payload' i det globale scope (payload er navnet på besked-variablen)

//Important, use pwm capable pins
const int RedPin = D3;
const int GreenPin = D6;
const int BluePin = D5;

//Important, use analog pin to read temperture.
const int TempPin = A0;

// The used MCU Nude, has max voltage of 3.3v, it doesnt matter, it is for the equation.
const float referenceVoltage = 5.0;

// Initialisation of variables.
int BlueIntensity = 0;
int GreenIntensity = 0;
int sensorValue = 0;
float voltage = 0.0;
float Temp = 0.0;

//to account for nois, we use avarge temp reading.
float Temp_Avg = 0;

// Default prefered temperture
float temp_pref_low = 21.0, temp_pref_high = 23.0;

/////// INITIERING SLUT //////////

//
//
//
//
//
//

/////// FUNKTIONSOPSÆTNING ////////


// Opretter en placeholder for callback-funktionen til brug senere. Den rigtige funktion ses længere nede.
void callback(char* byteArraytopic, byte* byteArrayPayload, unsigned int length);

// Opretter en klient der kan forbinde til en specifik internet IP adresse.
WiFiClient espClient;  // Initialiserer wifi bibloteket ESP8266Wifi, som er inkluderet under "nødvendige bibloteker"

// Opretter forbindelse til mqtt klienten:
PubSubClient client(espClient);  // Initialiserer bibloteket for at kunne modtage og sende beskeder til mqtt. Den henter fra definerede mqtt server og port. Den henter fra topic og beskeden payload

/////// FUNKTIONSOPSÆTNING SLUT /////////

//
//
//
//
//
//

///////// CALLBACKFUNKTION ////////

// Definerer callback funktionen der modtager beskeder fra mqtt
// OBS: den her funktion kører hver gang MCU'en modtager en besked via mqtt
void callback(char* byteArraytopic, byte* byteArrayPayload, unsigned int length) {

  // Konverterer indkomne besked (topic) til en string:
  String topic;
  topic = String(byteArraytopic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  // Konverterer den indkomne besked (payload) fra en array til en string:
  // Topic == Temperaturmaaler, Topic == Kraftsensor
  if (topic == mqtt_topic_temp) {  // OBS: der subscribes til et topic nede i reconnect-funktionen. I det her tilfælde er der subscribed til "Test". Man kan subscribe til alle topics ved at bruge "#"
    payload = "";             // Nulstil payload variablen så forloopet ikke appender til en allerede eksisterende payload
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }

    // Modtagne temperature som Konverteres fra string til float:
    float received_temp = payload.toFloat();
    Serial.print("Received temperature: ");
    Serial.println(received_temp);

    if (received_temp > 0) {  // Sikrer at temperaturen er valid
      Temp_interval(received_temp);
    }

    //Serial.println(payload);
    //client.publish("mqtt", String(payload).c_str()); // Publish besked fra MCU til et valgt topic. Husk at subscribe til topic'et i NodeRed.
  } 
  
  else if (topic == mqtt_topic_hum) {
    payload = "";             // Nulstil payload variablen så forloopet ikke appender til en allerede eksisterende payload
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }

    // humidity kode
    float received_hum = payload.toFloat();
    Serial.print("Received humidity: ");
    Serial.println(received_hum);
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
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Kunne ikke forbinde til WiFi!");
    return;
  }
  if (payload.length() == 0) {
    Serial.println("Tom payload modtaget!");
    return;
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

    if (client.connect("sensor", mqtt_user, mqtt_pass)) {  // Forbinder til klient med mqtt bruger og password
      Serial.println("connected");
      // Derudover subsribes til topic "Test" hvor NodeMCU modtager payload beskeder fra
      client.subscribe(mqtt_topic_hum);
      client.subscribe(mqtt_topic_temp);
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

///////// OPSÆTNING AF WIFI SLUT /////////

//
//
//
//
//
//

///////// SETUP ///////////
void setup() {

  Serial.begin(115200);  // Åbner serial porten og sætter data raten til 115200 baud
  delay(1000);

  // Set pin modes
  pinMode(RedPin, OUTPUT);
  pinMode(GreenPin, OUTPUT);
  pinMode(BluePin, OUTPUT);

  Serial.println("ESP8266 ADC Receiver Started...");
  pinMode(D1,OUTPUT);
  pinMode(D4,OUTPUT);

  setup_wifi();                              // Kører WiFi loopet og forbinder herved.
  client.setServer(mqtt_server, mqtt_port);  // Forbinder til mqtt serveren (defineret længere oppe)
  client.setCallback(callback);              // Ingangsætter den definerede callback funktion hver gang der er en ny besked på den subscribede "cmd"- topic


  dht.begin();
  float humidity = dht.readHumidity();
  Serial.print("Air humidity: ");
  Serial.println(humidity);
  //delay(1000);

  
}
//////// SETUP SLUT ////////

//
//
//
//
//
//

/////// LOOP /////////

void loop() {

  // Hvis der opstår problemer med forbindelsen til mqtt broker oprettes forbindelse igen ved at køre client loop
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  float temp = Temp_sens();
  String tempStr = String(temp, 2);       // Konverter float til string
  
  client.publish("s183668@student.dtu.dk/SensorToCloudTemp", tempStr.c_str());  // Konverter til C-streng


  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println("Error: Failed to read humidity!");
    return;
  }
  String humidityStr = String(humidity, 2);
  
  client.publish("s183668@student.dtu.dk/SensorToCloudHum", humidityStr.c_str());
  Serial.print("Humidity: ");
  Serial.println(humidity);

  //fane kode
  digitalWrite(D1, HIGH);
  delay(1000);
  digitalWrite(D1, LOW);
  delay(1000);

  if(temp >= temp_pref_high){
    fanturnon();
  }
  else if(temp <= temp_pref_low){
    radiatorturnon();
  }
  else{
    neutral();
  }

}

//////// Loop slut ////////

float Temp_sens() {
  Temp_Avg = 0;
  for (int i = 0; i < 20; i++) {
    sensorValue = analogRead(TempPin);
    voltage = sensorValue * 3.0 / 1023.0;
    Temp = voltage / 0.01;
    //Serial.println(Temp);
    Temp_Avg = Temp_Avg + Temp;
    LED_Indication(Temp);
    delay(100);
  }
  Temp = (Temp_Avg / 20);
  Serial.print("Temperature: ");
  Serial.println(Temp);
  return Temp;
}

//The desired temperture interval
void Temp_interval(float temp_pref) {
  temp_pref_low = temp_pref - 2;
  temp_pref_high = temp_pref + 2;
}

void LED_Indication(float current_temp) {
  // green when in prefered temp range
  if (current_temp >= temp_pref_low && current_temp <= temp_pref_high) {
    analogWrite(RedPin, 0);
    analogWrite(GreenPin, 255);
    analogWrite(BluePin, 0);
  }

  // when lower than prefered (mix of green and blue when between -5 and lower preference)
  else if (current_temp < temp_pref_low) {
    // Intensity of blue
    BlueIntensity = map(current_temp, temp_pref_low - 5, temp_pref_low, 255, 0);
    BlueIntensity = constrain(BlueIntensity, 0, 255);

    //Intensisty of green
    GreenIntensity = map(current_temp, temp_pref_low, temp_pref_low - 5, 255, 0);
    GreenIntensity = constrain(GreenIntensity, 0, 255);

    analogWrite(RedPin, 0);
    analogWrite(GreenPin, GreenIntensity);
    analogWrite(BluePin, BlueIntensity);
  }

  // When higer than prefered orange when between +5 and upper preference
  else {
    //Intensity of green
    GreenIntensity = map(current_temp, temp_pref_high, temp_pref_high + 5, 100, 0);
    GreenIntensity = constrain(GreenIntensity, 0, 100);

    analogWrite(RedPin, 255);
    analogWrite(GreenPin, GreenIntensity);
    analogWrite(BluePin, 0);
  }
}

void fanturnon(){
digitalWrite(D1,HIGH);
}

void radiatorturnon(){
  digitalWrite(D4, HIGH);
}

void neutral(){
   digitalWrite(D1, LOW);
   digitalWrite(D4, LOW);
}