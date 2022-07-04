#ifndef Move_h
#define Move_h

#include <Arduino.h>

/* For supported models, the library knows these values. But for unsupported
models, we can only discover them experimentally. These variables will be set
to -1 if not known, or to a specific value if known either via the library or
via experiment. */
extern int16_t defaultRangeLeftAPV;
extern int16_t defaultRangeRightAPV;
extern int16_t defaultRangeCenterAPV;

/* More values that can be known either statically or experimentally. Note, the
variables always follow a clockwise convention. The functions dynamically
convert based on the value of settings.counterclockwise. */
extern int16_t widestRangeLeftAPVClockwise;
extern int16_t widestRangeRightAPVClockwise;
extern int16_t widestRangeCenterAPVClockwise;
int16_t widestRangeLeftAPV();
int16_t widestRangeRightAPV();
int16_t widestRangeCenterAPV();

void askAndMoveToMicros();
void moveToQuarterMicros(int16_t quarterMicros);

extern bool usingGentleMovementSettings;
void useGentleMovementSettings();
void undoGentleMovementSettings();
void moveGentlyToAPV(int16_t targetAPV, int16_t *actualAPV);

#endif /* Move_h */
