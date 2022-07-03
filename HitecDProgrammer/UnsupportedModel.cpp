#include "UnsupportedModel.h"

#include "CommandLine.h"
#include "Common.h"

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

void printDiagnosticsForUnsupportedModel() {
  Serial.println(F(
    "====================================================================\r\n"
    "Warning: Your servo model is not fully supported. Currently, only\r\n"
    "the D485HW model is fully supported. To improve support for your\r\n"
    "servo model, please open a GitHub issue at\r\n"
    "<https://github.com/timmaxw/HitecDServo/issues/new>\r\n"
    "Include the following following debug information:"
  ));

  static uint8_t registersToDebug[] = {
    /* Model number register */
    0x00,

    /* Settings registers. I want to know if the default values are different
    for other models. */
    0x32, 0x44, 0x4C, 0x4E, 0x54, 0x5E, 0x60, 0x64, 0x66, 0x68, 0x6C, 0x9C,
    0xB0, 0xB2, 0xC2,

    /* Mystery registers that always seem to read a constant value on the
    D485HW. I want to know if they return a different value on other models. */
    0x04, 0x06, 0x8A, 0x8C, 0xC4, 0xD4, 0xD6,

    /* Mystery registers that the DPC-11 always writes to a constant value on
    the D485HW. I want to know if they are set to something else on other
    models. */
    0x50, 0x52, 0x56, 0x72, 0x98, 0x9A
  };

  for (int i = 0; i < sizeof(registersToDebug); ++i) {
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
    useRangeMeasurementSettings();
    Serial.println(F("Moving left as far as possible..."));
    temporarilyMoveToAPV(50, &left);
    Serial.println(F("Moving right as far as possible..."));
    temporarilyMoveToAPV(16333, &right);
    center = (left + right) / 2;
    undoRangeMeasurementSettings();

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
