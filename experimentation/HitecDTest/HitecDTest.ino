#define PIN 2

uint8_t pinBitMask;
volatile uint8_t *pinInputRegister, *pinOutputRegister;

#define HITECD_ERR_NO_SERVO -1
#define HITECD_ERR_NO_RESISTOR -2
#define HITECD_ERR_CORRUPT -3

void setup() {
  Serial.begin(115200);

  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, LOW);
  pinBitMask = digitalPinToBitMask(PIN);
  uint8_t port = digitalPinToPort(PIN);
  pinInputRegister = portInputRegister(port);
  pinOutputRegister = portOutputRegister(port);
}

/* We're bit-banging a 115200 baud serial connection, so we need precise timing.
The AVR libraries have a macro _delay_us() that delays a precise number of
microseconds, using compile-time floating-point math. However, we also need to
compensate for the time spent executing instructions, which is a function of the
CPU time. DELAY_US_COMPENSATED(us, cycles) will delay for an amount of time such
that if 'cycles' instruction cycles are executed, the total time elapsed will be
'us'. */
#define DELAY_US_COMPENSATED(us, cycles) _delay_us((us) - (cycles)/(F_CPU/1e6))

void writeByte(uint8_t value) {
  /* Write start bit. Note polarity is inverted, so start bit is HIGH. */
  *pinOutputRegister |= pinBitMask;

  /* We're operating at 115200 baud, so theoretically there should be an 8.68us
  interval between edges. In practice, this loop seems to take about 25 clock
  cycles per iteration, so compensate for that. */
  DELAY_US_COMPENSATED(8.68, 25);

  for (int m = 0x001; m != 0x100; m <<= 1) {
    if (value & m) {
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

int readByte() {
  /* Wait up to 4ms for start bit. The "/ 15" factor arises because this loop
  empirically takes about 15 clock cycles per iteration. */
  int timeout_counter = F_CPU * 0.004 / 15;
  while (!(*pinInputRegister & pinBitMask)) {
    if (--timeout_counter == 0) {
      return HITECD_ERR_NO_SERVO;
    }
  }

  /* Delay until approximate center of first data bit. */
  DELAY_US_COMPENSATED(8.68*1.5, 32);

  /* Read data bits */
  uint8_t value = 0;
  for (int m = 0x001; m != 0x100; m <<= 1) {
    if(!(*pinInputRegister & pinBitMask)) {
      value |= m;
    }
    DELAY_US_COMPENSATED(8.68, 19);
  }

  /* We expect to see stop bit (low) */
  if (*pinInputRegister & pinBitMask) {
    return HITECD_ERR_CORRUPT;
  }

  return value;
}

void writeRegister(uint8_t reg, uint16_t val) {
  uint8_t old_sreg = SREG;
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

  SREG = old_sreg;
}

int readRegister(uint8_t reg, uint16_t *val_out) {
  Serial.println("Trying to read register");

  uint8_t old_sreg = SREG;
  cli();

  writeByte((uint8_t)0x96);
  writeByte((uint8_t)0x00);
  writeByte(reg);
  writeByte((uint8_t)0x00);
  uint8_t checksum = (0x00 + reg + 0x00) & 0xFF;
  writeByte(checksum);
  digitalWrite(PIN, LOW);

  SREG = old_sreg;

  delay(14);

  /* Note, most of the pull-up force is actually provided by the 2k resistor;
  the microcontroller pullup by itself is nowhere near strong enough. */
  pinMode(PIN, INPUT);
  if (digitalRead(PIN) != LOW) {
    delay(2);
    return HITECD_ERR_NO_SERVO;
  }

  old_sreg = SREG;
  cli();

  int const_0x69 = readByte();
  int mystery = readByte();
  int reg2 = readByte();
  int const_0x02 = readByte();
  int low = readByte();
  int high = readByte();
  int checksum2 = readByte();

  SREG = old_sreg;
  
  delay(1);

  if (digitalRead(PIN) != HIGH) {
    return HITECD_ERR_NO_RESISTOR;
  }

  digitalWrite(PIN, LOW);
  pinMode(PIN, OUTPUT);

  /* Error checking. Note, readByte() can return HITECD_ERR_NO_SERVO if it times
  out. But, we know the servo is present, or else we'd have hit HITECD_ERR_NO_SERVO
  or HITECD_ERR_NO_RESISTOR above. So this is unlikely to happen unless something's
  horribly wrong. So for simplicity, we just round this off to HITECD_ERR_CORRUPT. */
  if (const_0x69 != 0x69) return HITECD_ERR_CORRUPT;
  if (mystery < 0) return HITECD_ERR_CORRUPT;
  if (reg2 != reg) return HITECD_ERR_CORRUPT;
  if (const_0x02 != 0x02) return HITECD_ERR_CORRUPT;
  if (low < 0) return HITECD_ERR_CORRUPT;
  if (high < 0) return HITECD_ERR_CORRUPT;
  if (checksum2 != (mystery + reg2 + const_0x02 + low + high) & 0xFF) return HITECD_ERR_CORRUPT;

  *val_out = low + (high << 8);
  return 0;
}


void loop() {
  /* writeRegister(0x1E, 0x0190);
  delay(1000);
  writeRegister(0x1E, 0x15E0);
  readRegister(0x0C); */
  delay(1000);
  // uint16_t val;
  // readRegister(0x06, &val);
  digitalWrite(3, HIGH);
  readByte();
  digitalWrite(3, LOW);
}
