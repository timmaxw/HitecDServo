#include "HitecDServo.h"

const char *hitecdErrToString(int err) {
  if (err >= 0) {
    return "OK";
  }
  switch (err) {
    case HITECD_ERR_NOT_ATTACHED:
      return "attach() was not called, or the call to attach() failed.";
    case HITECD_ERR_NO_SERVO:
      return "No servo detected.";
    case HITECD_ERR_NO_PULLUP:
      return "Missing pullup resistor.";
    case HITECD_ERR_CORRUPT:
      return "Corrupt response from servo.";
    case HITECD_ERR_UNSUPPORTED_MODEL:
      return "Unsupported model of servo.";
    case HITECD_ERR_CONFUSED:
      return "Confusing response from servo.";
    default:
      return "Unknown error.";
  }
}

HitecDServoConfig::HitecDServoConfig() :
  id(defaultId),
  counterclockwise(defaultCounterclockwise),
  speed(defaultSpeed),
  deadband(defaultDeadband),
  softStart(defaultSoftStart),
  rawAngleFor850(-1),
  rawAngleFor1500(-1),
  rawAngleFor2150(-1),
  failSafe(defaultFailSafe),
  failSafeLimp(defaultFailSafeLimp),
  powerLimit(defaultPowerLimit),
  overloadProtection(defaultOverloadProtection),
  smartSense(defaultSmartSense),
  sensitivityRatio(defaultSensitivityRatio)
{ }

int16_t HitecDServoConfig::defaultRawAngleFor850(int modelNumber) {
  switch (modelNumber) {
    case 485: return 3381;
    default: return -1;
  }
}

int16_t HitecDServoConfig::defaultRawAngleFor1500(int modelNumber) {
  switch (modelNumber) {
    case 485: return 8192;
    default: return -1;
  }
}

int16_t HitecDServoConfig::defaultRawAngleFor2150(int modelNumber) {
  switch (modelNumber) {
    case 485: return 13002;
    default: return -1;
  }
}

int16_t HitecDServoConfig::safeMinRawAngle(int modelNumber) {
  switch (modelNumber) {
    /* I measured 731, and added +50 as a margin of error */
    case 485: return 731 + 50;
    default: return -1;
  }
}

int16_t HitecDServoConfig::safeMaxRawAngle(int modelNumber) {
  int16_t min = safeMinRawAngle(modelNumber);
  if (min == -1) {
    return -1;
  }
  return 0x3FFF - min;
}

HitecDServo::HitecDServo() : pin(-1) { }

