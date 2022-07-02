#ifndef Common_h
#define Common_h

#include <HitecDServo.h>

extern HitecDServo servo;
extern int modelNumber;
extern HitecDSettings settings;
extern bool allowUnsupportedModel;

void printErr(int res, bool needReset);
void printValueWithDefault(int16_t value, int16_t defaultValue);
bool checkSupportedModel();
void writeSettings();

void useRangeMeasurementSettings();
void undoRangeMeasurementSettings();
void temporarilyMoveToAPV(int16_t targetAPV, int16_t *actualAPV);

#endif /* Common_h */
