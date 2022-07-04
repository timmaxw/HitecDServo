#ifndef Move_h
#define Move_h

#include <Arduino.h>

extern bool usingGentleMovementSettings;
void useGentleMovementSettings();
void undoGentleMovementSettings();
void moveGentlyToAPV(int16_t targetAPV, int16_t *actualAPV);

#endif /* Move_h */
