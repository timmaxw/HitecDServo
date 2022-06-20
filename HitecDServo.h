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
  /* `id` is an arbitrary number from 0 to 254. Intended for keeping track of
  multiple servos. No effect on servo behavior. The default is `id=0`. */
  uint8_t id;

  /* `counterclockwise=false` if increasing pulse widths make the servo turn
  clockwise. `counterclockwise=true` if increasing pulse widths make the servo
  turn counterclockwise. The default is `counterclockwise=false`. */
  bool counterclockwise;

  /* `speed` defines how fast the servo moves to a new position, as a percentage
  of maximum speed. Legal values are 10, 20, ... 100. The default is
  `speed=100`. */
  int8_t speed;

  /* `deadband` defines the servo deadband width, in microseconds. Small values
  make the servo more precise, but it may jitter; or if multiple servos are
  physically connected in parallel, they may fight each other. Larger values
  make the servo more stable, but it will not react to small adjustments. Legal
  values are 1, 2, ... 10, and the default is `deadband=1` (most precise). */
  int8_t deadband;

  /* `softStart` limits how fast the servo moves when power is first applied.
  If the servo is at the wrong position when power is first applied, soft start
  can help prevent damage. This has no effect on how fast the servo moves during
  normal operation. Legal values are 20, 40, 60, 80, 100. Setting
  `softStart=100` means the servo starts at full power immediately. The default
  is `softStart=20`. */
  int8_t softStart;

  /* `leftPoint`, `centerPoint` and `rightPoint` define how servo pulse
  widths are related to physical servo positions:
  - `leftPoint` defines the position that corresponds to a 850us pulse
  - `centerPoint` defines the position that corresponds to a 1500us pulse
  - `rightPoint` defines the position that corresponds to a 2150us pulse

  TODO confirm this

  The positions are defined by numbers ranging from 0 to 16383. If
  `counterclockwise=false`, higher numbers are clockwise. But if
  `counterclockwise=true`, higher numbers are counterclockwise; this means that
  `leftPoint < rightPoint < centerPoint`, even though this makes the "left"
  point clockwise of center, and the "right" point counterclockwise of center.

  For the Hitec D485HW model, the default values are `leftPoint=3381`,
  `centerPoint=8192`, and `rightPoint=13002`, which gives the D485HW a
  147-degree range of motion. To use the D485HW's full 200-degree range of
  motion, reasonable values are `leftPoint=1646` and `rightPoint=14736`.
  (Other Hitec D-series models may be different!) */
  int16_t leftPoint, centerPoint, rightPoint;

  /* If the servo isn't receiving a signal, it will move to a default position
  defined by a pulse width of `failSafe` microseconds. If `failSafeLimp=true`,
  then instead the servo will go limp. If `failSafe=0` and `failSafeLimp=false`,
  the servo will hold its previous position (this is the default). */
  int16_t failSafe;
  bool failSafeLimp;

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

  /* `smartSense` is a Hitec proprietary feature that "allows the servo to
  analyse operational feed back and automatically make on the fly parameter
  adjustments to optimize performance". The default is `smartSense=true`. */
  bool smartSense;

  /* `sensitivityRatio` ranges from 819 to 4095. Higher values will make the
  servo react faster to changes in input, but it may jitter more. Lower values
  will make the servo more stable, but it may feel sluggish. If
  `smartSense=true`, then `sensitivityRatio` is ignored. */
  int16_t sensitivityRatio;
};

class HitecDServo {
public:
  HitecDServo();

  void attach(int pin);
  bool attached();
  void detach();

  int readModelNumber();
  int readConfig(HitecDServoConfig *config_out);

  int writeConfig(HitecDServoConfig config);

private:
  friend void debugUnknownModelRegistersToSerial();

  void writeByte(uint8_t value);
  int readByte();

  void writeReg(uint8_t reg, uint16_t val);
  int readReg(uint8_t reg, uint16_t *val_out);

  int pin;
  uint8_t pinBitMask;
  volatile uint8_t *pinInputRegister, *pinOutputRegister;
};

#endif /* HitecDServo_h */
