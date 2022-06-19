#include <SoftwareSerial.h>

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

  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}

void writeByte(uint8_t value) {
  /* Write start bit. Note polarity is inverted, so start bit is HIGH. */
  *pinOutputRegister |= pinBitMask;

  /* We're operating at 115200 baud, so theoretically there should be an 8.68us
  interval between edges. In practice, I found through trial and error that
  7.15us produces better results, because some time is taken up by the non-delay
  instructions as well. */
  _delay_us(7.15);

  /* Write data bits. */
  for (int m = 0x001; m != 0x100; m <<= 1) {
    if (value & m) {
      *pinOutputRegister &= ~pinBitMask;
    } else {
      *pinOutputRegister |= pinBitMask;
    }
    _delay_us(7.15);
  }

  /* Write stop bit. */
  *pinOutputRegister &= ~pinBitMask;
  _delay_us(7.15);
}

uint8_t readByte() {
  /* Wait for start bit */
  while (!(*pinInputRegister & pinBitMask)) { }

  /* Delay until approximate center of first data bit. Again, this value was
  found through trial and error. */
  _delay_us(11);

  /* Read data bits */
  uint8_t value = 0;
  for (int m = 0x001; m != 0x100; m <<= 1) {
    if(!(*pinInputRegister & pinBitMask)) {
      value |= m;
    }
    _delay_us(7.5);
  }

  PORTD |= (1 << PORTD3);
  PORTD &= ~(1 << PORTD3);

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

  delay(7);

  /* Note, most of the pull-up force is actually provided by the 2k resistor;
  the microcontroller pullup by itself is nowhere near strong enough. */
  pinMode(PIN, INPUT_PULLUP);

  delay(7);

  old_sreg = SREG;
  cli();

  uint8_t reply[7];
  for (int i = 0; i < 7; ++i) {
    reply[i] = readByte();
  }

  SREG = old_sreg;
  
  delay(7);
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
