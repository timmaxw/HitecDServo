/* This example shows how to read the settings from the servo and print them
to the computer over the serial port. */

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

  /* Read the settings from the servo */
  HitecDSettings settings;
  result = servo.readSettings(&settings);
  if (result != HITECD_OK) { printError(result); }

  /* Print the settings over the serial port. See HitecDServo/src/HitecDServo.h
  for documentation about what each of these variables means. */
  Serial.print("id=");
  Serial.println(settings.id);
  Serial.print("counterclockwise=");
  Serial.println(settings.counterclockwise);
  Serial.print("speed=");
  Serial.println(settings.speed);
  Serial.print("deadband=");
  Serial.println(settings.deadband);
  Serial.print("softStart=");
  Serial.println(settings.softStart);
  Serial.print("rangeLeftAPV=");
  Serial.println(settings.rangeLeftAPV);
  Serial.print("rangeRightAPV=");
  Serial.println(settings.rangeRightAPV);
  Serial.print("rangeCenterAPV=");
  Serial.println(settings.rangeCenterAPV);
  Serial.print("failSafe=");
  Serial.println(settings.failSafe);
  Serial.print("failSafeLimp=");
  Serial.println(settings.failSafeLimp);
  Serial.print("powerLimit=");
  Serial.println(settings.powerLimit);
  Serial.print("overloadProtection=");
  Serial.println(settings.overloadProtection);
  Serial.print("smartSense=");
  Serial.println(settings.smartSense);
  Serial.print("sensitivityRatio=");
  Serial.println(settings.sensitivityRatio);
}

void loop() {
  /* Do nothing */
}

void printError(int result) {
  Serial.print("Error: ");
  Serial.println(hitecdErrToString(result));
  while (1) { }
}
