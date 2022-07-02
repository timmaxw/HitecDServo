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
void moveTempRawAngle(int16_t targetRawAngle, int16_t *actualRawAngle);

#endif /* Common_h */
