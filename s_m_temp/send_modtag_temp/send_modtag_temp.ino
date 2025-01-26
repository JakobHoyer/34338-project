/**
 * @file send_modtag_temp.ino
 * @brief This file contains the code for Temperture sensor and control module.
 * 
 * The code connects to WiFi, subscribes to MQTT topics, reads sensor data (temperature prefrence and humidity prefrence), 
 * controls a servo motor, indicates wether to heat or cool the room and indicates temperature status using an RGB LED.
 * @author Abbas Heidari, Daniel Heiðar Qasemiani & Jakob Høyer Madsen
 */

/** 
 * --------------------------------------------------------------------------------------------------------------------
 * Includes & Definitions
 * --------------------------------------------------------------------------------------------------------------------
 */

#include <ESP8266WiFi.h>        ///< WiFi library for ESP8266
#include <PubSubClient.h>       ///< MQTT library
#include <ESP8266HTTPClient.h>  ///< HTTP client library for ESP8266
#include <DHT.h>                ///< Library for DHT temperature/humidity sensors
#include "ESP8266_ISR_Servo.h"  ///< Non-blocking Servo library for ESP8266

/// GPIO pin for the servo
#define SERVO_PIN D7

/// GPIO pin for the DHT sensor
#define DHT_PIN D2
/// DHT sensor type
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);  ///< DHT sensor object

/**
 * --------------------------------------------------------------------------------------------------------------------
 * WiFi & MQTT Configuration
 * --------------------------------------------------------------------------------------------------------------------
 */
/// WiFi SSID
const char* ssid = "Jakob - iPhone";
/// WiFi Password
const char* password = "kodekodeadad";

// Alternative network configuration (commented out)
// const char* ssid = "Abisu";
// const char* password = "00000000";

/// MQTT server address
const char* mqtt_server = "maqiatto.com";
/// MQTT server port
const int mqtt_port = 1883;
/// MQTT username
const char* mqtt_user = "s183668@student.dtu.dk";
/// MQTT password
const char* mqtt_pass = "kodekodeadad";

/// MQTT topic for receiving desired temperature from the cloud
const char* mqtt_topic_temp = "s183668@student.dtu.dk/CloudToSensorTemp";
/// MQTT topic for receiving desired humidity from the cloud
const char* mqtt_topic_hum = "s183668@student.dtu.dk/CloudToSensorHum";

/**
 * @brief Global string for storing the received MQTT payload.
 */
String payload;

/**
 * --------------------------------------------------------------------------------------------------------------------
 * Pin Assignments & Sensor Variables
 * --------------------------------------------------------------------------------------------------------------------
 */
/// Red LED pin (PWM capable)
const int RedPin = D3;
/// Green LED pin (PWM capable)
const int GreenPin = D6;
/// Blue LED pin (PWM capable)
const int BluePin = D5;

/// Analog pin used to read temperature
const int TempPin = A0;

/// The reference voltage for the ADC calculations (for an ESP8266 NodeMCU board, ~3.0–3.3V)
const float referenceVoltage = 3.3;

/**
 * @brief Global initilation of measured or calculated sensor values.
 */
int sensorValue = 0;        ///< Latest ADC reading from TempPin
float voltage = 0.0;        ///< Calculated voltage from ADC
float Temp = 0.0;           ///< Latest temperature reading
float Temp_Avg = 0.0;       ///< Average temperature reading
float received_temp = 0.0;  ///< Desired temperature received from MQTT

/**
 * @brief Global initilation of RGB LED color intensities.
 */
int BlueIntensity = 0;   ///< Intensity for the blue channel
int GreenIntensity = 0;  ///< Intensity for the green channel

/**
 * @brief Global initilation of default temperature bounds around the desired temperature.
 */
float temp_pref_low = 21.0;   ///< Lower bound for preferred temperature
float temp_pref_high = 23.0;  ///< Upper bound for preferred temperature

/**
 * @brief Indices and states used for servo control.
 */
int servoIndex = -1;
int lastState = 0;
int state = 0;

/**
 * --------------------------------------------------------------------------------------------------------------------
 * MQTT & WiFi Client Objects
 * --------------------------------------------------------------------------------------------------------------------
 */
WiFiClient espClient;            ///< WiFi client object
PubSubClient client(espClient);  ///< MQTT client object

/**
 * --------------------------------------------------------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------------------------------------------------------
 */
