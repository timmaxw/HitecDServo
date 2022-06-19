# Main patterns
## Electrical protocol
The programmer and the servo communicate via a half-duplex serial connection at 115.2k baud, inverted polarity, 8 bits, no parity bit, 1 stop bit.
- When idle, the line is low (0V).
- When the programmer is transmitting, it drives the line low (0V) or high (about 2V).
- When the servo is transmitting, the programmer weakly pulls the line high (about 2V) and the servo drives it low (0V) to transmit. See "Read" below.

## Write
- Programmer sends bytes `0x96 0x00 reg 0x02 low high checksum`, where `checksum = (0x02+reg+0x02+low+high) & 0xFF`.
- No response from servo.
- The programmer typically waits 32ms before sending another command.

## Read
- Programmer sends `0x96 0x00 reg 0x00 checksum`, where `checksum = (0x00+reg+0x00) & 0xFF` (i.e. the same as `reg`)
- Then, both the servo and the programmer drive the line low. It stays this way for anywhere from about 1ms-15ms, apparently at random. Then, the programmer stops driving the line low and instead weakly pulls it high (~1.3mA drive strength).
- Exactly 15.2ms after the end of the original transmission, the servo responds `0x69 mystery reg 0x02 low high checksum`, where `checksum = (mystery+reg+0x02+low+high) & 0xFF`. The `mystery` byte can be either 0xFE or 0x00; I'm not sure why.
- After the servo completes its transmission, it stops driving the line low, allowing the programmer to pull it high. It stays this way until 16ms after the programmer originally began pulling the line high.
- Then the programmer starts driving the line low again, and the read is over.

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
- 0x0333 (= "20% 819") is lowest legal value. Note 0x0333 = decimal 819.
- 0x0FFF (= "99% 4095") is highest legal value (default). Note 0x0FFF = decimal 4095.
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
- About 1.6ms after the end of the write to 0x46 (2.2ms after the start of the write) the servo suddenly draws a lot of current for a few microseconds. The current drawn is more than when the servo moves normally, but more a much shorter period. This causes the supply voltage to dip. At the same time, the data line briefly glitches up to a few hundred millivolts above ground. Because the serial connection uses inverted polarity, a UART listening to the data line may interpret this glitch as an 0xFF byte.

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
- Writing `0x0BB8 + 4 * (microseconds - 1500)` to this register makes the servo move to the position specified by `microseconds`.
    - Note the resolution is 0.25 microsecond per unit of this register.
    - Center position is 0x0BB8, corresponding to 1500us
    - Min position is 0x0190, corresponding to 850us
    - Max position is 0x15E0, corresponding to 2150us
- These are the values displayed in red text in EPA settings mode.
- DPC-11 GUI "manual setting" mode has inconsistent behavior: When dragging the slider, it will use `0x0BC2 + 4 * (microseconds - 1500)` rather than `0x0BB8 + 4 * (microseconds - 1500)`. However, the buttons labeled "1500", "1000", "2000", etc. will use the 0x0BB8 formula.
- Approximately 1-3ms after end of the write to 0x1E, the servo begins to actually move. When the motor turns on, the supply voltage dips modestly, but not enough to cause a glitch on the data line.

## Register 0x0C: Read physical servo position
- Read-only register that appears to return actual physical servo position
- Theoretical range is 0x0000-0x3FFF, but the actual range is from approximately 0x02A0-0x3D5F (probably due to manufacturing variation in the specific servo)
- Depending on 0x5E (direction) register, meaning is inverted:
    - In CW mode, larger values go clockwise. So if servo is turned counterclockwise to the physical limit (approximately -101 degrees), 0x0C will read approximately 0x02A0; clockwise to the physical limit (approximately +101 degrees), 0x0C will read approximately 0x3D5F.
    - In CCW mode, larger values go counterclockwise. So if the servo is turned counterclockwise to the physical limit (approximately -101 degrees), 0x0C will read approximately 0x3D5F; clockwise to the physical limit (approximately +101 degrees), 0x0C will read approximately 0x02A0.

## Register 0xB2: EPA left point
- Describes physical servo position corresponding to a min-length pulse (850us)
- Follows same convention as 0x0C register.
- Default value is 0x0D35. This corresponds to approximately -65-75 degrees counterclockwise if 0x5E (direction) is in CW mode (or the reverse in CCW mode)
- When changing EPA points, value is temporarily set to 0x0032.

## Register 0xC2: EPA center point
- Describes physical servo position corresponding to a mid-length pulse (1500us)
- Follows same convention as 0x0C register.
- Default value is 0x2000.

## Register 0xB0: EPA right point
- Describes physical servo position corresponding to a max-length pulse (2150us)
    - https://www.servocity.com/d485hw-servo/ claims max-length pulse is 2350us; I believe this is an error.
- Follows same convention as 0x0C register.
- Default value is 0x32CA. This corresponds to approximately +65-75 degrees clockwise if 0x5E (direction) is in CW mode (or the reverse in CCW mode)
- When changing EPA points, value is temporarily set to 0x3FCD.

## Register 0x50: Unknown
- When starting EPA settings mode, write 0x3FFF

## Register 0x52: Unknown
- When starting EPA settings mode, write 0x0000

## Register 0x56: Unknown
- When starting EPA settings mode, write 0x0190
- On EPA reset, write 0x0FFF (immediately after writing 0x0FFF to 0x54)

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
2. Read 0x8A and 0x8C (if disabling) or 0xD4 and 0xD6 (if enabling)
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
4. Write 0xB2, 0xB0, 0xC2 by setting `new_center=0x3FFF-old_center`, `new_left=0x3FFF-old_right`, `new_right=0x3FFF-old_left`.
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
    - 0xC2 (EPA center) is set to same value read from 0x0C
    - 0xB2 (EPA left) is set to `EPA_center + round((left_value_from_0x0C - EPA_center) * (650/600))`. The `650/600` correction factor is because `left_value_from_0x0C` maps to a 900us pulse, but EPA left maps to a 850us pulse. 
    - 0xB0 (EPA right) is set to `EPA_center + round((right_value_from_0x0C - EPA_center) * (650/600))`. The `650/600` correction factor is because `right_value_from_0x0C` maps to a 2100us pulse, but EPA right maps to a 2150us pulse.
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
