#define PIN 2

uint8_t pinBitMask;
volatile uint8_t *pinInputRegister, *pinOutputRegister;

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

  /* Write data bits. */
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

uint8_t readByte() {
  /* Wait for start bit */
  while (!(*pinInputRegister & pinBitMask)) { }

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

void readRegister(uint8_t reg) {
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
  pinMode(PIN, INPUT_PULLUP);

  old_sreg = SREG;
  cli();

  uint8_t reply[7];
  for (int i = 0; i < 7; ++i) {
    reply[i] = readByte();
  }

  SREG = old_sreg;
  
  delay(1);
  digitalWrite(PIN, LOW);
  pinMode(PIN, OUTPUT);

  for (int i = 0; i < 7; ++i) {
    Serial.print("Received: ");
    Serial.println(reply[i], HEX);
  }
  Serial.println("Done trying to read register");
}


void loop() {
  /* writeRegister(0x1E, 0x0190);
  delay(1000);
  writeRegister(0x1E, 0x15E0);
  readRegister(0x0C); */
  delay(1000);
  readRegister(0x06);
}
