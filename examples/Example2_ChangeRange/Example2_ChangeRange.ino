/* This example shows how to change the servo's range setting to use the full
range that the servo is physically capable of. */

#include <HitecDServo.h>

HitecDServo servo;

void setup() {
  int result;

  Serial.begin(115200);

  int servoPin = 2;
  result = servo.attach(servoPin);

  /* Always check that the return value is HITECD_OK. If not, this can indicate
  a problem communicating with the servo. */
  if (result != HITECD_OK) { printError(result); }

  int modelNumber = servo.readModelNumber();

  /* rangeLeftAPV and rangeRightAPV define where the servo will move when it
  receives a 850us or 2150us PWM pulse, respectively. Use the helper functions
  widestRangeLeftAPV() and widestRangeRightAPV() to get the safe min/max values
  for this model of servo. */
  HitecDSettings settings;
  settings.rangeLeftAPV = HitecDSettings::widestRangeLeftAPV(modelNumber);
  settings.rangeRightAPV = HitecDSettings::widestRangeRightAPV(modelNumber);
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
