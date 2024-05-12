#include "HitecDServo.h"

#include "HitecDServoInternal.h"

HitecDServo::HitecDServo() : pin(-1) { }

int HitecDServo::attach(int _pin) {
  if (attached()) {
    detachAndReset();
  }

  pin = _pin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);

  pinBitMask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  pinInputRegister = portInputRegister(port);
  pinOutputRegister = portOutputRegister(port);

  int res;
  uint16_t temp;

  if ((res = readRawRegister(HD_REG_MODEL_NUMBER, &temp)) != HITECD_OK) {
    detachAndReset();
    return res;
  }
  modelNumber = temp;

  if ((res = readRawRegister(HD_REG_RANGE_LEFT_APV, &temp)) != HITECD_OK) {
    detachAndReset();
    return res;
  }
  rangeLeftAPV = temp;

  if ((res = readRawRegister(HD_REG_RANGE_RIGHT_APV, &temp)) != HITECD_OK) {
    detachAndReset();
    return res;
  }
  rangeRightAPV = temp;

  if ((res = readRawRegister(HD_REG_RANGE_CENTER_APV, &temp)) != HITECD_OK) {
    detachAndReset();
    return res;
  }
  rangeCenterAPV = temp;

  return HITECD_OK;
}

bool HitecDServo::attached() {
  return (pin != -1);
}

void HitecDServo::detachAndReset() {
  writeRawRegister(HD_REG_REBOOT, HD_REBOOT_CONST);

  pin = -1;
}

void HitecDServo::writeTargetMicroseconds(int16_t microseconds) {
  writeTargetQuarterMicros(4 * microseconds);
}

void HitecDServo::writeTargetQuarterMicros(int16_t quarterMicros) {
  if (!attached()) {
    return;
  }
  quarterMicros = constrain(quarterMicros, 4*850, 4*2150);
  writeRawRegister(HD_REG_TARGET, quarterMicros - 3000);
}

int16_t HitecDServo::readCurrentMicroseconds() {
  int16_t quarterMicros = readCurrentQuarterMicros();
  if (quarterMicros < 0) {
    return quarterMicros;
  }
  return quarterMicros / 4;
}

int16_t HitecDServo::readCurrentQuarterMicros() {
  int16_t currentAPV = readCurrentAPV();
  if (currentAPV < 0) {
    return currentAPV;
  }
  if (currentAPV < rangeCenterAPV) {
    return map(currentAPV, rangeLeftAPV, rangeCenterAPV, 4*850, 4*1500);
  } else {
    return map(currentAPV, rangeCenterAPV, rangeRightAPV, 4*1500, 4*2150);
  }
}

int16_t HitecDServo::readCurrentAPV() {
  if (!attached()) {
    return HITECD_ERR_NOT_ATTACHED;
  }
  int res;
  uint16_t currentAPV;
  if ((res = readRawRegister(HD_REG_CURRENT_APV, &currentAPV)) != HITECD_OK) {
    return res;
  }
  return currentAPV;
}

int HitecDServo::readModelNumber() {
  if (!attached()) {
    return HITECD_ERR_NOT_ATTACHED;
  }
  return modelNumber;
}

bool HitecDServo::isModelSupported() {
  if (!attached()) {
    return false;
  }
  switch (modelNumber) {
    case 485: return true;
    default: return false;
  }
}

