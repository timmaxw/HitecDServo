#ifndef HitecDServo_h
#define HitecDServo_h

#include <Arduino.h>

#define HITECD_OK 0
#define HITECD_ERR_NO_SERVO -1
#define HITECD_ERR_NO_RESISTOR -2
#define HITECD_ERR_CORRUPT -3

class HitecDServo {
public:
  HitecDServo();

  void attach(int pin);
  bool attached();
  void detach();

  int readModelNumber();

private:
  void writeByte(uint8_t value);
  int readByte();

  void writeReg(uint8_t reg, uint16_t val);
  int readReg(uint8_t reg, uint16_t *val_out);

  int pin;
  uint8_t pinBitMask;
  volatile uint8_t *pinInputRegister, *pinOutputRegister;
};

#endif /* HitecDServo_h */
