#ifndef HitecDServo_h
#define HitecDServo_h

#include <Arduino.h>

/* Many of the functions in this library return error codes. The possible error
codes are as follows: */

/* OK (no error occurred) */
#define HITECD_OK 1

/* attach() was not called, or the call to attach() failed. */
#define HITECD_ERR_NOT_ATTACHED -1

/* No servo detected. */
#define HITECD_ERR_NO_SERVO -2

/* Either the servo is still booting (which takes 1000ms) or the pullup resistor
is missing. (Use a 2k resistor with a 5V microcontroller, or a 1k resistor with
a 3.3V microcontroller.) */
#define HITECD_ERR_BOOTING_OR_NO_PULLUP -3

/* Corrupt response from servo. */
#define HITECD_ERR_CORRUPT -4

/* Unsupported model of servo. (Only D485HW is fully supported.) */
#define HITECD_ERR_UNSUPPORTED_MODEL -5

/* Confusing response from servo. */
#define HITECD_ERR_CONFUSED -6

/* hitecdErrToString() returns a string description of the given error code. You
can print this with Serial for debugging purposes. For example:
  int res = doSomething();
  if (res < 0) {
    Serial.print("Something went wrong: ");
    Serial.println(hitecdErrToString(res));
  }
*/
const char *hitecdErrToString(int err);

struct HitecDSettings {
  /* The default constructor initializes the settings to factory-default values.

  `rangeLeftAPV`, `rangeCenterAPV`, and `rangeRightAPV` will be set to -1;
  this isn't the factory-default value, but it will cause `writeSettings()` to
  keep the factory-default value. */
  HitecDSettings();

  /* `id` is an arbitrary number from 0 to 254. Intended for keeping track of
  multiple servos. No effect on servo behavior. */
  uint8_t id;
  static const uint8_t defaultId = 0;

  /* `counterclockwise=false` if increasing pulse widths make the servo turn
  clockwise. `counterclockwise=true` if increasing pulse widths make the servo
  turn counterclockwise.
  
  Note: Switching the servo direction will invert the meaning of the
  `rangeLeftAPV`, `rangeRightAPV`, and `rangeCenterAPV` settings. If
  you've changed those settings to non-default values, you can use the following
  formulas to convert between the clockwise values and equivalent
  counterclockwise values:
    settings.rangeLeftAPV = 16383 - prevRangeRightAPV;
    settings.rangeCenterAPV = 16383 - prevRangeCenterAPV;
    settings.rangeRightAPV = 16383 - prevRangeLeftAPV;
  */
  bool counterclockwise;
  static const bool defaultCounterclockwise = false;

  /* `speed` defines how fast the servo moves to a new position, as a percentage
  of maximum speed. Legal values are 10, 20, 30, 40, 50, 60, 70, 80, 90, 100. */
  int8_t speed;
  static const int8_t defaultSpeed = 100;

  /* `deadband` defines the servo deadband width. Small values make the servo
  more precise, but it may jitter; or if multiple servos are physically
  connected in parallel, they may fight each other. Larger values make the servo
  more stable, but it will not react to small adjustments. Legal values are
  1 (most precise), 2, 3, 4, 5, 6, 7, 8, 9, 10 (least jitter). */
  int8_t deadband;
  static const int8_t defaultDeadband = 1;

  /* `softStart` limits how fast the servo moves when power is first applied.
  If the servo is at the wrong position when power is first applied, soft start
  can help prevent damage. This has no effect on how fast the servo moves during
  normal operation. Legal values are 20, 40, 60, 80, 100. Setting
  `softStart=100` means the servo starts at full power immediately. */
  int8_t softStart;
  static const int8_t defaultSoftStart = 20;

  /* `rangeLeftAPV`, `rangeRightAPV` and `rangeCenterAPV` define the servo's
  physical range of motion.
  
  Internally, the D-series servos measure the physical servo angle using a
  potentiometer. The angle potentiometer's values are represented as numbers
  from 0 to 16383 (=2**14-1). In this library, the abbreviation "APV" stands for
  "Angle Potentiometer Value".

  PWM pulse widths are related to APVs as follows:
  - If the servo receives a 850us pulse, it will move to `rangeLeftAPV`.
  - If the servo receives a 1500us pulse, it will move to `rangeCenterAPV`.
  - If the servo receives a 2150us pulse, it will move to `rangeRightAPV`.
  - In between, the servo will interpolate.

  Note that APVs depend on the servo's direction. If `counterclockwise=false`,
  higher APVs are clockwise. But if `counterclockwise=true`, higher APVs are
  counterclockwise. This means `rangeLeftAPV < rangeCenterAPV < rangeRightAPV`
  regardless of the servo's direction. But when `counterclockwise=true`, this
  means that the `rangeLeftAPV` is actually the clockwise-most end of the range,
  and `rangeRightAPV` is the counterclockwise-most end of the range.

  If you call `writeSettings()` with `rangeLeftAPV`, `rangeRightAPV`, or
  `rangeCenterAPV` set to -1, then the factory-default value will be used. */
  int16_t rangeLeftAPV, rangeRightAPV, rangeCenterAPV;

  /* Convenience functions to return the factory-default range APVs for the
  given servo model. Right now this only works for the D485HW; other models will
  return -1. */
  static int16_t defaultRangeLeftAPV(int modelNumber);
  static int16_t defaultRangeRightAPV(int modelNumber);
  static int16_t defaultRangeCenterAPV(int modelNumber);

