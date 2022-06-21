#ifndef HitecDServo_h
#define HitecDServo_h

#include <Arduino.h>

#define HITECD_OK 1
#define HITECD_ERR_NO_SERVO -1
#define HITECD_ERR_NO_RESISTOR -2
#define HITECD_ERR_CORRUPT -3
#define HITECD_ERR_UNSUPPORTED_MODEL -4
#define HITECD_ERR_NOT_ATTACHED -5

struct HitecDServoConfig {
  /* The default constructor initializes the settings to factory-default values.

  `leftPoint`, `centerPoint`, and `rightPoint` will be set to -1; this isn't the
  factory-default value, but it will cause `writeConfig()` to keep the factory-
  default value. */
  HitecDServoConfig();

  /* `id` is an arbitrary number from 0 to 254. Intended for keeping track of
  multiple servos. No effect on servo behavior. */
  uint8_t id;
  static const uint8_t idDefault = 0;

  /* `counterclockwise=false` if increasing pulse widths make the servo turn
  clockwise. `counterclockwise=true` if increasing pulse widths make the servo
  turn counterclockwise.
  
  Note: Switching the servo direction will invert the meaning of the
  `leftPoint`, `rightPoint`, and `centerPoint` settings. If you've changed those
  settings to non-default values, you can use the following formulas to convert
  between the clockwise values and equivalent counterclockwise values:
    ccwSettings.leftPoint   = 16383 - cwSettings.rightPoint;
    ccwSettings.centerPoint = 16383 - cwSettings.centerPoint;
    ccwSettings.rightPoint  = 16383 - cwSettings.leftPoint;
  */
  bool counterclockwise;
  static const bool counterclockwiseDefault = false;

  /* `speed` defines how fast the servo moves to a new position, as a percentage
  of maximum speed. Legal values are 10, 20, 30, 40, 50, 60, 70, 80, 90, 100. */
  int8_t speed;
  static const int8_t speedDefault = 100;

  /* `deadband` defines the servo deadband width. Small values make the servo
  more precise, but it may jitter; or if multiple servos are physically
  connected in parallel, they may fight each other. Larger values make the servo
  more stable, but it will not react to small adjustments. Legal values are
  1 (most precise), 2, 3, 4, 5, 6, 7, 8, 9, 10 (least jitter). */
  int8_t deadband;
  static const int8_t deadbandDefault = 1;

  /* `softStart` limits how fast the servo moves when power is first applied.
  If the servo is at the wrong position when power is first applied, soft start
  can help prevent damage. This has no effect on how fast the servo moves during
  normal operation. Legal values are 20, 40, 60, 80, 100. Setting
  `softStart=100` means the servo starts at full power immediately. */
  int8_t softStart;
  static const int8_t softStartDefault = 20;

  /* `leftPoint`, `centerPoint` and `rightPoint` define how servo pulse
  widths are related to physical servo positions:
  - `leftPoint` defines the position that corresponds to a 850us pulse
  - `centerPoint` defines the position that corresponds to a 1500us pulse
  - `rightPoint` defines the position that corresponds to a 2150us pulse

  The positions are defined by numbers ranging from 0 to 16383. If
  `counterclockwise=false`, higher numbers are clockwise. But if
  `counterclockwise=true`, higher numbers are counterclockwise; this means that
  `leftPoint < rightPoint < centerPoint`, even though this makes the "left"
  point clockwise of center, and the "right" point counterclockwise of center.

  If you call writeConfig() with these values set to -1, then the
  factory-default values will be kept. */
  int16_t leftPoint, centerPoint, rightPoint;

  /* Convenience functions to return the factory-default values of `leftPoint`,
  `rightPoint`, and `centerPoint` for the given servo model. Right now this only
  works for the D485HW; other models will return -1. */
  static int16_t leftPointDefault(int modelNumber);
  static int16_t centerPointDefault(int modelNumber);
  static int16_t rightPointDefault(int modelNumber);

  /* The factory-default values usually produce a narrower movement range than
  the servo is physically capable of (e.g. for the D485HW, the factory-default
  values produce a 147 degree range, but the servo can actually do 201 degrees.)
  To access the full range of motion, set `leftPoint=leftPointFullRange(model)`
  and `rightPoint=rightPointFullRange(model)`. Right now this only works for the
  D485HW; other models will return -1. */
  static int16_t leftPointFullRange(int modelNumber);
  static int16_t rightPointFullRange(int modelNumber);

  /* If the servo isn't receiving a signal, it will move to a default position
  defined by a pulse width of `failSafe` microseconds. If `failSafeLimp=true`,
  then instead the servo will go limp. If `failSafe=0` and `failSafeLimp=false`,
  the servo will hold its previous position (this is the default). */
  int16_t failSafe;
  bool failSafeLimp;
  static const int16_t failSafeDefault = 0;
  static const bool failSafeLimpDefault = false;

  /* If the servo is overloaded or stalled, then it will automatically reduce
  power by `overloadProtection` percent to prevent damage. Legal values are:
  - 100 (no overload protection; the default)
  - 10 (reduce power by 10%, so 90% of max power)
  - 20 (reduce power by 20%, so 80% of max power)
  - 30 (reduce power by 30%, so 70% of max power)
  - 40 (reduce power by 40%, so 60% of max power)
  - 50 (reduce power by 50%, so 50% of max power)
  TODO why is 100 no overload protection, not 0*/
  int8_t overloadProtection;
  static const int8_t overloadProtectionDefault = 100;

  /* `smartSense` is a Hitec proprietary feature that "allows the servo to
  analyse operational feed back and automatically make on the fly parameter
  adjustments to optimize performance". */
  bool smartSense;
  static const bool smartSenseDefault = true;

  /* `sensitivityRatio` ranges from 819 to 4095. Higher values will make the
  servo react faster to changes in input, but it may jitter more. Lower values
  will make the servo more stable, but it may feel sluggish. If
  `smartSense=true`, then `sensitivityRatio` is ignored. */
  int16_t sensitivityRatio;
  static const int16_t sensitivityRatioDefault = 4095;
};

class HitecDServo {
public:
  HitecDServo();

  void attach(int pin);
  bool attached();
  void detach();

  int readModelNumber();

  int readConfig(HitecDServoConfig *config_out);

  /* Resets the servo to its factory-default configuration, then uploads the
  given configuration. Note: Right now, this only works for the D485HW model.
  Other models will return an error. */
  int writeConfig(const HitecDServoConfig &config);

  /* It's dangerous to change the configuration of a non-D485HW model; this
  hasn't been tested, and might damage the servo. If you're willing to take the
  risk, you can use writeConfigUnknownModelThisMightDamageTheServo() with
  bypassModelNumberCheck=true, to bypass the logic that checks the servo model.
  */
  int writeConfigUnknownModelThisMightDamageTheServo(
    const HitecDServoConfig &config,
    bool bypassModelNumberCheck);

  /* Directly read/write registers on the servo. Don't use this unless you know
  what you're doing. (The only reason these methods are declared public is so
  that tests, etc. can access them.) */
  int readRawRegister(uint8_t reg, uint16_t *val_out);
  void writeRawRegister(uint8_t reg, uint16_t val);

private:
  void writeByte(uint8_t value);
  int readByte();

  int pin;
  uint8_t pinBitMask;
  volatile uint8_t *pinInputRegister, *pinOutputRegister;
};

#endif /* HitecDServo_h */