void callback(char* byteArraytopic, byte* byteArrayPayload, unsigned int length);
void setup_wifi();
void reconnect();
float Temp_sens();
void Temp_interval(float temp_pref);
void LED_Indication(float current_temp);
void fanturnon();
void radiatorturnon();
void neutral();
void turnServo180();
void turnServo0();

/**
 * @brief MQTT callback function.
 * 
 * This function is called whenever a new message is received on a subscribed topic.
 * It processes the incoming topic and payload, updating relevant global variables 
 * and taking actions such as controlling a servo based on the received temperature command.
 * 
 * @param byteArraytopic Topic of the received MQTT message.
 * @param byteArrayPayload Payload of the received MQTT message.
 * @param length Length of the payload.
 */
void callback(char* byteArraytopic, byte* byteArrayPayload, unsigned int length) {
  String topic = String(byteArraytopic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // If the received topic matches the desired temperature topic
  if (topic == mqtt_topic_temp) {
    // Clear the old payload
    payload = "";
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }

    // Convert the received string to a float
    received_temp = payload.toFloat();
    Serial.print("Received temperature: ");
    Serial.println(received_temp);

    // If the temperature is valid, adjust the preference range
    if (received_temp > 0) {
      Temp_interval(received_temp);
    }

    Serial.println(payload);
    client.publish("mqtt", String(payload).c_str());
  }
  // If the received topic matches the desired humidity topic
  else if (topic == mqtt_topic_hum) {
    payload = "";
    for (int i = 0; i < length; i++) {
      payload += (char)byteArrayPayload[i];
    }

    float received_hum = payload.toFloat();
    Serial.print("Received humidity: ");
    Serial.println(received_hum);
  }
}

/**
 * @brief Connects the ESP8266 to the specified WiFi network.
 * 
 * Tries to connect to the WiFi network within a given timeout period. 
 * Prints debug information to the Serial monitor.
 */
void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime) < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Could not connect to WiFi!");
    return;
  }
  if (payload.length() == 0) {
    Serial.println("Empty payload received!");
    return;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Attempts to reconnect to the MQTT broker if the connection is lost.
 * 
 * This function blocks until the device is successfully reconnected to the MQTT broker,
 * subscribing again to the necessary topics after reconnection.
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect to the MQTT broker with credentials
    if (client.connect("sensor", mqtt_user, mqtt_pass)) {
      Serial.println("connected");

      // Subscribe to relevant MQTT topics
      client.subscribe(mqtt_topic_hum);
      client.subscribe(mqtt_topic_temp);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/**
 * @brief Arduino setup function, runs once at startup.
 * 
 * Initializes serial communication, servo, pin modes, WiFi, MQTT client, and DHT sensor.
 */
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize the servo using the ISR-based library
  servoIndex = ISR_Servo.setupServo(SERVO_PIN, 500, 2500);
  if (servoIndex == -1) {
    Serial.println("Error: Could not set up the servo.");
  } else {
    Serial.println("Servo initialized.");
  }

  // Set pin modes
  pinMode(RedPin, OUTPUT);
  pinMode(GreenPin, OUTPUT);
  pinMode(BluePin, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D4, OUTPUT);

  Serial.println("ESP8266 ADC Receiver Started...");

  // Connect to WiFi
  setup_wifi();

  // Configure MQTT server and callback
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Initialize DHT sensor
  dht.begin();
  float humidity = dht.readHumidity();
  Serial.print("Air humidity: ");
  Serial.println(humidity);
}

/**
 * @brief Arduino loop function, runs continuously.
 * 
 * Maintains the MQTT connection, reads temperature and humidity, and controls 
 * fan, radiator, and servo based on the measured and desired temperature.
 */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read and publish temperature
  float temp = Temp_sens();
  String tempStr = String(temp, 2);
  client.publish("s183668@student.dtu.dk/SensorToCloudTemp", tempStr.c_str());

  // Read and publish humidity
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println("Error: Failed to read humidity!");
    return;
  }
  String humidityStr = String(humidity, 2);
  client.publish("s183668@student.dtu.dk/SensorToCloudHum", humidityStr.c_str());

  // Control fan (Blue LED) or radiator(Servo and Red led) based on measured vs. desired temperature
  if (temp >= received_temp + 2) {
    Serial.println("Too Warm");
    fanturnon();
    Serial.println("Fan on");
    turnServo0();
  } else if (temp <= received_temp - 2) {
    Serial.println("Too Cold");
    radiatorturnon();
    Serial.println("Radiator on");
    turnServo180();
  } else {
    Serial.println("Neutral");
    neutral();
  }

  delay(10);
}

