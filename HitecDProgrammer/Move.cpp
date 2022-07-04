#include "Move.h"

#include "Common.h"

/* When moving gently to arbitrary APVs, temporarily overwrite the servo
settings by moving the endpoints beyond the physical limits that the servo can
actually reach; but reduce the servo power limit to 20% so it doesn't damage
itself. */
#define GENTLE_MOVEMENT_RANGE_LEFT_APV 50
#define GENTLE_MOVEMENT_RANGE_CENTER_APV 8192
#define GENTLE_MOVEMENT_RANGE_RIGHT_APV 16333

bool usingGentleMovementSettings = false;

uint16_t saved0xB2, saved0xC2, saved0xB0, saved0x54, saved0x56;

void useGentleMovementSettings() {
  if (usingGentleMovementSettings) {
    return;
  }

  Serial.println(F(
    "Temporarily changing servo settings to widest range & low power..."));

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

  servo.writeRawRegister(0xB2, GENTLE_MOVEMENT_RANGE_LEFT_APV);
  servo.writeRawRegister(0xC2, GENTLE_MOVEMENT_RANGE_CENTER_APV);
  servo.writeRawRegister(0xB0, GENTLE_MOVEMENT_RANGE_RIGHT_APV);
  servo.writeRawRegister(0x54, 0x0005);
  servo.writeRawRegister(0x56, 0x0190);

  servo.writeRawRegister(0x70, 0xFFFF);
  servo.writeRawRegister(0x46, 0x0001);
  delay(1000);

  Serial.println(F("Done."));
  usingGentleMovementSettings = true;
}

void undoGentleMovementSettings() {
  if (!usingGentleMovementSettings) {
    return;
  }

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
  usingGentleMovementSettings = false;
}

void moveGentlyToAPV(int16_t targetAPV, int16_t *actualAPV) {
  useGentleMovementSettings();

  /* Instruct the servo to move */
  int16_t targetQuarterMicros = map(
    targetAPV,
    GENTLE_MOVEMENT_RANGE_LEFT_APV,
    GENTLE_MOVEMENT_RANGE_RIGHT_APV,
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

