# DPC-11 notes

## Installing and running DPC-11 software
The DPC-11 software may require some extra steps to get working. The following steps worked for me on Windows 10 Home (version 10.0.19042 build 19042).

1. In the Windows control panel, find "Turn Windows features on or off". Enable the feature called ".NET Framework 3.5 (includes .NET 2.0 and 3.0)".

2. Disable Windows driver signature verification. There are several ways of doing this. One way is as follows:
    * In the Start menu, click Power, then hold Shift while clicking the "Restart" button.
    * A screen will appear saying "Choose an option". Click on "Troubleshoot", then "Advanced options", then "Startup settings", then "Restart".
    * Your computer will restart. You'll see a screen saying "startup settings". Press 7 to select "Disable driver signature enforcement".
    * Windows will boot normally, but driver signature enforcement will be disabled until the next time you reboot. Note that you need to re-disable Windows driver signature verification every time you reboot your PC.

3. Follow the instructions that came with the DPC-11 to install the DPC-11 control software:
    * Download the DPC-11 installation file from https://hitecrcd.com/products/servos/programmers/dpc-11/product
    * Extract the .ZIP file into a directory.
    * Run the DPC-11_Setup installer.
    * The installer should work normally. If you get a message about needing "dotnet framework 2.0.50727", double-check that you ran Step 1 above.

4. Connect the DPC-11 dongle to your PC.
    * You will see a popup saying something like "CP2102 USB to UART Bridge Controller has been configured". However, the DPC-11 won't actually work yet. Proceed to the next step.

5. Open the Windows Device Manager and look for the dongle in the list.
    * It will appear under "Other devices" as "CP2102 USB to UART Bridge Controller".
    * Double-click on it to open the Properties window. The "Device Status" will say "The drivers for this device are not installed. (Code 28)"
    * Click "Update Driver".
    * "Browse my computer for drivers". Browse to the folder where you unpacked the DPC-11 software .ZIP file. Select the folder "HITECRCD_DPC-11 Driver Installer".
    * You'll get a Windows Security popup saying that the driver may be unsafe. Click "Install this driver anyway".
    * In the Windows Device Manager list, the DPC-11 should now appear under "Universal Serial Bus controllers" as "HITECRCD_DPC-11". If you double-click on it, the "Device Status" should say "This device is working properly."

6. Now, you can open the DPC-11 application and use it normally.
    * If the DPC-11 is connected, the app startup screen should show a green box saying "DPC-11 is connected".
    * If you instead see a red box saying "DPC-11 is not connected", double-check that you disabled Windows driver signature verification by following Step 2 above, and that you haven't rebooted your PC since the last time you disabled Windows driver signature verification. (And of course, double-check that the DPC-11 is actually connected!)

## Details of DPC-11 behavior

This section documents the exact registers that the DPC-11 reads or writes when
each function in the GUI is performed. Refer to `HitecDServoInternal.h` for
register definitions.

### Initial connection
1. Write 0x22=0x1000
2. Read 0x06=0x4ABF, MODEL_NUMBER=485, again MODEL_NUMBER=485,
  0x04=36, DIRECTION=DIRECTION_CLOCKWISE, again DIRECTION=DIRECTION_CLOCKWISE,
  FAIL_SAFE=FAIL_SAFE_OFF, 0xC4=1300, again 0xC4=1300, ID=0,
  SENSITIVITY_RATIO=SENSITIVITY_RATIO_MAX, SPEED=0x0FFF, DEADBAND_1=1,
  SOFT_START=SOFT_SMART_20, SS_DISABLE_1=SS_DISABLE_1_CONST,
  SMART_SENSE_1=SS_ENABLE_1_CONST, OVERLOAD_PROTECTION=100

### OPEN button
The OPEN button writes many registers that are normally treated as read-only,
or that don't seem to play a role in any other context.

