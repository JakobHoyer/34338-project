void setup() {
    Serial.begin(115200);  // Initialize Serial Monitor at 115200 baud
    Serial.println("ESP8266 ADC Receiver Started...");
    pinMode(D5,OUTPUT);
    pinMode(D4,OUTPUT);
}

void fanturnon(){
digitalWrite(D5,HIGH);
}

void radiatorturnon(){
  digitalWrite(D4, HIGH);
}

void neutral(){
   digitalWrite(D4, LOW);
   digitalWrite(D5, LOW);
}

void loop() {
    int adcValue = analogRead(A0);  // Read the ADC value (0-1023)
    
    Serial.print("ADC Value: ");  // Print label
    int redadcval = adcValue/20;
    Serial.println(redadcval);     // Print ADC value

  if(redadcval >= 27){
    fanturnon();
  }
  else if(redadcval <= 23){
    radiatorturnon();
  }
  else{
    neutral();
  }

    
    delay(10);  // Delay for readability (adjust as needed)
}