int HitecDServo::readSettings(HitecDSettings *settingsOut) {
  if (!attached()) {
    return HITECD_ERR_NOT_ATTACHED;
  }

  int res;
  uint16_t temp;

  /* Read ID */
  if ((res = readRawRegister(HD_REG_ID, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp > 255) {
    return HITECD_ERR_CONFUSED;
  }
  settingsOut->id = temp;

  /* Read counterclockwise */
  if ((res = readRawRegister(HD_REG_DIRECTION, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == HD_DIRECTION_CLOCKWISE) {
    settingsOut->counterclockwise = false;
  } else if (temp == HD_DIRECTION_COUNTERCLOCKWISE) {
    settingsOut->counterclockwise = true;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read speed */
  if ((res = readRawRegister(HD_REG_SPEED, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0FFF) {
    settingsOut->speed = 100;
  } else if (temp < 20) {
    settingsOut->speed = temp*5;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read deadband. There are three deadband-related registers; their values are
  expected to be consistent with each other. */
  uint16_t deadband_1, deadband_2, deadband_3;
  if ((res = readRawRegister(HD_REG_DEADBAND_1, &deadband_1)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(HD_REG_DEADBAND_2, &deadband_2)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(HD_REG_DEADBAND_3, &deadband_3)) != HITECD_OK) {
    return res;
  }
  if (deadband_1 == 1 && deadband_2 == 5 && deadband_3 == 11) {
    settingsOut->deadband = 1;
  } else if (deadband_1 >= 4 && deadband_1 <= 36 && deadband_1 % 4 == 0 &&
      deadband_2 == deadband_1 + 4 && deadband_3 == deadband_1 + 10) {
    settingsOut->deadband = deadband_1 / 4 + 1;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read softStart */
  if ((res = readRawRegister(HD_REG_SOFT_START, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == HD_SOFT_START_20) {
    settingsOut->softStart = 20;
  } else if (temp == HD_SOFT_START_40) {
    settingsOut->softStart = 40;
  } else if (temp == HD_SOFT_START_60) {
    settingsOut->softStart = 60;
  } else if (temp == HD_SOFT_START_80) {
    settingsOut->softStart = 80;
  } else if (temp == HD_SOFT_START_100) {
    settingsOut->softStart = 100;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read rangeLeftAPV, rangeRightAPV, rangeCenterAPV */
  if ((res = readRawRegister(HD_REG_RANGE_LEFT_APV, &temp)) != HITECD_OK) {
    return res;
  }
  settingsOut->rangeLeftAPV = rangeLeftAPV = temp;

  if ((res = readRawRegister(HD_REG_RANGE_RIGHT_APV, &temp)) != HITECD_OK) {
    return res;
  }
  settingsOut->rangeRightAPV = rangeRightAPV = temp;

  if ((res = readRawRegister(HD_REG_RANGE_CENTER_APV, &temp)) != HITECD_OK) {
    return res;
  }
  settingsOut->rangeCenterAPV = rangeCenterAPV = temp;

  /* Read failSafe and failSafeLimp. (A single register controls both.) */
  if ((res = readRawRegister(HD_REG_FAIL_SAFE, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp >= 850 && temp <= 2150) {
    settingsOut->failSafe = temp;
    settingsOut->failSafeLimp = false;
  } else if (temp == 0) {
    settingsOut->failSafe = 0;
    settingsOut->failSafeLimp = true;
  } else if (temp == 1) {
    settingsOut->failSafe = 0;
    settingsOut->failSafeLimp = false;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read powerLimit */
  if ((res = readRawRegister(HD_REG_POWER_LIMIT, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0FFF) {
    settingsOut->powerLimit = 100;
  } else {
    /* Divide rounding up, so nonzero values stay nonzero */
    settingsOut->powerLimit = (temp + 19) / 20;
  }

  /* Read overloadProtection */
  if ((res = readRawRegister(HD_REG_OVERLOAD_PROTECTION, &temp)) != HITECD_OK) {
    return res;
  }
  settingsOut->overloadProtection = temp;

  /* Read smartSense. smartSense is controlled by two registers, 0x6C and 0x44.
  If smartSense is enabled, these should be set to values read from two
  read-only registers, 0xD4 and 0xD6. If smartSense is disabled, these should be
  set to values read from two other read-only registers, 0x8A and 0x8C. So we
  read all six registers and confirm the values follow one of the two expected
  patterns. */
  uint16_t ss_1, ss_2, ss_enable_1, ss_enable_2, ss_disable_1, ss_disable_2;
  if ((res = readRawRegister(HD_REG_SMART_SENSE_1, &ss_1)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(HD_REG_SMART_SENSE_2, &ss_2)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(HD_REG_SS_ENABLE_1, &ss_enable_1)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(HD_REG_SS_ENABLE_2, &ss_enable_2)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(HD_REG_SS_DISABLE_1, &ss_disable_1)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(HD_REG_SS_DISABLE_2, &ss_disable_2)) != HITECD_OK) {
    return res;
  }
  if (ss_1 == ss_enable_1 && ss_2 == ss_enable_2) {
    settingsOut->smartSense = true;
  } else if (ss_1 == ss_disable_1 && ss_2 == ss_disable_2) {
    settingsOut->smartSense = false;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read sensitivityRatio */
  if ((res = readRawRegister(HD_REG_SENSITIVITY_RATIO, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp >= HD_SENSITIVITY_RATIO_MIN && temp <= HD_SENSITIVITY_RATIO_MAX) {
    settingsOut->sensitivityRatio = temp;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  return HITECD_OK;
}

int HitecDServo::writeSettings(const HitecDSettings &settings) {
  return writeSettingsUnsupportedModelThisMightDamageTheServo(settings, false);
}

int HitecDServo::writeSettingsUnsupportedModelThisMightDamageTheServo(
  const HitecDSettings &settings,
  bool allowUnsupportedModel
) {
  int res;
  uint16_t temp;

  if (!attached()) {
    return HITECD_ERR_NOT_ATTACHED;
  }

  if (!isModelSupported() && !allowUnsupportedModel) {
    return HITECD_ERR_UNSUPPORTED_MODEL;
  }

  /* Reset to factory defaults. (We'll then ignore any settings that are already
  at the factory defaults.) */
  writeRawRegister(HD_REG_FACTORY_RESET, HD_FACTORY_RESET_CONST);

  /* The DPC-11 always writes these registers to these constants after a factory
  reset, and also whenever it changes overloadProtection. I'm not sure why, but
  we do the same to be safe. */
  writeRawRegister(HD_REG_MYSTERY_OP1, HD_MYSTERY_OP1_CONST);
  writeRawRegister(HD_REG_MYSTERY_OP2, HD_MYSTERY_OP2_CONST);

  /* Write id */
  if (settings.id != HitecDSettings::defaultId) {
    writeRawRegister(HD_REG_ID, settings.id);
  }

  /* Write counterclockwise */
  if (settings.counterclockwise) {
    writeRawRegister(HD_REG_DIRECTION, HD_DIRECTION_COUNTERCLOCKWISE);
  }

  /* Write speed */
  if (settings.speed != HitecDSettings::defaultSpeed) {
    writeRawRegister(HD_REG_SPEED, settings.speed / 5);
  }

  /* Write deadband */
  if (settings.deadband != HitecDSettings::defaultDeadband) {
    /* The DPC-11 always writes this register to the this constant whenever it
    changes the deadband. I'm not sure why, but we do the same to be safe. */
    writeRawRegister(HD_REG_MYSTERY_DB, HD_MYSTERY_DB_CONST);
 
    /* Note, these formulas are wrong if deadband=1, but deadband=1 is the
    factory default, so that's OK. */
    writeRawRegister(HD_REG_DEADBAND_1, 4 * settings.deadband - 4);
    writeRawRegister(HD_REG_DEADBAND_2, 4 * settings.deadband);
    writeRawRegister(HD_REG_DEADBAND_3, 4 * settings.deadband + 6);
  }

  /* Write softStart */
  if (settings.softStart != HitecDSettings::defaultSoftStart) {
    /* Note, we omit the softStart=20 case because it's the factory default. */
    if (settings.softStart == 40) {
      writeRawRegister(HD_REG_SOFT_START, HD_SOFT_START_40);
    } else if (settings.softStart == 60) {
      writeRawRegister(HD_REG_SOFT_START, HD_SOFT_START_60);
    } else if (settings.softStart == 80) {
      writeRawRegister(HD_REG_SOFT_START, HD_SOFT_START_80);
    } else if (settings.softStart == 100) {
      writeRawRegister(HD_REG_SOFT_START, HD_SOFT_START_100);
    }
  }

  /* Write rangeLeftAPV, rangeRightAPV, and rangeCenterAPV. Also, update the
  instance variables that we initialized in attach(). (If we're using the
  default values, then read them back into instance variables, so we have the
  correct default values.) */
  if (settings.rangeLeftAPV != -1 &&
      settings.rangeLeftAPV !=
        HitecDSettings::defaultRangeLeftAPV(modelNumber)) {
    writeRawRegister(HD_REG_RANGE_LEFT_APV, settings.rangeLeftAPV);
    rangeLeftAPV = settings.rangeLeftAPV;
  } else {
    if ((res = readRawRegister(HD_REG_RANGE_LEFT_APV, &temp)) != HITECD_OK) {
      return res;
    }
    rangeLeftAPV = temp;
  }

  if (settings.rangeRightAPV != -1 &&
      settings.rangeRightAPV !=
        HitecDSettings::defaultRangeRightAPV(modelNumber)) {
    writeRawRegister(HD_REG_RANGE_RIGHT_APV, settings.rangeRightAPV);
    rangeRightAPV = settings.rangeRightAPV;
  } else {
     if ((res = readRawRegister(HD_REG_RANGE_RIGHT_APV, &temp)) != HITECD_OK) {
      return res;
    }
    rangeRightAPV = temp;
  }

  if (settings.rangeCenterAPV != -1 &&
      settings.rangeCenterAPV !=
        HitecDSettings::defaultRangeCenterAPV(modelNumber)) {
    writeRawRegister(HD_REG_RANGE_CENTER_APV, settings.rangeCenterAPV);
    rangeCenterAPV = settings.rangeCenterAPV;
  } else {
    if ((res = readRawRegister(HD_REG_RANGE_CENTER_APV, &temp)) != HITECD_OK) {
      return res;
    }
    rangeCenterAPV = temp;
  }

  /* Write failSafe and failSafeLimp (controlled by same register) */
  if (settings.failSafe != 0) {
    writeRawRegister(HD_REG_FAIL_SAFE, settings.failSafe);
  } else if (settings.failSafeLimp) {
    writeRawRegister(HD_REG_FAIL_SAFE, HD_FAIL_SAFE_LIMP);
  }

  /* Write powerLimit */
  if (settings.powerLimit != HitecDSettings::defaultPowerLimit) {
    writeRawRegister(HD_REG_POWER_LIMIT, settings.powerLimit * 20);
  }

  /* Write overloadProtection */
  if (settings.overloadProtection !=
      HitecDSettings::defaultOverloadProtection) {
    writeRawRegister(HD_REG_OVERLOAD_PROTECTION, settings.overloadProtection);
  }

  /* Write smartSense */
  if (!settings.smartSense) {
    /* The DPC-11 always writes this register to the this constant whenever it
    enables or disables smartSense. I'm not sure why, but we do the same to be
    safe. */
    writeRawRegister(HD_REG_MYSTERY_DB, HD_MYSTERY_DB_CONST);

    /* To disable smartSense, we have to read magic numbers from the two
    SS_DISABLE_* registers and write them to the SMART_SENSE_* registers. */
    if ((res = readRawRegister(HD_REG_SS_DISABLE_1, &temp)) != HITECD_OK) {
      return res;
    }
    writeRawRegister(HD_REG_SMART_SENSE_1, temp);

    if ((res = readRawRegister(HD_REG_SS_DISABLE_2, &temp)) != HITECD_OK) {
      return res;
    }
    writeRawRegister(HD_REG_SMART_SENSE_2, temp);
  }

  /* Write sensitivityRatio */
  if (settings.sensitivityRatio != HitecDSettings::defaultSensitivityRatio) {
    writeRawRegister(HD_REG_SENSITIVITY_RATIO, settings.sensitivityRatio);
  }

  /* Save new settings to EEPROM */
  writeRawRegister(HD_REG_SAVE, HD_SAVE_CONST);

  /* Reboot the servo so the new settings take effect */
  writeRawRegister(HD_REG_REBOOT, HD_REBOOT_CONST);

  /* After writing to REBOOT, the servo will take 1000ms to boot. During this
  time, it won't respond to commands. The caller is responsible for waiting
  1000ms before trying to issue any commands. */

  /* After 1000ms elapses, the HPC-11 writes 0x1000 to register 0x22. I'm not
  sure what this is for; register 0x22 stores the calculated power limit (after
  applying overload protection) and writing to it appears to have no effect.
  This library doesn't write to register 0x22. */

  return HITECD_OK;
}

int HitecDServo::readRawRegister(uint8_t reg, uint16_t *valOut) {
  uint8_t oldSREG = SREG;
  cli();

  writeByte((uint8_t)0x96);
  writeByte((uint8_t)0x00);
  writeByte(reg);
  writeByte((uint8_t)0x00);
  uint8_t checksum = (0x00 + reg + 0x00) & 0xFF;
  writeByte(checksum);
  digitalWrite(pin, LOW);

  SREG = oldSREG;

  delay(14);

  /* Note, most of the pull-up current must actually provided by an external
  resistor; the microcontroller pullup by itself is nowhere near strong enough.
  We use INPUT_PULLUP anyway because that lets us detect the absence of a servo
  even if the pullup resistor is also absent. */
  pinMode(pin, INPUT_PULLUP);

  /* At this point, the servo should be pulling the pin low. If the pin goes
  high when we release the line, then no servo is connected. */
  if (digitalRead(pin) != LOW) {
    delay(2);
    pinMode(pin, OUTPUT);
    return HITECD_ERR_NO_SERVO;
  }

  oldSREG = SREG;
  cli();

  int const0x69 = readByte();
  int mystery = readByte(); /* I don't know what this byte is for... */
  int reg2 = readByte();
  int const0x02 = readByte();
  int low = readByte();
  int high = readByte();
  int checksum2 = readByte();

  SREG = oldSREG;
  
  delay(1);

  /* At this point, the servo should have released the line, allowing the
  pullup resistor to pull it high. If the pin is not high, there are two
  possible reasons this could happen:
  1. The servo is booting. This takes 1 second from when the servo first
     receives power, or is reset via register 0x46. During this time, it will
     pull the line low and not respond to commands.
  2. The pullup resistor is missing. */
  if (digitalRead(pin) != HIGH) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    return HITECD_ERR_BOOTING_OR_NO_PULLUP;
  }

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(1);

  /* Note, readByte() can return HITECD_ERR_NO_SERVO if it times out. But, we
  know the servo is present, or else we'd have hit either HITECD_ERR_NO_SERVO or
  HITECD_ERR_BOOTING_OR_NO_PULLUP above. So this is unlikely to happen unless
  something's horribly wrong. So for simplicity, we just round this off to
  HITECD_ERR_CORRUPT. */

  Serial.print("readRawRegister() reg=0x");
  Serial.print(reg, HEX);
  Serial.print(" const0x69=0x");
  Serial.print(const0x69, HEX);
  Serial.print(" mystery=0x");
  Serial.print(mystery, HEX);
  Serial.print(" reg2=0x");
  Serial.print(reg2, HEX);
  Serial.print(" const0x02=0x");
  Serial.print(const0x02, HEX);
  Serial.print(" low=0x");
  Serial.print(low, HEX);
  Serial.print(" high=0x");
  Serial.print(high, HEX);
  Serial.print(" checksum2=0x");
  Serial.println(checksum2, HEX);

  if (const0x69 != 0x69) return HITECD_ERR_CORRUPT;
  if (mystery < 0) return HITECD_ERR_CORRUPT;
  if (reg2 != reg) return HITECD_ERR_CORRUPT;
  if (const0x02 != 0x02) return HITECD_ERR_CORRUPT;
  if (low < 0) return HITECD_ERR_CORRUPT;
  if (high < 0) return HITECD_ERR_CORRUPT;
  if (checksum2 != ((mystery + reg2 + const0x02 + low + high) & 0xFF)) {
    return HITECD_ERR_CORRUPT;
  }

  *valOut = low + (high << 8);
  return HITECD_OK;
}

void HitecDServo::writeRawRegister(uint8_t reg, uint16_t val) {
  uint8_t oldSREG = SREG;
  cli();

  writeByte((uint8_t)0x96);
  writeByte((uint8_t)0x00);
  writeByte(reg);
  writeByte((uint8_t)0x02);
  uint8_t low = val & 0xFF;
  writeByte(low);
  uint8_t high = (val >> 8) & 0xFF;
  writeByte(high);
  uint8_t checksum = (0x00 + reg + 0x02 + low + high) & 0xFF;
  writeByte(checksum);

  SREG = oldSREG;

  digitalWrite(pin, LOW);
  delay(1);
}

#ifdef ARDUINO_ARCH_AVR

/* We're bit-banging a 115200 baud serial connection, so we need precise timing.
The AVR libraries have a macro _delay_us() that delays a precise number of
microseconds, using compile-time floating-point math. However, we also need to
compensate for the time spent executing non-noop instructions, which depends on
the CPU frequency. DELAY_US_COMPENSATED(us, cycles) will delay for an amount of
time such that if 'cycles' non-noop instruction cycles are executed, the total
time elapsed will be 'us'. */
#define DELAY_US_COMPENSATED(us, cycles) _delay_us((us) - (cycles)/(F_CPU/1e6))

int HitecDServo::readByte() {
  /* Wait up to 50ms for start bit. The "/ 10" factor arises because this loop
  empirically takes somewhere on the order of 10 clock cycles per iteration.
  (In theory we should only need to wait up to about 10ms, but the number of
  clock-cycles-per-iteration is very imprecise, so we add a lot of padding.) */
  int timeoutCounter = F_CPU * 0.050 / 10;
  while (!(*pinInputRegister & pinBitMask)) {
    if (--timeoutCounter == 0) {
      return HITECD_ERR_NO_SERVO;
    }
  }

  /* Delay until approximate center of first data bit. */
  DELAY_US_COMPENSATED(8.68*1.5, 32);

  /* Read data bits */
  uint8_t val = 0;
  for (int m = 0x001; m != 0x100; m <<= 1) {
    if(!(*pinInputRegister & pinBitMask)) {
      val |= m;
    }
    DELAY_US_COMPENSATED(8.68, 19);
  }

  /* We expect to see stop bit (low) */
  if (*pinInputRegister & pinBitMask) {
    Serial.println("readByte(): Missing stop bit!!")
    return HITECD_ERR_CORRUPT;
  }

  return val;
}

void HitecDServo::writeByte(uint8_t val) {
  /* Write start bit. Note polarity is inverted, so start bit is HIGH. */
  *pinOutputRegister |= pinBitMask;

  /* We're operating at 115200 baud, so theoretically there should be an 8.68us
  interval between edges. In practice, this loop seems to take about 25 clock
  cycles per iteration, so compensate for that. */
  DELAY_US_COMPENSATED(8.68, 25);

  for (int m = 0x001; m != 0x100; m <<= 1) {
    if (val & m) {
      *pinOutputRegister &= ~pinBitMask;
    } else {
      *pinOutputRegister |= pinBitMask;
    }
    DELAY_US_COMPENSATED(8.68, 25);
  }

  /* Write stop bit. */
  *pinOutputRegister &= ~pinBitMask;
  DELAY_US_COMPENSATED(8.68, 25);
}

#else
#error "HitecDServo library only works on AVR processors."
#endif

HitecDSettings::HitecDSettings() :
  id(defaultId),
  counterclockwise(defaultCounterclockwise),
  speed(defaultSpeed),
  deadband(defaultDeadband),
  softStart(defaultSoftStart),
  rangeLeftAPV(-1),
  rangeRightAPV(-1),
  rangeCenterAPV(-1),
  failSafe(defaultFailSafe),
  failSafeLimp(defaultFailSafeLimp),
  powerLimit(defaultPowerLimit),
  overloadProtection(defaultOverloadProtection),
  smartSense(defaultSmartSense),
  sensitivityRatio(defaultSensitivityRatio)
{ }

int16_t HitecDSettings::defaultRangeLeftAPV(int modelNumber) {
  switch (modelNumber) {
    case 485: return 3381;
    default: return -1;
  }
}

int16_t HitecDSettings::defaultRangeRightAPV(int modelNumber) {
  switch (modelNumber) {
    case 485: return 13002;
    default: return -1;
  }
}

int16_t HitecDSettings::defaultRangeCenterAPV(int modelNumber) {
  switch (modelNumber) {
    case 485: return 8192;
    default: return -1;
  }
}

int16_t HitecDSettings::widestRangeLeftAPV(int modelNumber) {
  switch (modelNumber) {
    /* I measured 731, and added +50 as a margin of error */
    case 485: return 731 + 50;
    default: return -1;
  }
}

int16_t HitecDSettings::widestRangeRightAPV(int modelNumber) {
  int16_t min = widestRangeLeftAPV(modelNumber);
  if (min == -1) {
    return -1;
  }
  return 0x3FFF - min;
}

int16_t HitecDSettings::widestRangeCenterAPV(int modelNumber) {
  if (widestRangeLeftAPV(modelNumber) == -1) {
    return -1;
  }
  return 8192;
}

const __FlashStringHelper *hitecdErrToString(int err) {
  if (err >= 0) {
    return F("OK");
  }
  switch (err) {
    case HITECD_ERR_NOT_ATTACHED:
      return F("attach() was not called, or the call to attach() failed.");
    case HITECD_ERR_NO_SERVO:
      return F("No servo detected.");
    case HITECD_ERR_BOOTING_OR_NO_PULLUP:
      return F("Either the servo is still booting, which takes 1000ms; or the "
        "pullup resistor is missing. With a 5V microcontroller, use a 2k "
        "pullup resistor to +5V. With a 3.3V microcontroller, use a 1k pullup "
        "resistor to +3.3V.");
    case HITECD_ERR_CORRUPT:
      return F("Corrupt response from servo.");
    case HITECD_ERR_UNSUPPORTED_MODEL:
      return F("Unsupported model of servo.");
    case HITECD_ERR_CONFUSED:
      return F("Confusing response from servo.");
    default:
      return F("Unknown error.");
  }
}
