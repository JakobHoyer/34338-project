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


/** @defgroup MainSketch Main Arduino Sketch
 *  @{
 *  @file scanner.ino
 *  @brief  This code is used for sending UID numbers from the RFID reader to the server, that then displays the current temperature and desired temperature on a LCD display
 *  @author Anel Hodza and Søren Zepernick         
 *  @}
 */

#include <Arduino.h>
#include "scanner.h"

/**
 * @brief Arduino setup function, called once at startup.
 * 
 * This function initializes the scanner module and sets up all necessary components
 * such as the WiFi connection and MQTT client.
 */
void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting Arduino Project"));

  setupscanner(); // Initialize the scanner module.
}

/**
 * @brief Arduino loop function, called repeatedly.
 * 
 * This function handles MQTT client operations and RFID card scanning in a continuous loop.
 */
void loop() {
  if (!client.connected()) {
    reconnect(); // Reconnect to MQTT server if disconnected.
  }
  client.loop(); // Process MQTT messages.

  // Look for new RFID cards.
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return; // Return if no new card is detected.
  }

  // verifies that card is read correctly.
  if (!rfid.PICC_ReadCardSerial())
    return;  

  // Print the UID of the scanned RFID card.
  Serial.print(F("RFID UID: "));
  Serial.print(F("In hex: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  Serial.print(F("In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Puts the scanned UID number into a string to be transmitted to the cloud in hexidecimal format.
  for (int i = 0; i < 4; i++) {
    sprintf(&hexString[i * 2], "%02X", nuidPICC[i]);
  }

  // 0 termination 
  hexString[8] = '\0';
  // publish the message to the cloud
  client.publish("s183668@student.dtu.dk/pub", hexString);

  // Halt the RFID card to allow scanning of new cards.
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