/**
 * @brief Reads and averages temperature from the ADC pin.
 * 
 * Reads the ADC multiple times, applies a mapping from voltage to temperature, 
 * updates the LED indication based on the measured temperature, and returns the average temperature.
 * 
 * @return The averaged temperature in °C.
 */
float Temp_sens() {
  Temp_Avg = 0;
  for (int i = 0; i < 20; i++) {
    sensorValue = analogRead(TempPin);
    voltage = sensorValue * referenceVoltage / 1023.0;
    Temp = voltage / 0.01;  // Convert voltage to temperature (based on a specific sensor equation)
    Temp_Avg += Temp;
    LED_Indication(Temp);
    delay(100);
  }
  Temp = (Temp_Avg / 20.0);
  Serial.print("Temperature: ");
  Serial.println(Temp);
  return Temp;
}

/**
 * @brief Adjusts the preferred temperature interval.
 * 
 * Given a desired temperature, this function sets the lower and upper 
 * bounds for comfortable temperature range (±2°C around desired temperature).
 * 
 * @param temp_pref The desired temperature setpoint.
 */
void Temp_interval(float temp_pref) {
  temp_pref_low = temp_pref - 2;
  temp_pref_high = temp_pref + 2;
}

/**
 * @brief Controls the RGB LED to reflect how the current temperature compares to the desired range.
 * 
 * - Green when within the preferred range.
 * - Blue gradient when below the preferred range.
 * - Red/Orange gradient when above the preferred range.
 * 
 * @param current_temp The current measured temperature.
 */
void LED_Indication(float current_temp) {
  if (current_temp >= temp_pref_low && current_temp <= temp_pref_high) {
    // Within preferred range, Green
    analogWrite(RedPin, 0);
    analogWrite(GreenPin, 255);
    analogWrite(BluePin, 0);
  } else if (current_temp < temp_pref_low) {
    // Below preferred range, Blue/Green mix
    BlueIntensity = map(current_temp, (temp_pref_low - 5), (temp_pref_low), 255, 0);
    BlueIntensity = constrain(BlueIntensity, 0, 255);
    GreenIntensity = map(current_temp, (temp_pref_low), (temp_pref_low - 5), 255, 0);
    GreenIntensity = constrain(GreenIntensity, 0, 255);

    analogWrite(RedPin, 0);
    analogWrite(GreenPin, GreenIntensity);
    analogWrite(BluePin, BlueIntensity);
  } else {
    // Above preferred range, Red/Green mix (orange) to Red
    GreenIntensity = map(current_temp, (temp_pref_high), (temp_pref_high + 5), 100, 0);
    GreenIntensity = constrain(GreenIntensity, 0, 100);

    analogWrite(RedPin, 255);
    analogWrite(GreenPin, GreenIntensity);
    analogWrite(BluePin, 0);
  }
}

/**
 * @brief Turns the fan on (connected to pin D1) and turns the radiator off (pin D4).
 */
void fanturnon() {
  digitalWrite(D1, HIGH);
  digitalWrite(D4, LOW);
}

/**
 * @brief Turns the radiator on (connected to pin D4) and turns the fan off (pin D1).
 */
void radiatorturnon() {
  digitalWrite(D4, HIGH);
  digitalWrite(D1, LOW);
}

/**
 * @brief Turns both fan and radiator off.
 */
void neutral() {
  digitalWrite(D1, LOW);
  digitalWrite(D4, LOW);
  turnServo0();
}

/**
 * @brief Moves the servo to 180 degrees position.
 * 
 * This might represent a fully open or a fully closed valve, depending on mechanical setup.
 */
void turnServo180() {
  if (servoIndex != -1) {
    Serial.println("Servo moved to 180 degrees.");
    ISR_Servo.setPosition(servoIndex, 180);
  }
}

/**
 * @brief Moves the servo to 0 degrees position.
 * 
 * This might represent a fully open or fully closed valve, depending on mechanical setup.
 */
void turnServo0() {
  if (servoIndex != -1) {
    Serial.println("Servo moved to 0 degrees.");
    ISR_Servo.setPosition(servoIndex, 0);
  }
}