1. Read RANGE_LEFT_APV (before even selecting the file)
2. Write MYSTERY_DB (=0x4E54), ID (=0x0000), 0x34 (=0x0005), 0x38 (=0x0000),
  0x3A (=0x0041), FAIL_SAFE (=0x0001), DEADBAND_1 (=0x0001), 0x50 (=0x3FFF),
  0x52 (=0x0000), SPEED (=0x0FFF), POWER_LIMIT (=0x0FFF), 0x58 (=0x0064),
  0x5A (=0x0190), 0x5C (=0x0320), DIRECTION (=0x0000), SOFT_START (=0x0001),
  0x62 (=0x0000), 0x78 (=0x0000), 0x7A (=0x0320), 0x7C (=0x0190),
  0x7E (=0x00E8), 0x80 (=0x000A), 0x82 (=0x01F4), 0x84 (=0x0000),
  0x86 (=0x07D0), 0x88 (=0x001E), SS_DISABLE_2 (=0x0FA0),
  SS_DISABLE_1 (=0x6D60), 0x8E (=0x0000), 0x90 (=0x0000), 0x92 (=0x0019),
  0x94 (=0x3FFF), 0x96 (=0x0000), MYSTERY_OP1 (=0x00C8),
  MYSTERY_OP2 (=0x0003), OVERLOAD_PROTECTION (=0x0064),
  0x9E (=0x4E20), 0xA0 (=0x0064), 0xA2 (=0x0000), 0xA4 (=0x0005),
  0xA6 (=0x0040), 0xA8 (=0x0040), 0xAA (=0x0040), 0xAC (=0x06EA),
  0xAE (=0x0668), RANGE_RIGHT_APV (=0x32CA), RANGE_LEFT_APV (=0x0D35),
  RANGE_CENTER_APV (=0x2000), 0xC4 (=0x0514), 0xC0 (=0x0002), 0xB4 (=0x000E),
  0xB6 (=0x0010), 0xB8 (=0x0000), 0xBA (=0x0014), 0x3C (=0x0003),
  0x3E (=0x000A), 0x40 (=0x0096), 0x42 (=0x0005), SMART_SENSE_1 (=0x36B0),
  SENSITIVITY_RATIO (=0x0FFF), DEADBAND_2 (=0x0005), DEADBAND_3 (=0x000B),
  again DEADBAND_3 (=0x000B), 0x6A (=0x0002), SMART_SENSE_2 (=0x07D0),
  0xCE (=0x000A), 0xD0 (=0x0014), 0xD2 (=0x0032), SS_ENABLE_2 (=0x07D0),
  SS_ENABLE_1 (=0x36B0)
3. Write SAVE=SAVE_CONST
4. Read same registers as on initial connection (see above)
5. Write REBOOT=REBOOT_CONST, then delay 1000ms
6. Write TARGET=3000 (equivalent to 1500us PWM pulse)

(All delays between reads/writes are around 20-50ms unless otherwise noted.)

### SAVE button
The SAVE button reads many registers that don't seem to play a role in any other
context. It also reads the SAVE, FACTORY_RESET, and REBOOT registers, which is
odd (all three return 0x0000).

1. Read RANGE_LEFT_APV
2. Read 0x78 (=0x0000), 0x7A (=0x0320), 0x7C (=0x0190), 0x7E (=0x00E8),
  0x80 (=0x000A), 0x82 (=0x01F4), 0x84 (=0x0000), 0x86 (=0x07D0),
  0x88 (=0x001E), SS_DISABLE_2 (=0x0FA0), SS_DISABLE_1 (=0x6D60),
  0x8E (=0x0000), 0x90 (=0x0000), 0x30 (=0x0000), MODEL_NUMBER (=0x01E5),
  0x02 (=0x0004), 0x04 (=0x0024), 0x06 (=0x4ABF), 0x08 (=0x001D), 
  0xCC (=0x0096), 0xCE (=0x000A), 0x0A (=0x0000), CURRENT_APV (=0x1FDB),
  0x0E (=0x0000), 0x10 (=0x0000), 0x12 (=0x0000), 0x14 (=0x0000), ID (=0x0000),
  0x34 (=0x0005), 0x36 (=0x0002), 0x38 (=0x0000), 0x3A (=0x0041),
  REBOOT (=0x0000), 0x48 (=0x0000), 0x4A (=0x0000), FAIL_SAFE (=0x0001),
  DEADBAND_1 (=0x0001), 0x50 (=0x3FFF), 0x52 (=0x0000), SPEED (=0x0FFF),
  POWER_LIMIT (=0x0FFF), 0x58 (=0x0064), 0x5A (=0x0190), 0x5C (=0x0320),
  DIRECTION (=0x0000)
