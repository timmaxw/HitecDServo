#ifndef Programmer_h
#define Programmer_h

#include <HitecDServo.h>

extern HitecDServo servo;
extern int modelNumber;
extern HitecDSettings settings;

void printErr(int res, bool fatal);
void fatalErr();
void saveSettings();

#endif /* Programmer_h */
