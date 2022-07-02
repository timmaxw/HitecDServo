#include "Common.h"

#include "CommandLine.h"

HitecDServo servo;
int modelNumber;
HitecDServoConfig config;
bool allowUnsupportedModel = false;
bool usingRangeMeasurementSettings = false;

void printErr(int res, bool needReset) {
  if (res == HITECD_OK) {
    return;
  }

  Serial.print(F("Error: "));
  Serial.println(hitecdErrToString(res));

  if (needReset) {
    Serial.println(F("Please fix the problem and then reset your Arduino."));
    while (true) { }
  }
}

void printValueWithDefault(int16_t value, int16_t defaultValue) {
  Serial.print(value, DEC);
  if (value == defaultValue) {
    Serial.println(F(" (default)"));
  } else if (defaultValue != -1) {
    Serial.print(F(" (default is "));
    Serial.print(defaultValue, DEC);
    Serial.println(')');
  }
}

bool checkSupportedModel() {
  if (servo.isModelSupported() || allowUnsupportedModel) {
    return true;
  }

  Serial.println(F(
    "Warning: Your servo model is not fully supported. Changing the\r\n"
    "settings may lead to unexpected behavior or even damage the servo.\r\n"
    "Proceed at your own risk. If you want to proceed, please enter\r\n"
    "\"This might damage the servo\" exactly (or enter nothing to cancel):"));
  scanRawInput();
  if (parseWord("This might damage the servo")) {
    allowUnsupportedModel = true;
    return true;
  } else if (rawInputLen == 0) {
    return false;
  } else {
    Serial.println(F("You did not enter \"This might damage the servo\"."));
    return false;
  }
}

void writeConfig() {
  int res;
  Serial.println(F("Changing servo config..."));

  /* Writing the config starts by resetting the servo to factory settings,
  which will overwrite any range-measurement settings. */
  usingRangeMeasurementSettings = false;

  res = servo.writeConfigUnsupportedModelThisMightDamageTheServo(
    config,
    allowUnsupportedModel
  );
  if (res != HITECD_OK) {
    printErr(res, true);
  }

  /* Read back the settings to make sure we have the latest values. */
  if ((res = servo.readConfig(&config)) != HITECD_OK) {
    printErr(res, true);
  }

  Serial.println(F("Done."));
}

/* When moving to temp raw angles, temporarily overwrite the servo config by
moving the endpoints beyond the physical limits that the servo can actually
reach; but reduce the servo power limit to 20% so it doesn't damage itself. */
#define RANGE_MEASUREMENT_RAW_ANGLE_FOR_850 50
#define RANGE_MEASUREMENT_RAW_ANGLE_FOR_1500 8192
#define RANGE_MEASUREMENT_RAW_ANGLE_FOR_2150 16333

uint16_t saved0xB2, saved0xC2, saved0xB0, saved0x54, saved0x56;

void useRangeMeasurementSettings() {
  if (!usingRangeMeasurementSettings) {
    Serial.println(F(
      "Temporarily changing servo config to extreme range & low power..."));

    int res;
    if ((res = servo.readRawRegister(0xB2, &saved0xB2)) != HITECD_OK) {
      printErr(res, true);
    }
    if ((res = servo.readRawRegister(0xC2, &saved0xC2)) != HITECD_OK) {
      printErr(res, true);
    }
    if ((res = servo.readRawRegister(0xB0, &saved0xB0)) != HITECD_OK) {
      printErr(res, true);
    }
    if ((res = servo.readRawRegister(0x54, &saved0x54)) != HITECD_OK) {
      printErr(res, true);
    }
    if ((res = servo.readRawRegister(0x56, &saved0x56)) != HITECD_OK) {
      printErr(res, true);
    }

    servo.writeRawRegister(0xB2, RANGE_MEASUREMENT_RAW_ANGLE_FOR_850);
    servo.writeRawRegister(0xC2, RANGE_MEASUREMENT_RAW_ANGLE_FOR_1500);
    servo.writeRawRegister(0xB0, RANGE_MEASUREMENT_RAW_ANGLE_FOR_2150);
    servo.writeRawRegister(0x54, 0x0005);
    servo.writeRawRegister(0x56, 0x0190);

    servo.writeRawRegister(0x70, 0xFFFF);
    servo.writeRawRegister(0x46, 0x0001);
    delay(1000);

    Serial.println(F("Done."));
    usingRangeMeasurementSettings = true;
  }
}

void undoRangeMeasurementSettings() {
  if (usingRangeMeasurementSettings) {
    Serial.println(F("Undoing temporary changes to servo config..."));

    servo.writeRawRegister(0xB2, saved0xB2);
    servo.writeRawRegister(0xC2, saved0xC2);
    servo.writeRawRegister(0xB0, saved0xB0);
    servo.writeRawRegister(0x54, saved0x54);
    servo.writeRawRegister(0x56, saved0x56);

    servo.writeRawRegister(0x70, 0xFFFF);
    servo.writeRawRegister(0x46, 0x0001);
    delay(1000);

    /* Read back the settings to make sure we have the latest values. */
    int res;
    if ((res = servo.readConfig(&config)) != HITECD_OK) {
      printErr(res, true);
    }

    Serial.println(F("Done."));
    usingRangeMeasurementSettings = false;
  }
}

void moveTempRawAngle(int16_t targetRawAngle, int16_t *actualRawAngle) {
  useRangeMeasurementSettings();

  /* Instruct the servo to move */
  int16_t targetQuarterMicros = map(
    targetRawAngle,
    RANGE_MEASUREMENT_RAW_ANGLE_FOR_850,
    RANGE_MEASUREMENT_RAW_ANGLE_FOR_2150,
    850 * 4,
    2150 * 4);
  servo.writeTargetQuarterMicros(targetQuarterMicros);

  /* Wait until it seems to have successfully moved */
  int16_t lastActualRawAngle = servo.readCurrentRawAngle();
  if (lastActualRawAngle < 0) {
    printErr(lastActualRawAngle, true);
  }
  for (int i = 0; i < 50; ++i) {
    delay(100);
    *actualRawAngle = servo.readCurrentRawAngle();
    if (*actualRawAngle < 0) {
      printErr(*actualRawAngle, true);
    }
    if (abs(lastActualRawAngle - *actualRawAngle) <= 3) {
      break;
    }
    lastActualRawAngle = *actualRawAngle;
  }
}