3. Write DIRECTION=0
4. Write SAVE=SAVE_CONST
5. Write REBOOT=REBOOT_CONST (followed by 1 second delay)
6. Write 0x22=0x1000
7. Read SOFT_START (=0x0001), SENSITIVITY_RATIO (=0x0FFF), 0x62 (=0x0000),
  FACTORY_RESET (=0x0000), SAVE (=0x0000), MYSTERY_DB (=0x0000),
  0x92 (=0x0019), 0x94 (=0x3FFF), 0x96 (=0x0000),
  MYSTERY_OP1 (=0x00C8), MYSTERY_OP2 (=0x0003),
  OVERLOAD_PROTECTION (=0x0064), 0x9E (=0x4E20), 0xA0 (=0x0064),
  0xA2 (=0x0000), 0xA4 (=0x0005), 0xA6 (=0x0040), 0xA8 (=0x0040),
  0xAA (=0x0040), 0xAC (=0x06EA), 0xAE (=0x0668), RANGE_RIGHT_APV (=0x32CA),
  RANGE_LEFT_APV (=0x0D35), RANGE_CENTER_APV (=0x2000), 0xC4 (=0x0514),
  0xC0 (=0x0002), 0xB4 (=0x000E), 0xB6 (=0x0010), 0xB8 (=0x0000),
  0xBA (=0x00D0), 0xBC (=0x0000), DEADBAND_2 (=0x0005), DEADBAND_3 (=0x0075),
  0x6A (=0x006E), SMART_SENSE_2 (=0x07D0), 0x3C (=0x0003), 0x3E (=0x000A),
  0x40 (=0x0096), 0x42 (=0x0005), SMART_SENSE_1 (=0x36B0), 0xCE (=0x000A),
  0xD0 (=0x0014), 0xD2 (=0x0032), SS_ENABLE_2 (=0x07D0),
  SS_ENABLE_1 (=0x36B0)

(All delays between reads/writes are around 20-50ms unless noted.)

### Refresh button
1. Read RANGE_LEFT_APV
2. Read same registers as on initial connection (see above)

(All delays between reads are around 20-50ms.)

### Program Reset button
1. Read RANGE_LEFT_APV
2. Write FACTORY_RESET=FACTORY_RESET_CONST
3. Write MYSTERY_OP1=MYSTERY_OP1_CONST and MYSTERY_OP2=MYSTERY_OP2_CONST
4. Write SAVE=SAVE_CONST
5. Write REBOOT=REBOOT_CONST (followed by 1 second delay)
6. Write 0x22=0x1000
7. Read same registers as on initial connection (see above)

(All delays between reads/writes are around 20-50ms unless noted.)

### Enable/disable FS_limp
1. Write FAIL_SAFE
2. Write SAVE=SAVE_CONST
3. Write REBOOT=REBOOT_CONST (Note enable/disable Fail_Safe does not do this)

### Change overload protection
1. Write OVERLOAD_PROTECTION to old value
2. Write MYSTERY_OP1=MYSTERY_OP1_CONST
3. Write MYSTERY_OP2=MYSTERY_OP2_CONST
4. Write SAVE=SAVE_CONST
5. Write OVERLOAD_PROTECTION to new value
6. Write MYSTERY_OP1=MYSTERY_OP1_CONST
7. Write MYSTERY_OP2=MYSTERY_OP2_CONST
8. Write SAVE=SAVE_CONST

### Change soft start
1. Write SOFT_START to old value
2. Write SAVE=SAVE_CONST
3. Write SOFT_START to new value
4. Write SAVE=SAVE_CONST

### Enable/disable Smart Sense
1. Write MYSTERY_DB=MYSTERY_DB_CONST
2. If disabling, read SS_DISABLE_2 and SS_DISABLE_1. If enabling, read
  SS_ENABLE_2 and SS_ENABLE_1.
3. Write SMART_SENSE_2 and SMART_SENSE_1 with values that were read
4. Write SAVE=SAVE_CONST
5. Read back SMART_SENSE_1 and SMART_SENSE_2
6. Write REBOOT=REBOOT_CONST

### Enable/disable Fail_Safe
1. Write FAIL_SAFE
2. Write SAVE

### Change CW/CCW
1. Write FAIL_SAFE
2. Read RANGE_RIGHT_APV, RANGE_LEFT_APV, RANGE_CENTER_APV
3. Write DIRECTION to the opposite of before
4. Write:
   * RANGE_LEFT_APV = HITECD_APV_MAX - old_right_APV
   * RANGE_RIGHT_APV = HITECD_APV_MAX - old_left_APV
   * RANGE_CENTER_APV = HITECD_APV_MAX - old_center_APV