int HitecDServo::attach(int _pin) {
  if (attached()) {
    detach();
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

  if ((res = readRawRegister(0x00, &temp)) != HITECD_OK) {
    detach();
    return res;
  }
  modelNumber = temp;

  if ((res = readRawRegister(0xB2, &temp)) != HITECD_OK) {
    detach();
    return res;
  }
  rawAngleFor850 = temp;

  if ((res = readRawRegister(0xC2, &temp)) != HITECD_OK) {
    detach();
    return res;
  }
  rawAngleFor1500 = temp;

  if ((res = readRawRegister(0xB0, &temp)) != HITECD_OK) {
    detach();
    return res;
  }
  rawAngleFor2150 = temp;

  return HITECD_OK;
}

bool HitecDServo::attached() {
  return (pin != -1);
}

void HitecDServo::detach() {
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
  writeRawRegister(0x1E, quarterMicros - 3000);
}

int16_t HitecDServo::readCurrentMicroseconds() {
  int16_t quarterMicros = readCurrentQuarterMicros();
  if (quarterMicros < 0) {
    return quarterMicros;
  }
  return quarterMicros / 4;
}

int16_t HitecDServo::readCurrentQuarterMicros() {
  int16_t rawAngle = readCurrentRawAngle();
  if (rawAngle < 0) {
    return rawAngle;
  }
  if (rawAngle < rawAngleFor1500) {
    return map(rawAngle, rawAngleFor850, rawAngleFor1500, 4*850, 4*1500);
  } else {
    return map(rawAngle, rawAngleFor1500, rawAngleFor2150, 4*1500, 4*2150);
  }
}

int16_t HitecDServo::readCurrentRawAngle() {
  if (!attached()) {
    return HITECD_ERR_NOT_ATTACHED;
  }
  int res;
  uint16_t currentRawAngle;
  if ((res = readRawRegister(0x0C, &currentRawAngle)) != HITECD_OK) {
    return res;
  }
  return currentRawAngle;
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

int HitecDServo::readConfig(HitecDServoConfig *configOut) {
  if (!attached()) {
    return HITECD_ERR_NOT_ATTACHED;
  }

  int res;
  uint16_t temp;

  /* Read ID */
  if ((res = readRawRegister(0x32, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp & 0xFF00) {
    return HITECD_ERR_CONFUSED;
  }
  configOut->id = temp & 0xFF;

  /* Read counterclockwise */
  if ((res = readRawRegister(0x5E, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0000) {
    configOut->counterclockwise = false;
  } else if (temp == 0x0001) {
    configOut->counterclockwise = true;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read speed */
  if ((res = readRawRegister(0x54, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0FFF) {
    configOut->speed = 100;
  } else if (temp <= 0x0012) {
    configOut->speed = temp*5;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read deadband. There are three deadband-related registers; their values are
  expected to be consistent with each other. */
  uint16_t temp0x4E, temp0x66, temp0x68;
  if ((res = readRawRegister(0x4E, &temp0x4E)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(0x66, &temp0x66)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(0x68, &temp0x68)) != HITECD_OK) {
    return res;
  }
  if (temp0x4E == 0x0001 && temp0x66 == 0x0005 && temp0x68 == 0x000B) {
    configOut->deadband = 1;
  } else if (temp0x4E >= 0x0004 && temp0x4E <= 0x0024 && (temp0x4E % 4) == 0 &&
      temp0x66 == temp0x4E + 0x0004 && temp0x68 == temp0x4E + 0x000A) {
    configOut->deadband = temp0x4E / 4 + 1;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read softStart */
  if ((res = readRawRegister(0x60, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0001) {
    configOut->softStart = 20;
  } else if (temp == 0x0003) {
    configOut->softStart = 40;
  } else if (temp == 0x0006) {
    configOut->softStart = 60;
  } else if (temp == 0x0008) {
    configOut->softStart = 80;
  } else if (temp == 0x0064) {
    configOut->softStart = 100;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read rawAngleFor850, rawAngleFor1500, rawAngleFor2150 */
  if ((res = readRawRegister(0xB2, &temp)) != HITECD_OK) {
    return res;
  }
  configOut->rawAngleFor850 = rawAngleFor850 = temp;
  if ((res = readRawRegister(0xC2, &temp)) != HITECD_OK) {
    return res;
  }
  configOut->rawAngleFor1500 = rawAngleFor1500 = temp;
  if ((res = readRawRegister(0xB0, &temp)) != HITECD_OK) {
    return res;
  }
  configOut->rawAngleFor2150 = rawAngleFor2150 = temp;

  /* Read failSafe and failSafeLimp. (A single register controls both.) */
  if ((res = readRawRegister(0x4C, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp >= 850 && temp <= 2150) {
    configOut->failSafe = temp;
    configOut->failSafeLimp = false;
  } else if (temp == 0) {
    configOut->failSafe = 0;
    configOut->failSafeLimp = true;
  } else if (temp == 1) {
    configOut->failSafe = 0;
    configOut->failSafeLimp = false;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read powerLimit */
  if ((res = readRawRegister(0x56, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0FFF) {
    configOut->powerLimit = 100;
  } else {
    /* Divide rounding up, so nonzero values stay nonzero */
    configOut->powerLimit = (temp + 19) / 20;
  }

  /* Read overloadProtection */
  if ((res = readRawRegister(0x9C, &temp)) != HITECD_OK) {
    return res;
  }
  configOut->overloadProtection = temp;

  /* Read smartSense. smartSense is controlled by two registers, 0x6C and 0x44.
  If smartSense is enabled, these should be set to values read from two
  read-only registers, 0xD4 and 0xD6. If smartSense is disabled, these should be
  set to values read from two other read-only registers, 0x8A and 0x8C. So we
  read all six registers and confirm the values follow one of the two expected
  patterns. */
  uint16_t temp0x6C, temp0x44, temp0xD4, temp0xD6, temp0x8A, temp0x8C;
  if ((res = readRawRegister(0x6C, &temp0x6C)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(0x44, &temp0x44)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(0xD4, &temp0xD4)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(0xD6, &temp0xD6)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(0x8A, &temp0x8A)) != HITECD_OK) {
    return res;
  }
  if ((res = readRawRegister(0x8C, &temp0x8C)) != HITECD_OK) {
    return res;
  }
  if (temp0x6C == temp0xD4 && temp0x44 == temp0xD6) {
    configOut->smartSense = true;
  } else if (temp0x6C == temp0x8A && temp0x44 == temp0x8C) {
    configOut->smartSense = false;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  /* Read sensitivityRatio */
  if ((res = readRawRegister(0x64, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp >= 819 && temp <= 4095) {
    configOut->sensitivityRatio = temp;
  } else {
    return HITECD_ERR_CONFUSED;
  }

  return HITECD_OK;
}

int HitecDServo::writeConfig(const HitecDServoConfig &config) {
  return writeConfigUnsupportedModelThisMightDamageTheServo(config, false);
}

int HitecDServo::writeConfigUnsupportedModelThisMightDamageTheServo(
  const HitecDServoConfig &config,
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
  writeRawRegister(0x6E, 0x0F0F);

  /* The DPC-11 always writes these registers to these constants after a factory
  reset, and also whenever it changes overloadProtection. I'm not sure why, but
  we do the same to be safe. */
  writeRawRegister(0x98, 0x00C8);
  writeRawRegister(0x9A, 0x0003);

  /* Write id */
  if (config.id != HitecDServoConfig::defaultId) {
    writeRawRegister(0x32, config.id);
  }

  /* Write counterclockwise */
  if (config.counterclockwise) {
    writeRawRegister(0x5E, 0x0001);
  }

  /* Write speed */
  if (config.speed != HitecDServoConfig::defaultSpeed) {
    writeRawRegister(0x54, config.speed / 5);
  }

  /* Write deadband */
  if (config.deadband != HitecDServoConfig::defaultDeadband) {
    /* The DPC-11 always writes this register to the this constant whenever it
    changes the deadband. I'm not sure why, but we do the same to be safe. */
    writeRawRegister(0x72, 0x4E54);
 
    /* Note, these formulas are wrong if deadband=1, but deadband=1 is the
    factory default, so that's OK. */
    writeRawRegister(0x4E, 0x0004*config.deadband - 0x0004);
    writeRawRegister(0x66, 0x0004*config.deadband);
    writeRawRegister(0x68, 0x0004*config.deadband + 0x0006);
  }

  /* Write softStart */
  if (config.softStart != HitecDServoConfig::defaultSoftStart) {
    /* Note, we omit the softStart=20 case because it's the factory default. */
    if (config.softStart == 40) {
      writeRawRegister(0x60, 0x0003);
    } else if (config.softStart == 60) {
      writeRawRegister(0x60, 0x0006);
    } else if (config.softStart == 80) {
      writeRawRegister(0x60, 0x0008);
    } else if (config.softStart == 100) {
      writeRawRegister(0x60, 0x0064);
    }
  }

  /* Write rawAngleFor850, rawAngleFor1500, and rawAngleFor2150. Also, update
  the instance variables that we initialized in attach(). (If we're using the
  default values, then read them back into instance variables, so we have the
  correct default values.) */
  if (config.rawAngleFor850 != -1 &&
      config.rawAngleFor850 !=
        HitecDServoConfig::defaultRawAngleFor850(modelNumber)) {
    writeRawRegister(0xB2, config.rawAngleFor850);
    rawAngleFor850 = config.rawAngleFor850;
  } else {
    if ((res = readRawRegister(0xB2, &temp)) != HITECD_OK) {
      return res;
    }
    rawAngleFor850 = temp;
  }
  if (config.rawAngleFor1500 != -1 &&
      config.rawAngleFor1500 !=
        HitecDServoConfig::defaultRawAngleFor1500(modelNumber)) {
    writeRawRegister(0xC2, config.rawAngleFor1500);
    rawAngleFor1500 = config.rawAngleFor1500;
  } else {
    if ((res = readRawRegister(0xC2, &temp)) != HITECD_OK) {
      return res;
    }
    rawAngleFor1500 = temp;
  }
  if (config.rawAngleFor2150 != -1 &&
      config.rawAngleFor2150 !=
        HitecDServoConfig::defaultRawAngleFor2150(modelNumber)) {
    writeRawRegister(0xB0, config.rawAngleFor2150);
    rawAngleFor2150 = config.rawAngleFor2150;
  } else {
     if ((res = readRawRegister(0xB0, &temp)) != HITECD_OK) {
      return res;
    }
    rawAngleFor2150 = temp;
  }

  /* Write failSafe and failSafeLimp (controlled by same register) */
  if (config.failSafe != HitecDServoConfig::defaultFailSafe) {
    writeRawRegister(0x4C, config.failSafe);
  } else if (config.failSafeLimp != HitecDServoConfig::defaultFailSafeLimp) {
    writeRawRegister(0x4C, 0x0000);
  }

  /* Write powerLimit */
  if (config.powerLimit != HitecDServoConfig::defaultPowerLimit) {
    writeRawRegister(0x56, config.powerLimit * 20);
  }

  /* Write overloadProtection */
  if (config.overloadProtection !=
      HitecDServoConfig::defaultOverloadProtection) {
    writeRawRegister(0x9C, config.overloadProtection);
  }

  /* Write smartSense */
  if (!config.smartSense) {
    /* The DPC-11 always writes this register to the this constant whenever it
    enables or disables smartSense. I'm not sure why, but we do the same to be
    safe. */
    writeRawRegister(0x72, 0x4E54);

    /* To disable smartSense, we have to write 0x6C and 0x44 to magic numbers
    that we read from two read-only registers, 0x8A and 0x8C. */
    if ((res = readRawRegister(0x8A, &temp)) != HITECD_OK) {
      return res;
    }
    writeRawRegister(0x6C, temp);

    if ((res = readRawRegister(0x8C, &temp)) != HITECD_OK) {
      return res;
    }
    writeRawRegister(0x44, temp);
  }

  /* Write sensitivityRatio */
  if (config.sensitivityRatio != HitecDServoConfig::defaultSensitivityRatio) {
    writeRawRegister(0x64, config.sensitivityRatio);
  }

  /* Writing 0xFFFF to 0x70 tells the servo to save its settings to EEPROM */
  writeRawRegister(0x70, 0xFFFF);

  /* Writing 0x0001 to 0x46 tells the servo to reset itself (necessary after
  some settings changes) */
  writeRawRegister(0x46, 0x0001);

  /* After the servo resets itself, it won't respond to any commands for 1
  second. */
  /* TODO: Instead of this delay, detect servo is booting and report better
  errors. */
  delay(1000);

  /* I don't know what writing 0x1000 to 0x22 does, but the HPC-11 does it after
  some settings changes, so we do it too, just to be safe. */
  /* TODO: Probably remove this. */
  writeRawRegister(0x22, 0x1000);

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

  /* Note, most of the pull-up force is actually provided by the 2k resistor;
  the microcontroller pullup by itself is nowhere near strong enough. */
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
  2k resistor to pull it high. If the pin is not high, probably no resistor is
  present. */
  if (digitalRead(pin) != HIGH) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    return HITECD_ERR_NO_PULLUP;
  }

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(1);

  /* Note, readByte() can return HITECD_ERR_NO_SERVO if it times out. But, we
  know the servo is present, or else we'd have hit either HITECD_ERR_NO_SERVO or
  HITECD_ERR_NO_PULLUP above. So this is unlikely to happen unless something's
  horribly wrong. So for simplicity, we just round this off to
  HITECD_ERR_CORRUPT. */

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

/* We're bit-banging a 115200 baud serial connection, so we need precise timing.
The AVR libraries have a macro _delay_us() that delays a precise number of
microseconds, using compile-time floating-point math. However, we also need to
compensate for the time spent executing non-noop instructions, which depends on
the CPU frequency. DELAY_US_COMPENSATED(us, cycles) will delay for an amount of
time such that if 'cycles' non-noop instruction cycles are executed, the total
time elapsed will be 'us'. */
#define DELAY_US_COMPENSATED(us, cycles) _delay_us((us) - (cycles)/(F_CPU/1e6))

int HitecDServo::readByte() {
  /* Wait up to 10ms for start bit. The "/ 15" factor arises because this loop
  empirically takes about 15 clock cycles per iteration. */
  int timeoutCounter = F_CPU * 0.010 / 15;
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

