#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <RF24.h>

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
RF24 radio(9, 10); // CE, CSN pin 

const byte address[5] = {0xE1, 0xF0, 0xF0, 0xF0, 0xF0};

void setup(void) {
  Serial.begin(115200);
  // pn532 output 
  while (!Serial) delay(10);

  Serial.println("Initializing NFC reader...");
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1);
  }
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  Serial.println("Waiting for an ISO14443A Card...");

  // nrf24l01 output 
  if (!radio.begin()) {
    Serial.println("Failed to initialize nRF24L01!");
    while (1);
  }
  // radio.setAutoAck(false); // for arduino rf test
  radio.setPALevel(RF24_PA_MAX); 
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(0x76);

  radio.setRetries(5, 15); // 5 times retry 
  radio.openWritingPipe(address);
  radio.stopListening(); 
}

String senduid() {
  uint8_t uid[7] = {0};
  uint8_t uidLength;
  
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    Serial.println("Card detected");
    String uidStr = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      uidStr += String(uid[i], HEX) + " ";
    }

    const char* text = uidStr.c_str();
    bool ok = radio.write(text, strlen(text) + 1);
    Serial.println(text);
    return uidStr;
  } else {
    return "No card detected";
  }
}

void loop(void) {
  senduid();
  delay(1000);
}
