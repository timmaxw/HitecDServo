# Main patterns
## Write
- Programmer sends bytes `0x96 0x00 reg 0x02 low high checksum`, where `checksum = (0x02+reg+0x02+low+high) & 0xFF`.
- No response from servo.
## Read
- Programmer sends `0x96 0x00 reg 0x00 checksum`, where `checksum = (0x00+reg+0x00) & 0xFF` (i.e. the same as `reg`)
- Servo responds `0x69 0xFE reg 0x02 low high checksum`, where `checksum = (0xFE+reg+0x02+low+high) & 0xFF`.

# Registers
## Register 0x22: Unknown
- Written to 0x1000 at startup, and after many other operations. (Note about convention: 0x1000 means `low=0x00` and `high=0x10`.)

## Register 0x06: Unknown
- Read at startup, returns 0x4ABF.

## Register 0x00: Model number
- Read (twice) at startup, returns 0x01E5. In decimal this is 485, and the servo is model D485.

## Register 0x04: Unknown
- Read at startup, returns 0x0024.

## Register 0x5E: Direction
- 0x0000 means clockwise
- 0x0001 means counterclockwise
- Read (twice) at startup and program reset

## Register 0x4C: Failsafe behavior
- Read at startup
- 0x0000 means failsafe by going limp
- 0x0001 means no failsafe (default)
- Other values are failsafe pulse width in microseconds

## Register 0xC4: Unknown
- Read (twice) at startup, returns 0x0514

## Register 0x32: Servo ID number
- Read at startup
- Low byte is servo ID number (0..254)
- High byte is always zero
- Default servo ID is 0

## Register 0x64: Sensitivity Ratio
- 0x0333 (= "20% 819") is lowest legal value
- 0x0FFF (= "99% 4095") is highest legal value (default)
- Read at startup (twice)

## Register 0x54: Speed
- 0x0002 means 10% speed
- 0x0004 means 20% speed
- etc.
- 0x0012 means 90% speed
- 0x0FFF means 100% speed (default)
- Read at startup, then written back
- Written to 0x0005 when starting EPA settings mode

## Register 0x70: Unknown
- Written to 0xFFFF at startup, and after many other operations.

## Register 0x4E: Deadband-related (one of three)
- Read at startup
- 0x0001 when deadband=1 (default)
- 0x0004 when deadband=2, 0x0008 when deadband=3, ...
  - Equivalently, 0x0004*deadband-0x0004 when deadband=2..10

## Register 0x66: Deadband-related (one of three)
- Not read at startup
- 0x0005 when deadband=1
- 0x0008 when deadband=2, 0x000C when deadband=3, ...
  - Equivalently, 0x0004*deadband when deadband=2..10

## Register 0x68: Deadband-related (one of three)
- Not read at startup
- 0x000B when deadband=1
- 0x000E when deadband=2, 0x0012 when deadband=3, ...
  - Equivalently, 0x0004*deadband+0x0006 when deadband=2...10

## Register 0x60: Soft start
- Read at startup
- 0x0001 means 20% soft start (default)
- 0x0003 means 40% soft start
- 0x0006 means 60% soft start
- 0x0008 means 80% soft start
- 0x0064 means 100% soft start

## Register 0x72: Unknown
- Written to 0x4E54 at startup and before changing deadband
- Written to 0x4E54 when disabling Smart Sense
- Written to 0x4E54 when enabling Smart Sense

## Register 0x8A: Smart Sense read
- Read 0x0FA0 when disable Smart Sense

## Register 0x8C: Smart Sense read
- Read at startup, returns 0x6D60
- Read when disabling Smart Sense, returns 0x6D60

## Register 0xD4: Smart Sense read
- Read at startup, returns 0x07D0
- Read when enabling Smart Sense, returns 0x07D0

## Register 0xD6: Smart Sense read
- Read at startup, returns 0x36B0
- Read when enabling Smart Sense, returns 0x36B0

## Register 0x6C: Smart Sense control
- Written to 0x07D0 at startup, shortly after reading register 0xD4; then read back
- Written to 0x0FA0 when disabling Smart Sense; then read back
- Written to 0x07D0 when enabling Smart Sense; then read back

## Register 0x44: Smart Sense control
- Written to 0x36B0 at startup, shortly after reading register 0xD6; then read back
- Written to 0x6D60 when disabling Smart Sense; then read back
- Written to 0x36B0 when enabling Smart Sense; then read back

## Register 0x46: Unknown
- Written to 0x0001 at startup and in several other situations
- Anomalous electrical behavior afterwards (need to investigate)

