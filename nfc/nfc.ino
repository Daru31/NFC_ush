#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>


// ESP32용 SPI 핀 정의
#define PN532_SCK  (18)
#define PN532_MOSI (23)
#define PN532_SS   (5)
#define PN532_MISO (19)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);


void setup(void) {
  Serial.begin(115200); // ESP32에서는 일반적으로 115200 사용
  delay(100); // 시리얼 안정화를 위한 짧은 대기

  Serial.println("Initializing NFC reader...");
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1); // 무한 루프로 대기
  }
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  Serial.println("Waiting for an ISO14443A Card...");

}

String getUID() {
  uint8_t uid[7] = {0};
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    Serial.println("Card detected");
    Serial.print("UID: ");
    String uidStr = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      Serial.print(" ");
      uidStr += String(uid[i], HEX) + " ";
    }
    Serial.println();
    return uidStr;
  } else {
    return "No card detected";
  }
}

void loop(void) {
  String uid = getUID();
  delay(1000);
}
