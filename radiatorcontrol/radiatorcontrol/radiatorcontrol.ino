/*
written by Anel Hodza
this code is for the radiator control and controls the servo motor (representing a radiator valve)
using the readings from the temperature sensor.
Code still needs to be able to get a desired temperature from our database, instead of hardcoded
temperature readings. This will be added in the next iteration
*/

#include <Servo.h>  // Include the Servo library

Servo myServo;  // Create a Servo object

int sensorPin = A0;   // select the input pin for the temperature sensor
int sensorValue = 0;  // variable to store the value coming from the sensor
int dataFromServer = 25;


void setup() {
  myServo.attach(3);   // Attach the servo to pin D3
  myServo.write(0);    // Set the servo to the starting position (0 degrees)
  Serial.begin(9600);  // Initialize serial communication for debugging
}

void loop() {
  radiatorControl(sensorValue, dataFromServer);
}



void radiatorControl(int input, int desiredTemp) {

  float voltage;      // variable used to calculate the temperature
  float Temperature;  // variable to store the calculated temperature

  input = analogRead(sensorPin);   // reading analog pin for the temperature sensor
  voltage = input * 5.0 / 1023.0;  // calculating voltage
  Temperature = voltage * 100;     // calculating the temperature

   if (Temperature < desiredTemp) { // if temperature is less than the desired temp turn on valve
      myServo.write(180);  // set servo to 180 degrees to indicate a valve turning on
    } 
    else if (Temperature > (desiredTemp+2)) { // else if temperature is more than two degrees over the desired temp turn off valve
      myServo.write(0);  // Set servo to 0 degrees to indicate a valve turning off
  }
  delay(1000);  // wait for 1 second between each reading


  // Debugging: Print the sensor value and temperature
  Serial.print("Sensor Value: ");
  Serial.print(input);
  Serial.print(" -> temperature: ");
  Serial.println(Temperature);
}
