#include "HitecDServo.h"

HitecDServo::HitecDServo() : pin(-1) { }

void HitecDServo::attach(int _pin) {
  pin = _pin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);

  pinBitMask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  pinInputRegister = portInputRegister(port);
  pinOutputRegister = portOutputRegister(port);
}

bool HitecDServo::attached() {
  return (pin != -1);
}

void HitecDServo::detach() {
  pin = -1;
}

int HitecDServo::readModelNumber() {
  int res;
  uint16_t modelNumber;
  if ((res = readReg(0x00, &modelNumber)) != HITECD_OK) {
    return res;
  }
  return modelNumber;
}

int HitecDServo::readConfig(HitecDServoConfig *configOut) {
  int res;
  uint16_t temp, temp2;

  if ((res = readReg(0x32, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp & 0xFF00) {
    return HITECD_ERR_CORRUPT;
  }
  configOut->id = temp & 0xFF;

  if ((res = readReg(0x5E, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0000) {
    configOut->counterclockwise = false;
  } else if (temp == 0x0001) {
    configOut->counterclockwise = true;
  } else {
    return HITECD_ERR_CORRUPT;
  }

  if ((res = readReg(0x54, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0FFF) {
    configOut->speed = 100;
  } else if (temp <= 0x0012) {
    configOut->speed = temp*5;
  } else {
    return HITECD_ERR_CORRUPT;
  }

  if ((res = readReg(0x4E, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x0001) {
    configOut->deadband = 1;
  } else if (temp >= 0x0004 && temp <= 0x0024 && (temp % 4) == 0) {
    configOut->deadband = temp / 4 + 1;
  } else {
    return HITECD_ERR_CORRUPT;
  }

  if ((res = readReg(0x60, &temp)) != HITECD_OK) {
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
    return HITECD_ERR_CORRUPT;
  }

  if ((res = readReg(0xB2, &temp)) != HITECD_OK) {
    return res;
  }
  configOut->leftPoint = temp;

  if ((res = readReg(0xC2, &temp)) != HITECD_OK) {
    return res;
  }
  configOut->centerPoint = temp;

  if ((res = readReg(0xB0, &temp)) != HITECD_OK) {
    return res;
  }
  configOut->rightPoint = temp;

  if ((res = readReg(0x4C, &temp)) != HITECD_OK) {
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
    return HITECD_ERR_CORRUPT;
  }

  if ((res = readReg(0x9C, &temp)) != HITECD_OK) {
    return res;
  }
  configOut->overloadProtection = temp;

  if ((res = readReg(0x6C, &temp)) != HITECD_OK) {
    return res;
  }
  if ((res = readReg(0x44, &temp2)) != HITECD_OK) {
    return res;
  }
  if (temp == 0x07D0 && temp2 == 0x36B0) {
    configOut->smartSense = true;
  } else if (temp == 0x0FA0 && temp2 == 0x6D60) {
    configOut->smartSense = false;
  } else {
    return HITECD_ERR_CORRUPT;
  }

  if ((res = readReg(0x64, &temp)) != HITECD_OK) {
    return res;
  }
  if (temp >= 819 && temp <= 4095) {
    configOut->sensitivityRatio = temp;
  } else {
    return HITECD_ERR_CORRUPT;
  }

  return HITECD_OK;
}

void HitecDServo::writeConfig(HitecDServoConfig config) {
  /* Reset to factory settings */
  writeReg(0x6E, 0x0F0F);

  /* Now write any values that differ from factory settings */
  if (config.id != 0) {
    writeReg(0x32, config.id);
  }

  if (config.counterclockwise) {
    writeReg(0x5E, 0x0001);
  }

  if (config.speed != 100) {
    writeReg(0x54, config.speed / 5);
  }

  if (config.deadband != 1) {
    writeReg(0x4E, 0x0004*config.deadband - 0x0004);
    writeReg(0x66, 0x0004*config.deadband);
    writeReg(0x68, 0x0004*config.deadband + 0x0006);
  }

  if (config.softStart == 40) {
    writeReg(0x60, 0x0003);
  } else if (config.softStart == 60) {
    writeReg(0x60, 0x0006);
  } else if (config.softStart == 80) {
    writeReg(0x60, 0x0008);
  } else if (config.softStart == 100) {
    writeReg(0x60, 0x0064);
  }

  if (config.leftPoint != 3381) {
    writeReg(0xB2, config.leftPoint);
  }
  if (config.centerPoint != 8192) {
    writeReg(0xC2, config.centerPoint);
  }
  if (config.rightPoint != 13002) {
    writeReg(0xB0, config.rightPoint);
  }

  if (config.failSafe) {
    writeReg(0x4C, config.failSafe);
  } else if (config.failSafeLimp) {
    writeReg(0x4C, 0x0000);
  }

  if (config.overloadProtection != 100) {
    /* TODO we don't write old value, is that OK? */
    writeReg(0x9C, config.overloadProtection);
    writeReg(0x98, 0x00C8); /* TODO wtf */
    writeReg(0x9A, 0x0003);
  }

  if (!config.smartSense) {
    /* TODO is this OK? */
    writeReg(0x72, 0x4E54);
    writeReg(0x6C, 0x0FA0);
    writeReg(0x44, 0x6D60);
  }

  if (config.sensitivityRatio != 4095) {
    writeReg(0x64, config.sensitivityRatio);
  }

  /* TODO explain */
  writeReg(0x70, 0xFFFF);
  writeReg(0x46, 0x0001);
  delay(1000); /* TODO try to shorten */
  writeReg(0x22, 0x0010);
}

/* We're bit-banging a 115200 baud serial connection, so we need precise timing.
The AVR libraries have a macro _delay_us() that delays a precise number of
microseconds, using compile-time floating-point math. However, we also need to
compensate for the time spent executing non-noop instructions, which depends on
the CPU frequency. DELAY_US_COMPENSATED(us, cycles) will delay for an amount of
time such that if 'cycles' non-noop instruction cycles are executed, the total
time elapsed will be 'us'. */
#define DELAY_US_COMPENSATED(us, cycles) _delay_us((us) - (cycles)/(F_CPU/1e6))

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

int HitecDServo::readByte() {
  /* Wait up to 4ms for start bit. The "/ 15" factor arises because this loop
  empirically takes about 15 clock cycles per iteration. */
  int timeoutCounter = F_CPU * 0.004 / 15;
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

void HitecDServo::writeReg(uint8_t reg, uint16_t val) {
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

int HitecDServo::readReg(uint8_t reg, uint16_t *valOut) {
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
  int mystery = readByte();
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
    return HITECD_ERR_NO_RESISTOR;
  }

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(1);

  /* Note, readByte() can return HITECD_ERR_NO_SERVO if it times out. But, we
  know the servo is present, or else we'd have hit either HITECD_ERR_NO_SERVO or
  HITECD_ERR_NO_RESISTOR above. So this is unlikely to happen unless something's
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