  /* Convenience functions to return the min/max APVs that the servo can be
  safely driven to without hitting physical stops. This may vary slightly from
  servo to servo; these are conservative values. Right now this only works for
  the D485HW; other models will return -1. */
  static int16_t widestRangeLeftAPV(int modelNumber);
  static int16_t widestRangeRightAPV(int modelNumber);
  static int16_t widestRangeCenterAPV(int modelNumber);

  /* If the servo isn't receiving a signal, it will move to a default position
  defined by a pulse width of `failSafe` microseconds. If `failSafeLimp=true`,
  then instead the servo will go limp. If `failSafe=0` and `failSafeLimp=false`,
  the servo will hold its previous position (this is the default). */
  int16_t failSafe;
  bool failSafeLimp;
  static const int16_t defaultFailSafe = 0;
  static const bool defaultFailSafeLimp = false;

  /* `powerLimit` sets the maximum power that the servo can use. It ranges from
  0 to 100 (the default). If set to less than about 10, the servo won't be
  strong enough to overcome the friction of its own gearbox. If set to 0, the
  servo will gently resist being moved, but won't attempt to return to the
  target point.

  Warning: Power limit is an undocumented setting, not supported by Hitec's
  official programmer software. Use at your own risk. */
  int16_t powerLimit;
  static const int16_t defaultPowerLimit = 100;

  /* If the servo is overloaded or stalled for more than about 3 seconds, then
  it will automatically reduce power to `overloadProtection` percent to prevent
  damage. Legal values are:
  - 100 (no overload protection; the default)
  - 10 (reduce power to 10% of max power)
  - 20 (reduce power to 20% of max power)
  - 30 (reduce power to 30% of max power)
  - 40 (reduce power to 40% of max power)
  - 50 (reduce power to 50% of max power)
  (Note, the DPC-11 manual claims that the X% setting will reduce power _by_ X%.
  I think this is an error; in my tests, the X% setting appears to reduce power
  _to_ X%.) */
  int8_t overloadProtection;
  static const int8_t defaultOverloadProtection = 100;

  /* `smartSense` is a Hitec proprietary feature that "allows the servo to
  analyse operational feed back and automatically make on the fly parameter
  adjustments to optimize performance". */
  bool smartSense;
  static const bool defaultSmartSense = true;

  /* `sensitivityRatio` ranges from 819 to 4095. Higher values will make the
  servo react faster to changes in input, but it may jitter more. Lower values
  will make the servo more stable, but it may feel sluggish. If
  `smartSense=true`, then `sensitivityRatio` is ignored. */
  int16_t sensitivityRatio;
  static const int16_t defaultSensitivityRatio = 4095;
};

class HitecDServo {
public:
  HitecDServo();

  int attach(int pin);
  bool attached();
  void detach();

  /* Write the servo's target point (i.e. tell the servo to move somewhere).
  - writeTargetMicroseconds() expresses the target as microseconds of PWM width.
  - writeTargetQuarterMicros() expresses the target as quarter-microseconds of
    PWM width, which is more precise. In both cases, the target will be sent to
    the servo via the serial protocol. */
  void writeTargetMicroseconds(int16_t microseconds);
  void writeTargetQuarterMicros(int16_t quarterMicros);

  /* Reads the servo's current point. You can use this to measure the servo's
  progress towards its target point. These three methods return the same value,
  but expressed in different units. */
  int16_t readCurrentMicroseconds();
  int16_t readCurrentQuarterMicros();
  int16_t readCurrentAPV();

  /* Returns the servo's model number, e.g. 485 for a D485HW model. */
  int readModelNumber();

  /* Returns whether this model of servo is considered fully-supported by this
  version of the HitecDServo library. */
  bool isModelSupported();

  /* Retrieves the current settings from the servo. */
  int readSettings(HitecDSettings *settingsOut);

  /* Resets the servo to its factory-default settings, then uploads the given
  settings, and reboots the servo. The servo will not respond to any commands
  for 1000ms after rebooting; so after writeSettings() returns, make sure to
  wait 1000ms before trying to do anything else with the servo.

  Note: Right now, this only works for the D485HW model. Other models
  will return an error. */
  int writeSettings(const HitecDSettings &settings);

  /* It's dangerous to change the settings of a non-D485HW model; this hasn't
  been tested, and might damage the servo. If you're willing to take the risk,
  you can use writeSettingsUnsupportedModelThisMightDamageTheServo() with
  allowUnsupportedModel=true to skip checking the servo model. */
  int writeSettingsUnsupportedModelThisMightDamageTheServo(
    const HitecDSettings &settings,
    bool allowUnsupportedModel);

  /* Directly read/write registers on the servo. Don't use this unless you know
  what you're doing. (The only reason these methods are declared public is so
  that tests, etc. can access them.) */
  int readRawRegister(uint8_t reg, uint16_t *valOut);
  void writeRawRegister(uint8_t reg, uint16_t val);

private:
  void writeByte(uint8_t value);
  int readByte();

  int pin;
  uint8_t pinBitMask;
  volatile uint8_t *pinInputRegister, *pinOutputRegister;

  int modelNumber;
  int16_t rangeLeftAPV, rangeRightAPV, rangeCenterAPV;
};

#endif /* HitecDServo_h */
