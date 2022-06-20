#include <HitecDServo.h>

HitecDServo servo;
HitecDServoConfig servoConfig;

void printError(int res) {
  switch (res) {
  case HITECD_OK:
    return;
  case HITECD_ERR_NO_SERVO:
    Serial.println("Error: No servo detected.");
    break;
  case HITECD_ERR_NO_RESISTOR:
    Serial.println("Error: Missing 2k resistor between signal wire and +5V rail.");
    break;
  case HITECD_ERR_CORRUPT:
    Serial.println("Error: Corrupt response from servo.");
    break;
  default:
    Serial.println("Error: Unknown error.");
    break;
  }
}

void printConfig() {
  Serial.print("ID: ");
  Serial.println(servoConfig.id, DEC);

  Serial.print("Direction: ");
  if (servoConfig.counterclockwise) {
    Serial.println("Counterclockwise");
  } else {
    Serial.println("Clockwise");
  }

  Serial.print("Speed: ");
  Serial.println(servoConfig.speed, DEC);

  Serial.print("Deadband: ");
  Serial.println(servoConfig.deadband, DEC);

  Serial.print("Soft start: ");
  Serial.println(servoConfig.softStart, DEC);

  Serial.print("Left point: ");
  Serial.println(servoConfig.leftPoint, DEC);
  Serial.print("Center point: ");
  Serial.println(servoConfig.centerPoint, DEC);
  Serial.print("Right point: ");
  Serial.println(servoConfig.rightPoint, DEC);

  Serial.print("Fail safe: ");
  if (servoConfig.failSafe) {
    Serial.println(servoConfig.failSafe, DEC);
  } else if (servoConfig.failSafeLimp) {
    Serial.println("Limp");
  } else {
    Serial.println("Off");
  }

  Serial.print("Overload protection: ");
  Serial.println(servoConfig.overloadProtection, DEC);

  Serial.print("Smart sense: ");
  if (servoConfig.smartSense) {
    Serial.println("On");
  } else {
    Serial.println("Off");
  }

  Serial.print("Sensitivity ratio: ");
  Serial.println(servoConfig.sensitivityRatio, DEC);
}

void setup() {
  Serial.begin(115200);

  servo.attach(2);

  Serial.println("Reading model number...");
  int modelNumber = servo.readModelNumber();
  if (modelNumber < 0) {
    printError(modelNumber);
    while (true) { }
  }
  Serial.print("Model number: D");
  Serial.println(modelNumber, DEC);

  Serial.println("Reading configuration...");
  int res = servo.readConfig(&servoConfig);
  if (res < 0) {
    printError(res);
    while (true) { }
  }
  printConfig();
}

void loop() {
  // put your main code here, to run repeatedly:

}
