#ifndef Move_h
#define Move_h

#include <Arduino.h>

void askAndMoveToMicros();
void moveToQuarterMicros(int16_t quarterMicros);

extern bool usingGentleMovementSettings;
void useGentleMovementSettings();
void undoGentleMovementSettings();
void moveGentlyToAPV(int16_t targetAPV, int16_t *actualAPV);

#endif /* Move_h */
