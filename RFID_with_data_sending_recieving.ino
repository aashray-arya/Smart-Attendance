#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include "WiFi.h"
#include <HTTPClient.h>
#include <ESP32Servo.h>

#define SS_PIN 21
#define RST_PIN 22
//MISO 19
//MOSI 23
//SDA 21
//SCK 18
#define relay 2

int blockNum1 = 1;
int blockNum2 = 2;

byte bufferLen = 18;
byte readBlockData1[18];
byte rb1[8];
byte readBlockData2[18];
byte rb2[8];
Servo servo1;
int number=0;


MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
//for spreadsheet
const uint8_t fingerprint[20] = {0x4a, 0x2e, 0xb2, 0xa8, 0x29, 0x12, 0x9a, 0xca, 0xac, 0xe1, 0xe0, 0xf4, 0xa0, 0x6c, 0x74, 0x4b, 0x4b, 0x7d, 0x5b, 0xab};

const String readurl="https://script.google.com/macros/s/AKfycbwAy3YBNFS1xsg1pOKevCuaRb-w2vOXxFdaC6CwjoPOFHubX1CPi09Q45VMwUjx-Hqc/exec?read";
const String data1 = "https://script.google.com/macros/s/AKfycbwigW0XA4ceq4NiVx8izCHN6RpvUz_Kc6Xpsy4QLY4k5wVkY3ppoci-n-gdFuH1we_0/exec?reg1=";
String data2;
WiFiClient client;
// WiFi
const char* ssid = "D_406";              // Your personal network SSID
const char* wifi_password = "98609860"; // Your personal network password


void setup() {
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  servo1.attach(13);
  connect_WiFi();
  pinMode(relay,OUTPUT);
  digitalWrite(relay,LOW);
}

void loop()
{
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock1(blockNum1, readBlockData1);
  ReadDataFromBlock2(blockNum2, readBlockData2);
  /* If you want to print the full memory dump, uncomment the next line */
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  /* Print the data read from block */
  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum1);
  Serial.print(blockNum2);
  Serial.print(F(" --> "));
  for (int j = 0 ; j < 8 ; j++)
  {
    Serial.write(readBlockData1[j]);
    rb1[j]=readBlockData1[j];
  }
  for (int j = 0 ; j < 8 ; j++)
  {
    Serial.write(readBlockData2[j]);
    rb2[j]=readBlockData2[j];
    
  }
  //Serial.println(data2);
  //WiFi + Sending to google sheet
  if (WiFi.status() == WL_CONNECTED)
  {
    //std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    // client->setInsecure();

    data2 = data1 + String((char*)readBlockData1) + String((char*)readBlockData2);
    //Serial.println(data2.length());
    data2.remove(126,14);
    data2.trim();
    //Serial.println(data2);

    HTTPClient http;
    http.begin(data2);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);    
    }
    //---------------------------------------------------------------------
    http.end();
    if(httpCode>-1){
    servo1.write(90);
    Serial.println("Door Unlocked");
    delay(5000);
    servo1.write(0);
    Serial.println("Door locked");
    readfrmspreadsheet();
    }
  }
  Serial.println("number=");
  Serial.println(number);
  if(number<1){
    digitalWrite(relay,LOW);
    Serial.println("mains off");
  }
  else{
    digitalWrite(relay,HIGH);
    Serial.println("mains on");
    }
}


void connect_WiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void ReadDataFromBlock1(int blockNum, byte readBlockData1[])
{
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Authentication success");
  }

  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData1, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Block was read successfully");
  }
}

void ReadDataFromBlock2(int blockNum, byte readBlockData2[])
{
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Authentication success");
  }

  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData2, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Block was read successfully");
  } 
}
void readfrmspreadsheet(void){
  HTTPClient http;
  Serial.print("Making a request to read the number of students in class");
  http.begin(readurl.c_str()); //Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  String payload;
    if (httpCode > 0) { //Check for the returning code
        payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
        number=payload.toInt();
    }
    else {
      Serial.println("Error on HTTP request");
      number=1;
    }
  http.end();
}
