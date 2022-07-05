#include <HitecDServo.h>

HitecDServo servo;

void setup() {
  int result;

  Serial.begin(115200);

  int servo_pin = 2;
  result = servo.attach(servo_pin);

  /* Always check that the return value is HITECD_OK. If not, this can indicate
  a problem communicating with the servo. */
  if (result != HITECD_OK) { printError(result); }

  /* Reduce the servo's speed to 50% and reverse the direction. */
  HitecDSettings settings;
  settings.speed = 50;
  settings.counterclockwise = true;
  result = servo.writeSettings(settings);
  if (result != HITECD_OK) { printError(result); }

  /* After writeSettings(), we must wait 1000ms for the servo to reboot. */
  delay(1000);
}

void loop() {
  /* Sweep the servo back and forth between the end points */
  servo.writeTargetMicroseconds(850);
  delay(2000);
  servo.writeTargetMicroseconds(2150);
  delay(2000);
}

void printError(int result) {
  Serial.print("Error: ");
  Serial.println(hitecdErrToString(result));
  while (1) { }
}
