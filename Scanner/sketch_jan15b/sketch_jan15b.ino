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
#include "scanner.h"

void setup() {
  //pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  SPI.begin();       
  setupscanner();
}

void loop() {

  ////// LOOP /////////

  // Hvis der opstår problemer med forbindelsen til mqtt broker oprettes forbindelse igen ved at køre client loop
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;


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

  for (int i = 0; i < 4; i++) {
    sprintf(&hexString[i * 2], "%02X", nuidPICC[i]);
  }

  client.publish("s183668@student.dtu.dk/pub", hexString);
  hexString[8] = '\0';

  delay(100);

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

}
