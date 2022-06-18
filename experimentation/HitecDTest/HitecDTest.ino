#include <SoftwareSerial.h>

SoftwareSerial servoSerial(2, 2, true);

void setup() {
  Serial.begin(115200);
  servoSerial.begin(115200);
  servoSerial.stopListening();
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void writeRegister(uint8_t reg, uint16_t val) {
  servoSerial.write((uint8_t)0x96);
  servoSerial.write((uint8_t)0x00);
  servoSerial.write(reg);
  servoSerial.write((uint8_t)0x02);
  uint8_t low = val & 0xFF;
  servoSerial.write(low);
  uint8_t high = (val >> 8) & 0xFF;
  servoSerial.write(high);
  uint8_t checksum = (0x00 + reg + 0x02 + low + high) & 0xFF;
  servoSerial.write(checksum);
  digitalWrite(2, LOW);
}

void readRegister(uint8_t reg) {
  Serial.println("Trying to read register");
  servoSerial.write((uint8_t)0x96);
  servoSerial.write((uint8_t)0x00);
  servoSerial.write(reg);
  servoSerial.write((uint8_t)0x00);
  uint8_t checksum = (0x00 + reg + 0x00) & 0xFF;
  servoSerial.write(checksum);
  digitalWrite(2, LOW);
  delay(8);
  servoSerial.listen();
  pinMode(2, INPUT_PULLUP);
  delay(16);
  while (servoSerial.available()) {
    uint8_t reply = servoSerial.read();
    Serial.print("Received: ");
    Serial.println(reply, HEX);
  }
  servoSerial.stopListening();
  digitalWrite(2, LOW);
  pinMode(2, OUTPUT);
  Serial.println("Done trying to read register");
}


void loop() {
  readRegister(0x06);
  delay(1000);
}