5. Write SAVE=SAVE_CONST
6. Write REBOOT=REBOOT_CONST
7. Write 0x22=0x1000

### Change ID number
1. Write ID with new value
2. Write SAVE=SAVE_CONST
3. Write REBOOT=REBOOT_CONST, wait 1 second
4. Write 0x22=0x1000

### Change speed
1. Write SPEED with new value
2. Write SAVE=SAVE_CONST

### Change EPA
1. Read RANGE_LEFT_APV, RANGE_CENTER_APV, RANGE_RIGHT_APV
2. Write RANGE_RIGHT_APV, RANGE_LEFT_APV, RANGE_CENTER_APV to extreme values:
    * RANGE_LEFT_APV=0x0032 (this is 50)
    * RANGE_CENTER_APV=0x2000 (this is midway between 0 and HITECD_APV_MAX)
    * RANGE_RIGHT_APV=0x3FCD (this is HITECD_APV_MAX - 50)
3. Write SPEED=5 (25% of max speed)
4. Write 0x50=0x3FFF and 0x52=0x0000
5. Write SAVE=SAVE_CONST
6. Write REBOOT=REBOOT_CONST, wait 1000ms
7. Write POWER_LIMIT=400 (20% of max power)
8. Write TARGET
9. When slider is dragged, write TARGET with number in red.
    * Note that min=400=0x0190 and max=5600=0x15E0
10. When Left/Right/Center buttons are pressed, read CURRENT_APV and remember
  results. (We'll call these values left_APV, right_APV, and center_APV.)
11. When OK button is pressed:
    * Write RANGE_CENTER_APV = center_APV
    * Write RANGE_LEFT_APV = center_APV+round((left_APV-center_APV)*(650/600)).
      The 650/600 correction factor is because left_APV maps to a 900us pulse, but
      RANGE_LEFT_APV maps to a 850us pulse. 
    * Write RANGE_RIGHT_APV = center_APV+round((right_APV-center_APV)*(650/600)).
      The 650/600 correction factor is because right_APV maps to a 2100us pulse,
      but RANGE_RIGHT_APV maps to a 2150us pulse.
12. Write SPEED back to original value
13. Write TARGET, delay 1000ms
14. Write POWER_LIMIT=0x0FFF
15. Write SAVE=SAVE_CONST
16. Write REBOOT=REBOOT_CONST, delay 1000ms
17. Write SPEED back to original value (again)
18. Write 0x22=0x1000
19. Write TARGET, delay 1000ms
20. Write SPEED back to original value (a third time)
21. Write POWER_LIMIT=0x0FFF

### EPA_Reset
Starting after step 8 of the "Change EPA" procedure above:

9. Write TARGET
10. Write FACTORY_RESET=FACTORY_RESET_CONST
11. Write ID to original value
12. Read RANGE_LEFT_APV, RANGE_CENTER_APV, RANGE_RIGHT_APV
13. Write REBOOT=REBOOT_CONST, delay 1000ms
14. Write RANGE_LEFT_APV, RANGE_CENTER_APV, SPEED, POWER_LIMIT, and
  RANGE_RIGHT_APV with various values
15. Write SAVE=SAVE_CONST
16. Write REBOOT=REBOOT_CONST, delay 1000ms
17. Write 0x22=0x1000
18. Write TARGET

### Cancel EPA change
Starting after step 8 of the "Change EPA" procedure above:

9. Write RANGE_LEFT_APV, RANGE_CENTER_APV, RANGE_RIGHT_APV
10. Write TARGET
11. Write POWER_LIMIT=0x0FFF
12. Write SAVE=SAVE_CONST, then SAVE=SAVE_CONST again
13. Write REBOOT=REBOOT_CONST, delay 1 second
14. Write 0x22=0x1000
15. Write TARGET
16. Write SPEED to original value
17. Write SAVE=SAVE_CONST

### Change Dead Band
1. Write MYSTERY_DB=MYSTERY_DB_CONST
2. Write DEADBAND_1, DEADBAND_2, DEADBAND_3
3. Write SAVE=SAVE_CONST, then wait about 1500ms
4. Write REBOOT=REBOOT_CONST

(All delays between writes are 20-50ms unless otherwise noted.)

### Change Sensitivity Ratio
(Note, presets may trigger deadband changes; this assumes no deadband changes.)
1. Write SENSITIVITY_RATIO
2. Write SAVE=SAVE_CONST
3. Write REBOOT=REBOOT_CONST
