//////////// Initialisation ///////////

//Important, use pwm capable pins
const int RedPin = 3;
const int GreenPin = 6;
const int BluePin = 5;

//Important, use analog pin to read temperture.
const int TempPin = A0;

// Default prefered temperture
int temp_pref_low = 21.0, temp_pref_high = 23.0;

// The used MCU Nude, has max voltage of 3.3v, it doesnt matter, it is for the equation.
const float referenceVoltage = 5.0;

// Initialisation of variables.
int BlueIntensity = 0;
int GreenIntensity = 0;
int sensorValue = 0;
float voltage = 0.0;
float Temp = 0.0;

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);

  // Set pin modes
  pinMode(RedPin, OUTPUT);
  pinMode(GreenPin, OUTPUT);
  pinMode(BluePin, OUTPUT);
}

void loop() {
  temp_pref_low = 0;
  temp_pref_high = 0;
  LED_Indication(Temp_sens());
}

float Temp_sens() {

  sensorValue = analogRead(TempPin);
  voltage = sensorValue * referenceVoltage / 1023.0;
  Temp = voltage / 0.01;
  return Temp;
}

void LED_Indication(float current_temp) {
  if (Serial.available() > 0) {
    // define current temp (to test we use serial monitor)
    float current_temp = Serial.parseFloat();


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
      Serial.print(GreenIntensity);

      analogWrite(RedPin, 255);
      analogWrite(GreenPin, GreenIntensity);
      analogWrite(BluePin, 0);
    }
  }
}