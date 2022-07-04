#ifndef Settings_h
#define Settings_h

#include <Arduino.h>

void printValueWithDefault(int16_t value, int16_t defaultValue);

void printAllSettings();

void saveSettings();

void printIdSetting();
void changeIdSetting();

void printDirectionSetting();
void changeDirectionSetting();

void printSpeedSetting();
void changeSpeedSetting();

void printDeadbandSetting();
void changeDeadbandSetting();

void printSoftStartSetting();
void changeSoftStartSetting();

void printFailSafeSetting();
void changeFailSafeSetting();

void printPowerLimitSetting();
void changePowerLimitSetting();

void printOverloadProtectionSetting();
void changeOverloadProtectionSetting();

void printSmartSenseSetting();
void changeSmartSenseSetting();

void printSensitivityRatioSetting();
void changeSensitivityRatioSetting();

void resetSettingsToFactoryDefaults();

#endif /* Settings_h */
