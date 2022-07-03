#ifndef Common_h
#define Common_h

#include <HitecDServo.h>

extern HitecDServo servo;
extern int modelNumber;
extern HitecDSettings settings;

/* For supported models, the library knows these values. But for unsupported
models, we can only discover them experimentally. These variables will be set
to -1 if not known, or to a specific value if known either via the library or
via experiment.
Note: widestRangeLeftAPV/etc. always follow a clockwise convention. So if
the servo is in counterclockwise mode, we have to invert them. */
extern int16_t defaultRangeLeftAPV;
extern int16_t defaultRangeRightAPV;
extern int16_t defaultRangeCenterAPV;
extern int16_t widestRangeLeftAPV;
extern int16_t widestRangeRightAPV;
extern int16_t widestRangeCenterAPV;

void printErr(int res, bool needReset);
void printValueWithDefault(int16_t value, int16_t defaultValue);
void writeSettings();

void useRangeMeasurementSettings();
void undoRangeMeasurementSettings();
void temporarilyMoveToAPV(int16_t targetAPV, int16_t *actualAPV);

#endif /* Common_h */
