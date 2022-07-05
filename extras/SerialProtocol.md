# Serial protocol
This document contains "lab notes" from reverse-engineering the serial protocol between the Hitec DPC-11 programmer and the servo.

## Electrical protocol
The programmer and the servo communicate via a half-duplex serial connection at 115200 baud, inverted polarity, 8 bits, no parity bit, 1 stop bit.
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
- Note, all valid registers are even-numbered. Attempting to read an odd-numbered register gives 0xLLHH, where 0xHH is the high byte of the previous register, and 0xLL is the low byte of the next register. (Attempting to read 0xFF returns 0x0000, even though 0x00 has a non-zero low byte.)

## PWM behavior
Of course, the servo also accepts PWM signals as normal.

Pulses shorter than 850us or longer than 2350us are ignored. It's odd that the upper limit is 2350us; since the lower limit is 850us, and the center is 1500us, one would expect the upper limit to be 2150us. Also, the servo's "EPA right point" is the point where it will go if it receives a 2150us pulse. Nevertheless, the servo will accept pulses up to 2350us, and will move past the "EPA right point".

If the servo receives a serial command, it will disregard any subsequent PWM pulses it receives, until it's reset by cycling the power or writing register 0x46 (see below). However, the reverse is not true; if the servo receives PWM pulses, followed by a serial command, it will respect the serial command as normal.

# Registers
## Register 0x06: Unknown
- Read at startup, returns 0x4ABF. (Note about convention: 0x4ABF means low=0xBF and high=0x4A.)
- I speculate this might be a manufacturing date code? 0x4ABF in decimal is 19135, which could refer to "the 135th day of 2019".

## Register 0x00: Model number
- Read (twice) at startup, returns 0x01E5. In decimal this is 485, and the servo is model D485.

## Register 0x04: Unknown
- Read at startup, returns 0x0024.
- I speculate this might be a firmware version number?

## Register 0x5E: Direction
- 0x0000 means clockwise (default)
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

## Register 0x70: Save settings to EEPROM
- When 0x70 is written with 0xFFFF, the servo will write its settings from SRAM to EEPROM.
- If settings have not been saved to EEPROM, they will be lost when the servo loses power or is reset (see register 0x46)

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
- Written to 0x0FA0 when disabling Smart Sense; then read back
- Written to 0x07D0 when enabling Smart Sense; then read back

## Register 0x44: Smart Sense control
- Read at startup/refresh
- Written to 0x6D60 when disabling Smart Sense; then read back
- Written to 0x36B0 when enabling Smart Sense; then read back

## Register 0x46: Reset servo
- Resets the servo's microcontroller. Any settings not flushed to EEPROM (see register 0x70) will be lost.
- Written to 0x0001 after many (but not all) changes to settings.
- For a period of about 1s after the write to 0x46, or after power-on, the servo will ignore all commands (either PWM or serial).
- About 1.6ms after the end of the write to 0x46 (2.2ms after the start of the write) the servo suddenly draws a lot of current for a few microseconds. The current drawn is more than when the servo moves normally, but more a much shorter period. This causes the supply voltage to dip. At the same time, the data line briefly glitches up to a few hundred millivolts above ground. Because the serial connection uses inverted polarity, a UART listening to the data line may interpret this glitch as an 0xFF byte.

## Register 0x9C: Overload protection
- If the servo is overloaded for more than 3 seconds, overload protection reduces motor power to a percentage of the max power.
- 0x000A means reduce power to 10% of max
- 0x0014 means reduce power to 20% of max
- 0x001E means reduce power to 30% of max
- 0x0028 means reduce power to 40% of max
- 0x0032 means reduce power to 50% of max
- 0x0064 means do not reduce power; overload protection disabled (default)

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
- Returns different values when read back (returned values seem similar to 0x0C)

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
- Describes physical servo position corresponding to a "max-length" pulse (2150us)
    - Note 2150us isn't the _actual_ max pulse length; the servo will actually recognize pulses up to 2350us, moving past the EPA right point.
- Follows same convention as 0x0C register.
- Default value is 0x32CA. This corresponds to approximately +65-75 degrees clockwise if 0x5E (direction) is in CW mode (or the reverse in CCW mode)
- When changing EPA points, value is temporarily set to 0x3FCD.

## Register 0x50: Unknown
- When starting EPA settings mode, write 0x3FFF

## Register 0x52: Unknown
- When starting EPA settings mode, write 0x0000

## Register 0x56: Power limit setting
- Defines the maximum amount of power the servo will use. Ranges from 0x0000 to 0x07D0. Any value larger than 0x07D0 will be treated as equivalent to 0x07D0.
- When starting EPA settings mode, write 0x0190 (20% power)
- On EPA reset, write 0x0FFF (max power)

