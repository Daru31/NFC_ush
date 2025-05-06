#include <SPI.h>
#include <RF24.h>

// CE, CSN 핀
RF24 radio(9, 10);

// 송신 주소 (수신측과 일치해야 함)
const byte address[5] = {0xE1, 0xF0, 0xF0, 0xF0, 0xF0};

void setup() {
  Serial.begin(9600);
  
  if (!radio.begin()) {
    Serial.println("nRF24L01 초기화 실패!");
    while (1);
  }
  // radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX); // 전원 부족시 HIGH나 MAX는 문제 발생
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(0x76);

  radio.setRetries(5, 15); // 최대 5회 재시도
  radio.openWritingPipe(address);
  radio.stopListening(); // 송신 모드
}

void loop() {
  const char text[] = "SELECT * FROM student_db";
  bool ok = radio.write(&text, sizeof(text));

  if (ok) {
    Serial.println("✅ 전송 성공");
  } else {
    Serial.println("❌ 전송 실패");
  }

  delay(1000); // 1초 간격 전송
}