## Register 0x9C: Overload protection
- 0x000A means 10% overload protection
- 0x0014 means 20% overload protection
- 0x001E means 30% overload protection
- 0x0028 means 40% overload protection
- 0x0032 means 50% overload protection
- 0x0064 means overload protection off (default)

## Register 0x98: Related to overload protection
- Written to 0x00C8 every time overload protection is changed
- Never read; never written with any other value

## Register 0x9A: Related to overload protection
- Written to 0x0003 every time overload protection is changed
- Never read; never written with any other value

## Register 0x1E: Travel
- Writing `0x0BC2 + 4 * (microseconds - 1500)` to this register makes the servo move to the position specified by `microseconds`.
- So e.g. writing 0x0BC2 is makes the servo travel to the center position (equivalent to 1500 microseconds).
- Note the resolution is 0.25 microsecond per unit of this register.
- In some contexts, 0x0BB8 is treated as the center point rather than 0xBC2.
- Min point is 0x0190
- Max point is 0x15E0
- These are the same values displayed in red text in EPA mode.

## Register 0xB2: EPA left?
- Read on program reset, returns 0x0D35
- When starting EPA settings mode, read 0x0D35 then write 0x0032
- On EPA reset, read 0x0D35 then write 0x0D35
- On EPA set, write to 0x006C (round 2: 0x1969)
- On CW->CCW, read 0x1969 write 0x1951. On CCW->CW, the reverse.

## Register 0xC2: EPA center
- When starting EPA settings mode, read 0x2000 then write 0x2000
- On EPA reset, read 0x2000 then write 0x2000
- On EPA set, write to 0x200B (value read from 0x0C) (round 2: 0x2008)
- On CW->CCW, read 0x2008 write 0x1FF7. On CCW->CW, the reverse.

## Register 0xB0: EPA right?
- When starting EPA settings mode, read 0x32CA then write 0x3FCD
- On EPA reset, read 0x32CA then write 0x32CA
- On EPA set, write to 0x3FB0 (round 2: 0x26AE)
- On CW->CCW, read 0x26AE write 0x2696. On CCW->CW, the reverse.

## Register 0x50: Unknown
- When starting EPA settings mode, write 0x3FFF

## Register 0x52: Unknown
- When starting EPA settings mode, write 0x0000

## Register 0x56: Unknown
- When starting EPA settings mode, write 0x0190
- On EPA reset, write 0x0FFF (immediately after writing 0x0FFF to 0x54)

## Register 0x0C: Read current physical position?
- EPA "Center" button, read 0x200B (round 2: 0x2008)
- EPA "Left" button, read 0x02DB (round 2: 0x19EB)
- EPA "Right" button, read 0x3D41 (round 2: 0x262B)

## Register 0x6E: Reset to factory settings
- Written to 0x0F0F on program reset
- Written to 0x0F0F on EPA reset

# Procedures
## OPEN
1. Write 0x72, 0x32, 0x34, 0x38, 0x3A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E, 0x60, 0x62, 0x78, 0x7A, 0x7C, 0x7E, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E, 0xA0, 0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xC2, 0xC4, 0xC0, 0xB4, 0xB6, 0xB8, 0xBA, 0x3C, 0x3E, 0x40, 0x42, 0x44, 0x64, 0x66, 0x68 (twice; same value), 0x6A, 0x6C, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6
2. Write 0x70
3. Read 0x06, 0x00 (twice; same value), 0x04, 0x5E (twice; same value), 0x4C, 0xC4 (twice; same value), 0x42, 0x64, 0x54, 0x4E, 0x60, 0x8C, 0x44, 0x9C
4. Write 0x46
5. Write 0x1E

## SAVE
1. Read 0xB2
2. Read 0x78, 0x7A, 0x7C, 0x7E, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0x90, 0x30, 0x00, 0x02, 0x04, 0x06, 0x08, 0xCC, 0xCE, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x46, 0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E
3. Write 0x5E
4. Write 0x70
5. Write 0x46
6. Write 0x22
7. Read 0x60, 0x64, 0x62, 0x6E, 0x70 (= 0x0000), 0x72, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E, 0xA0, 0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xC2, 0xC4, 0xC0, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0x66, 0x68, 0x6A, 0x6C, 0x3C, 0x3E, 0x40, 0x42, 0x44, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6

## Refresh
1. Read 0xB2
2. Read 0x06, 0x00 (twice; same value), 0x04, 0x5E (twice; same value), 0x4C, 0xC4 (twice; same value), 0x42, 0x64, 0x54, 0x4E, 0x60, 0x8C, 0x44, 0x9C