## Register 0x22: Actual power limit (after overload protection)
- Returns the actual power limit the servo is using. Ranges from 0x0000 to 0x07D0.
- In normal operation, returns the same value as 0x56 (but capped to 0x07D0).
- If overload protection kicks in, returns the effective power limit after overload protection. (E.g. if 0x56 is set to 0x0400 and overload protection is set to 50%, then when overload protection kicks in, 0x22 will read 0x0200.)
- Written 0x1000 after any operation that changes EPA points or servo direction. However, writing to 0x22 doesn't seem to have any effect.
- Also written 0x1000 on initial connection, and after changing servo ID. (Not sure why!)

## Register 0x6E: Reset to factory settings
- Written to 0x0F0F on program reset
- Written to 0x0F0F on EPA reset

## Register 0xE4: Target point?
- When register 0x1E is written, 0xE4 appears to be updated with the target point as measured in the same units as the 0x0C register.

## Register 0xDC: Unknown
- Typically returns approximately the same value as 0x0C.

## Register 0xE0: Unknown
- Typically returns approximately the same value as 0x0C.

## Register 0x0E: Unknown
- Appears to be some kind of time-derivative of 0x0C.
- Signed integer.
- Typically 0, 1, or -1 when servo is stationary; larger values when moving.

## Register 0xDE: Unknown
- Appears to be some kind of time-derivative of 0x0C.
- Signed integer.
- Typically 0, 1, or -1 when servo is stationary; larger values when moving.

## Register 0xEA: Unknown
- Difference between register 0x0C and register 0xE4.
- Signed integer.

## Register 0xEC: Unknown
- Seems to be 0x0000 when servo is traveling to higher numbers, 0xFFFF when traveling to lower numbers.

## Register 0xFC: Unknown
- Cycles from 0x0000->0x0001->...->0x0004->0x0000 with a period of about 1 second.

## Register 0x10: Actual motor power
- Signed integer.
- Appears to describe the actual motor power. Ranges from approximately -2000 (max power in one direction) to +2000 (max power in other direction). If motor is not powered, returns zero.
- Uses the same units as 0x56 and 0x22. E.g. if we are hitting the power limit, then 0x10 will read the same value as 0x22, or negative the value of 0x22.

# Procedures
## Initial connection
1. Write 0x22 (=0x1000)
2. Read 0x06 (=0x4ABF), 0x00 (=0x01E5), again 0x00 (=0x01E5), 0x04 (=0x0024), 0x5E (=0x0000), again 0x5E (=0x0000), 0x4C (=0x0001), 0xC4 (=0x0514), again 0xC4 (=0x0514), 0x32 (=0x0000), 0x64 (=0x0FFF), 0x54 (=0x0FFF), 0x4E (=0x0001), 0x60 (=0x0001), 0x8C (=0x6D60), 0x44 (=0x36B0), 0x9C (=0x0064)

## OPEN
1. Read 0xB2 (before even selecting the file)
2. Write 0x72 (=0x4E54), 0x32 (=0x0000), 0x34 (=0x0005), 0x38 (=0x0000), 0x3A (=0x0041), 0x4C (=0x0001), 0x4E (=0x0001), 0x50 (=0x3FFF), 0x52 (=0x0000), 0x54 (=0x0FFF), 0x56 (=0x0FFF), 0x58 (=0x0064), 0x5A (=0x0190), 0x5C (=0x0320), 0x5E (=0x0000), 0x60 (=0x0001), 0x62 (=0x0000), 0x78 (=0x0000), 0x7A (=0x0320), 0x7C (=0x0190), 0x7E (=0x00E8), 0x80 (=0x000A), 0x82 (=0x01F4), 0x84 (=0x0000), 0x86 (=0x07D0), 0x88 (=0x001E), 0x8A (=0x0FA0), 0x8C (=0x6D60), 0x8E (=0x0000), 0x90 (=0x0000), 0x92 (=0x0019), 0x94 (=0x3FFF), 0x96 (=0x0000), 0x98 (=0x00C8), 0x9A (=0x0003), 0x9C (=0x0064), 0x9E (=0x4E20), 0xA0 (=0x0064), 0xA2 (=0x0000), 0xA4 (=0x0005), 0xA6 (=0x0040), 0xA8 (=0x0040), 0xAA (=0x0040), 0xAC (=0x06EA), 0xAE (=0x0668), 0xB0 (=0x32CA), 0xB2 (=0x0D35), 0xC2 (=0x2000), 0xC4 (=0x0514), 0xC0 (=0x0002), 0xB4 (=0x000E), 0xB6 (=0x0010), 0xB8 (=0x0000), 0xBA (=0x0014), 0x3C (=0x0003), 0x3E (=0x000A), 0x40 (=0x0096), 0x42 (=0x0005), 0x44 (=0x36B0), 0x64 (=0x0FFF), 0x66 (=0x0005), 0x68 (=0x000B), again 0x68 (=0x000B), 0x6A (=0x0002), 0x6C (=0x07D0), 0xCE (=0x000A), 0xD0 (=0x0014), 0xD2 (=0x0032), 0xD4 (=0x07D0), 0xD6 (=0x36B0)
3. Write 0x70 (=0xFFFF)
4. Read 0x06 (=0x4ABF), 0x00 (=0x01E5), again 0x00 (=0x01E5), 0x04 (=0x0024), 0x5E (=0x0000), again 0x5E (=0x0000), 0x4C (=0x0001), 0xC4 (=0x0514), again 0xC4 (=0x0514), 0x32 (=0x0000), 0x64 (=0x0FFF), 0x54 (=0x0FFF), 0x4E (=0x0001), 0x60 (=0x0001), 0x8C (=0x6D60), 0x44 (=0x36B0), 0x9C (=0x0064)
5. Write 0x46 (=0x0001), then delay 1 second
6. Write 0x1E (=0x0BB8)

