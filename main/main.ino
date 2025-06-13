#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi Info
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Firebase Project Info
#define API_KEY "YOUR_FIREBASE_API_KEY"
#define FIREBASE_PROJECT_ID "YOUR_PROJECT_ID"
#define USER_EMAIL "YOUR_FIREBASE_EMAIL"
#define USER_PASSWORD "YOUR_FIREBASE_PASSWORD"

// Firestore location 
#define COLLECTION_NAME "users"

// NFC SPI pins
#define PN532_SCK  (18)
#define PN532_MOSI (23)
#define PN532_SS   (5)
#define PN532_MISO (19)

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// NFC
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Firebase 
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");
}

void initFirebase() {
  config.api_key = API_KEY;
  config.project_id = FIREBASE_PROJECT_ID;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Connecting to Firebase...");
  while (!Firebase.ready()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Firebase ready.");
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");

  connectWiFi();
  initFirebase();

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1);
  }

  Serial.println("PN532 ready.");
  nfc.SAMConfig();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready for NFC");
}

String readUID() {
  uint8_t uid[7];
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    String uidStr = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) uidStr += "0"; // 0x0A → "0A"
      uidStr += String(uid[i], HEX);
      if (i < uidLength - 1) uidStr += " ";
    }
    uidStr.toUpperCase(); 
    Serial.print("UID: ");
    Serial.println(uidStr);
    return uidStr;
  }
  return "";
}

void checkUIDinFirestore(String uid) {
  String documentPath = COLLECTION_NAME + String("/") + uid;

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str())) {
    if (fbdo.httpCode() == 200) {
      Serial.println("Yes");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access: Yes");
    } else {
      Serial.println("No match");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No match");
    }
  } else {
    Serial.println("Error accessing Firestore");
    Serial.println(fbdo.errorReason());
  }
}

void loop() {
  String uid = readUID();
  if (uid != "") {
    checkUIDinFirestore(uid);
    delay(3000); 
  }
}