## Program Reset
1. Read 0xB2
2. Write 0x6E
3. Write 0x98, 0x9A
4. Write 0x70
5. Write 0x46 (followed by 1 second delay)
6. Write 0x22
7. Read 0x06, 0x00 (twice; same value), 0x04, 0x5E (twice; same value), 0x4C, 0xC4 (twice; same value), 0x42, 0x64, 0x54, 0x4E, 0x60, 0x8C, 0x44, 0x9C

## Enable/disable FS_limp
1. Write 0x4C (fail safe)
2. Write 0x70
3. Write 0x46 (weird! note enable/disable Fail_Safe does not do this)

## Change overload protection
1. Write 0x9C (overload protection) to old value
2. Write 0x98 (overload protection constant???)
3. Write 0x9A (overload protection constant???)
4. Write 0x70
5. Write 0x9C to new value
6. Write 0x98 (overload protection constant???)
7. Write 0x9A (overload protection constant???)
8. Write 0x70

## Change soft start
1. Write 0x60 to old value
2. Write 0x70
3. Write 0x60 to new value
4. Write 0x70

## Enable/disable Smart Sense
1. Write 0x72
2. Read 0x8A and 0x8C (disabling) or 0xD4 and 0xD6 (enabling)
3. Write 0x6C and 0x44 with values that were read
4. Write 0x70
5. Read back 0x44 and 0x6C
6. Write 0x46

## Enable/disable Fail_Safe
1. Write 0x4C (fail safe)
2. Write 0x70

## Change CW/CCW
1. Write 0x4C
2. Read 0xB0, 0xB2, 0xC2 (EPA points)
3. Write 0x5E (direction flag)
4. Write 0xB2, 0xB0, 0xC2 with slightly modified values
5. Write 0x70
6. Write 0x46
7. Write 0x22

## Change ID number
1. Write 0x32 (ID number)
2. Write 0x70
3. Write 0x46, wait 1 second
4. Write 0x22

## Change speed
1. Write 0x54
2. Write 0x70

## Change EPA
1. Read 0xB2, 0xC2, 0xB0 (EPA points)
2. Write 0xB0, 0xB2, 0xC2 to extreme/default values: 0xB2=0x0032, 0xC2=0x2000, 0xB0=0x3FCD.
  - Note on a range from 0x0000 to 0x3FFF, these values are the center and 0x32 away from the extremes.
3. Write 0x54 (speed) with 0x0005 (slow-ish? not normally valid)
4. Write 0x50, 0x52 with 0x3FFF, 0x0000
5. Write 0x70
6. Write 0x46, wait 1 second
7. Write 0x56 with 0x0190 (decimal 400...?)
8. Write 0x1E
9. When slider is dragged, write 0x1E with number in red.
  - Note that min=400=0x0190 and max=5600=0x15E0
10. When Left/Right/Center buttons are pressed, read 0x0C.
  - Value appears to be on the 0x0000-0x3FFF range.
11. When OK button is pressed, write 0xC2, 0xB0, 0xB2
12. Write 0x54 (speed) with original value
13. Write 0x1E, delay 1 second
14. Write 0x56 with 0x0FFF
15. Write 0x70
16. Write 0x46, delay 1 second
17. Write 0x54 (speed) with original value
18. Write 0x22
19. Write 0x1E, delay 1 second
20. Write 0x54 (speed) with original value
21. Write 0x56 with 0x0FFF

## EPA_Reset
Starting after step 8 above:
9. Write 0x1E
10. Write 0x6E (reset all settings)
11. Write 0x32 (ID number)
12. Read 0xB2, 0xC2, 0xB0 (EPA points)
13. Write 0x46, delay 1 second
14. Write 0xB2, 0xC2, 0x54, 0x56, 0xB0
15. Write 0x70
16. Write 0x46, delay 1 second
17. Write 0x22
18. Write 0x1E

## Cancel EPA change
Starting after step 8 above:
9. Write 0xB2, 0xC2, 0xB0 (EPA points)
10. Write 0x1E
11. Write 0x56 with 0x0FFF
12. Write 0x70 (twice)
13. Write 0x46, delay 1 second
14. Write 0x22
15. Write 0x1E
16. Write 0x54 (speed) with original value
17. Write 0x70

## Change Dead Band
1. Write 0x72 (always to 0x4E54)
2. Write 0x4E, 0x66, 0x68 to new deadband values
3. Write 0x70, then wait 1 second
4. Write 0x46

## Change Sensitivity Ratio
(Note, presets may trigger deadband changes; this assumes no deadband changes.)
1. Write 0x64 (sensitivity ratio)
2. Write 0x70
3. Write 0x46
