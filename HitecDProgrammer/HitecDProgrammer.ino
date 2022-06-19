#include <HitecDServo.h>

void setup() {
  Serial.begin(115200);
  HitecDServo servo;
  servo.attach(2);
  int modelNumber = servo.readModelNumber();
  Serial.print("Model number: D");
  Serial.println(modelNumber, DEC);
}

void loop() {
  // put your main code here, to run repeatedly:

}
