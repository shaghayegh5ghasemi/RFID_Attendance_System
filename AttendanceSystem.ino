#include <SPI.h>
#include <MFRC522.h>
 
#define RST_PIN         D3         // Configurable, see typical pin layout above
#define SS_PIN          D8        // Configurable, see typical pin layout above

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char *ssid     = "Shaghayegh";
const char *password = "shagha771117";

int low=5;
int high=20;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


 
MFRC522 rfid(SS_PIN, RST_PIN);  // Create MFRC522 instance
// Init array that will store new NUID 
byte nuidPICC[4];

void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();   // Init MFRC522
  rfid.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));


  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("connected");
  timeClient.setTimeOffset(16200);
// Initialize a NTPClient to get time
  timeClient.begin();
  pinMode(D1,OUTPUT);
  pinMode(D0,OUTPUT);
}
 
void loop() {
  timeClient.update();
  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  }
  Serial.println("scanned");
  // Select one of the cards
  if ( ! rfid.PICC_ReadCardSerial()) {
    return;
  }
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Scan Time: ");
  Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
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
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  Serial.println(currentHour>low && currentHour<high);
  if(currentHour>low && currentHour<high){
    digitalWrite(D0,HIGH);
    delay(3000);
    digitalWrite(D0,LOW);
  }
  else{
    digitalWrite(D1,HIGH);
    delay(3000);
    digitalWrite(D1,LOW);
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