(All delays are 20-50ms unless otherwise noted.)

## SAVE
1. Read 0xB2
2. Read 0x78 (=0x0000), 0x7A (=0x0320), 0x7C (=0x0190), 0x7E (=0x00E8), 0x80 (=0x000A), 0x82 (=0x01F4), 0x84 (=0x0000), 0x86 (=0x07D0), 0x88 (=0x001E), 0x8A (=0x0FA0), 0x8C (=0x6D60), 0x8E (=0x0000), 0x90 (=0x0000), 0x30 (=0x0000), 0x00 (=0x01E5), 0x02 (=0x0004), 0x04 (=0x0024), 0x06 (=0x4ABF), 0x08 (=0x001D), 0xCC (=0x0096), 0xCE (=0x000A), 0x0A (=0x0000), 0x0C (=0x1FDB), 0x0E (=0x0000), 0x10 (=0x0000), 0x12 (=0x0000), 0x14 (=0x0000), 0x32 (=0x0000), 0x34 (=0x0005), 0x36 (=0x0002), 0x38 (=0x0000), 0x3A (=0x0041), 0x46 (=0x0000), 0x48 (=0x0000), 0x4A (=0x0000), 0x4C (=0x0001), 0x4E (=0x0001), 0x50 (=0x3FFF), 0x52 (=0x0000), 0x54 (=0x0FFF), 0x56 (=0x0FFF), 0x58 (=0x0064), 0x5A (=0x0190), 0x5C (=0x0320), 0x5E (=0x0000)
3. Write 0x5E (=0x0000)
4. Write 0x70 (=0xFFFF)
5. Write 0x46 (=0x0001) (followed by 1 second delay)
6. Write 0x22 (=0x1000)
7. Read 0x60 (=0x0001), 0x64 (=0x0FFF), 0x62 (=0x0000), 0x6E (=0x0000), 0x70 (=0x0000), 0x72 (=0x0000), 0x92 (=0x0019), 0x94 (=0x3FFF), 0x96 (=0x0000), 0x98 (=0x00C8), 0x9A (=0x0003), 0x9C (=0x0064), 0x9E (=0x4E20), 0xA0 (=0x0064), 0xA2 (=0x0000), 0xA4 (=0x0005), 0xA6 (=0x0040), 0xA8 (=0x0040), 0xAA (=0x0040), 0xAC (=0x06EA), 0xAE (=0x0668), 0xB0 (=0x32CA), 0xB2 (=0x0D35), 0xC2 (=0x2000), 0xC4 (=0x0514), 0xC0 (=0x0002), 0xB4 (=0x000E), 0xB6 (=0x0010), 0xB8 (=0x0000), 0xBA (=0x00D0), 0xBC (=0x0000), 0x66 (=0x0005), 0x68 (=0x0075), 0x6A (=0x006E), 0x6C (=0x07D0), 0x3C (=0x0003), 0x3E (=0x000A), 0x40 (=0x0096), 0x42 (=0x0005), 0x44 (=0x36B0), 0xCE (=0x000A), 0xD0 (=0x0014), 0xD2 (=0x0032), 0xD4 (=0x07D0), 0xD6 (=0x36B0)

(All delays are around 20-50ms unless noted.)

## Refresh
1. Read 0xB2
2. Read 0x06, 0x00 (twice; same value), 0x04, 0x5E (twice; same value), 0x4C, 0xC4 (twice; same value), 0x32, 0x64, 0x54, 0x4E, 0x60, 0x8C, 0x44, 0x9C

(All delays are around 20-50ms.)

## Program Reset
1. Read 0xB2
2. Write 0x6E
3. Write 0x98, 0x9A
4. Write 0x70
5. Write 0x46 (followed by 1 second delay)
6. Write 0x22
7. Read 0x06, 0x00 (twice; same value), 0x04, 0x5E (twice; same value), 0x4C, 0xC4 (twice; same value), 0x32, 0x64, 0x54, 0x4E, 0x60, 0x8C, 0x44, 0x9C

(All delays are around 20-50ms unless noted.)

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
3. Write 0x70, then wait 1.5 seconds
4. Write 0x46

(All delays are 20-50ms unless otherwise noted)

## Change Sensitivity Ratio
(Note, presets may trigger deadband changes; this assumes no deadband changes.)
1. Write 0x64 (sensitivity ratio)
2. Write 0x70
3. Write 0x46
