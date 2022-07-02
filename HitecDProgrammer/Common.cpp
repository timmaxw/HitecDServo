#include "Common.h"

#include "CommandLine.h"

HitecDServo servo;
int modelNumber;
HitecDSettings settings;
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

void writeSettings() {
  int res;
  Serial.println(F("Saving new servo settings..."));

  /* Writing the settings starts by resetting the servo to factory settings,
  which will overwrite any range-measurement settings. */
  usingRangeMeasurementSettings = false;

  res = servo.writeSettingsUnsupportedModelThisMightDamageTheServo(
    settings,
    allowUnsupportedModel
  );
  if (res != HITECD_OK) {
    printErr(res, true);
  }

  /* Read back the settings to make sure we have the latest values. */
  if ((res = servo.readSettings(&settings)) != HITECD_OK) {
    printErr(res, true);
  }

  Serial.println(F("Done."));
}

/* When moving to temp raw angles, temporarily overwrite the servo settings by
moving the endpoints beyond the physical limits that the servo can actually
reach; but reduce the servo power limit to 20% so it doesn't damage itself. */
#define RANGE_MEASUREMENT_RAW_ANGLE_FOR_850 50
#define RANGE_MEASUREMENT_RAW_ANGLE_FOR_1500 8192
#define RANGE_MEASUREMENT_RAW_ANGLE_FOR_2150 16333

uint16_t saved0xB2, saved0xC2, saved0xB0, saved0x54, saved0x56;

void useRangeMeasurementSettings() {
  if (!usingRangeMeasurementSettings) {
    Serial.println(F(
      "Temporarily changing servo settings to extreme range & low power..."));

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
    Serial.println(F("Undoing temporary changes to servo settings..."));

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
    if ((res = servo.readSettings(&settings)) != HITECD_OK) {
      printErr(res, true);
    }

    Serial.println(F("Done."));
    usingRangeMeasurementSettings = false;
  }
}

void temporarilyMoveToAPV(int16_t targetAPV, int16_t *actualAPV) {
  useRangeMeasurementSettings();

  /* Instruct the servo to move */
  int16_t targetQuarterMicros = map(
    targetAPV,
    RANGE_MEASUREMENT_RAW_ANGLE_FOR_850,
    RANGE_MEASUREMENT_RAW_ANGLE_FOR_2150,
    850 * 4,
    2150 * 4);
  servo.writeTargetQuarterMicros(targetQuarterMicros);

  /* Wait until it seems to have successfully moved */
  int16_t lastActualAPV = servo.readCurrentAPV();
  if (lastActualAPV < 0) {
    printErr(lastActualAPV, true);
  }
  for (int i = 0; i < 50; ++i) {
    delay(100);
    *actualAPV = servo.readCurrentAPV();
    if (*actualAPV < 0) {
      printErr(*actualAPV, true);
    }
    if (abs(lastActualAPV - *actualAPV) <= 3) {
      break;
    }
    lastActualAPV = *actualAPV;
  }
}

