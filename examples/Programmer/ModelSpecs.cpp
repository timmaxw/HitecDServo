#include "ModelSpecs.h"

#include <HitecDServoInternal.h>

#include "CommandLine.h"
#include "Programmer.h"
#include "Move.h"

void setupUnsupportedModelSpecs() {
  Serial.println(F(
    "====================================================================\r\n"
    "Warning: Your servo model is not fully supported. Currently, only\r\n"
    "the D485HW model is fully supported. To improve support for your\r\n"
    "servo model, please open a GitHub issue at\r\n"
    "<https://github.com/timmaxw/HitecDServo/issues/new>\r\n"
    "Include the following following diagnostic information:"
  ));

  static uint8_t registersToDebug[] = {
    HD_REG_MODEL_NUMBER,

    /* Settings registers. I want to know if the default values are different
    for other models. */
    HD_REG_ID,
    HD_REG_DIRECTION,
    HD_REG_SPEED,
    HD_REG_DEADBAND_1,
    HD_REG_DEADBAND_2,
    HD_REG_DEADBAND_3,
    HD_REG_SOFT_START,
    HD_REG_RANGE_LEFT_APV,
    HD_REG_RANGE_RIGHT_APV,
    HD_REG_RANGE_CENTER_APV,
    HD_REG_FAIL_SAFE,
    HD_REG_POWER_LIMIT,
    HD_REG_OVERLOAD_PROTECTION,
    HD_REG_SMART_SENSE_1,
    HD_REG_SMART_SENSE_2,
    HD_REG_SENSITIVITY_RATIO,

    /* Registers that always seem to be read/written with constant values on the
    D485HW. I want to know if they return a different value on other models. */
    HD_REG_SS_ENABLE_1,
    HD_REG_SS_ENABLE_2,
    HD_REG_SS_DISABLE_1,
    HD_REG_SS_DISABLE_2,
    HD_REG_MYSTERY_OP1,
    HD_REG_MYSTERY_OP2,
    HD_REG_MYSTERY_DB,
    0x04,
    0x06,
    0x50,
    0x52,
    0xC4
  };

  for (int i = 0; i < (int)sizeof(registersToDebug); ++i) {
    uint8_t reg = registersToDebug[i];
    uint16_t temp;
    int res;
    if ((res = servo.readRawRegister(reg, &temp)) != HITECD_OK) {
      printErr(res, true);
    }
    Serial.print((reg >> 4) & 0x0F, HEX);
    Serial.print((reg >> 0) & 0x0F, HEX);
    Serial.print(':');
    Serial.print((temp >> 12) & 0x0F, HEX);
    Serial.print((temp >> 8) & 0x0F, HEX);
    Serial.print((temp >> 4) & 0x0F, HEX);
    Serial.print((temp >> 0) & 0x0F, HEX);
    if (i % 8 == 7 || i + 1 == sizeof(registersToDebug)) {
      Serial.println();
    } else {
      Serial.print(' ');
    }
  }

  Serial.println(F(
    "Is it OK to move the servo to detect the physical range of motion?\r\n"
    "Make sure nothing is attached to the servo horn. Enter \"y\" or \"n\":"));
  if (scanYesNo()) {
    int16_t left, right, center;
    useGentleMovementSettings();
    Serial.println(F("Moving left as far as possible..."));
    moveGentlyToAPV(50, &left);
    Serial.println(F("Moving right as far as possible..."));
    moveGentlyToAPV(16333, &right);
    center = (left + right) / 2;
    undoGentleMovementSettings();

    /* Note widestRangeLeftAPVClockwise/etc. always follow a clockwise
    convention. So if the servo is in counterclockwise mode, we have to invert
    them. */
    if (!settings.counterclockwise) {
      widestRangeLeftAPVClockwise = left;
      widestRangeRightAPVClockwise = right;
      widestRangeCenterAPVClockwise = center;
    } else {
      widestRangeLeftAPVClockwise = 16383 - right;
      widestRangeRightAPVClockwise = 16383 - left;
      widestRangeCenterAPVClockwise = 16383 - center;
    }

    Serial.println(F("Please include the following in the issue report:"));
    Serial.print(F("widestRangeLeftAPVClockwise="));
    Serial.println(widestRangeLeftAPVClockwise);
    Serial.print(F("widestRangeRightAPVClockwise="));
    Serial.println(widestRangeRightAPVClockwise);

  } else {
    Serial.println(F("OK, servo will not be moved."));
  }

  Serial.println(F(
    "If you had already changed the values of any settings, please also\r\n"
    "note those changes in the issue. Thanks!\r\n"));
  Serial.println(F("Press enter to continue..."));
  scanRawInput(NO_ECHO);
  Serial.println(F(
    "===================================================================="
  ));
}

void setupModelSpecs() {
  if (servo.isModelSupported()) {
    defaultRangeLeftAPV =
      HitecDSettings::defaultRangeLeftAPV(modelNumber);
    defaultRangeRightAPV =
      HitecDSettings::defaultRangeRightAPV(modelNumber);
    defaultRangeCenterAPV =
      HitecDSettings::defaultRangeCenterAPV(modelNumber);
    widestRangeLeftAPVClockwise =
      HitecDSettings::widestRangeLeftAPV(modelNumber);
    widestRangeRightAPVClockwise =
      HitecDSettings::widestRangeRightAPV(modelNumber);
    widestRangeCenterAPVClockwise =
      HitecDSettings::widestRangeCenterAPV(modelNumber);
  } else {
    setupUnsupportedModelSpecs();
  }
}

int16_t defaultRangeLeftAPV = -1;
int16_t defaultRangeRightAPV = -1;
int16_t defaultRangeCenterAPV = -1;

int16_t widestRangeLeftAPVClockwise = -1;
int16_t widestRangeRightAPVClockwise = -1;
int16_t widestRangeCenterAPVClockwise = -1;

int16_t widestRangeLeftAPV() {
  if (widestRangeLeftAPVClockwise == -1) {
    return -1;
  } else if (!settings.counterclockwise) {
    return widestRangeLeftAPVClockwise;
  } else {
    return 16383 - widestRangeRightAPVClockwise;
  }
}

int16_t widestRangeRightAPV() {
  if (widestRangeRightAPVClockwise == -1) {
    return -1;
  } else if (!settings.counterclockwise) {
    return widestRangeRightAPVClockwise;
  } else {
    return 16383 - widestRangeLeftAPVClockwise;
  }
}

int16_t widestRangeCenterAPV() {
  if (widestRangeCenterAPVClockwise == -1) {
    return -1;
  } else if (!settings.counterclockwise) {
    return widestRangeCenterAPVClockwise;
  } else {
    return 16383 - widestRangeCenterAPVClockwise;
  }
}

bool allowUnsupportedModel = false;

bool checkSupportedModel() {
  if (servo.isModelSupported() || allowUnsupportedModel) {
    return true;
  }

  Serial.println(F(
    "Warning: Your servo model is not fully supported. Changing the\r\n"
    "settings may lead to unexpected behavior or even damage the servo.\r\n"
    "Proceed anyway? Enter \"y\" or \"n\":"));
  allowUnsupportedModel = scanYesNo();
  return allowUnsupportedModel;
}

