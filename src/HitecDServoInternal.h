#ifndef HitecDServoInternal_h
#define HitecDServoInternal_h

/* The servo and programmer communicate via a proprietary serial protocol. This
header file contains "lab notes" from reverse-engineering communications between
a Hitec DPC-11 serial programmer and a D485HW servo, mixed with #define'd
constants for the servo's registers.

This is not officially endorsed by Hitec! Many of the details in this file are
just my observations or speculation, and all the register names were made up by
me. */

/* 
Fundamentals
============

The programmer and the servo communicate via a half-duplex serial connection at
115200 baud, inverted polarity, 8 data bits, no parity bit, 1 stop bit. This
serial connection uses the same data line that's used for PWM signals.
- When idle, the line is low (0V)
- When the programmer is transmitting, it drives the line low (0V) or high
  (about 2V for the DPC-11).
- When the servo is transmitting, the programmer pulls the line high, and the
  servo drives it low to transmit. See notes on reading registers below.
The servo also contains a 3k pulldown resistor to ground.

The servo exposes a set of registers, identified by even-numbered 8-bit
addresses. The programmer can read and write these registers with 16-bit values.

Writing a register
------------------
1. The programmer sends:
    - 0x96
    - 0x00
    - address byte of register
    - 0x02
    - low byte of value
    - high byte of value
    - checksum byte: Computed as (0x00 + address + 0x02 + low + high) & 0xFF.
2. That's it. The servo doesn't send anything in response.

Reading a register
------------------
1. The programmer sends:
    - 0x96
    - 0x00
    - address byte of register
    - 0x00
    - checksum byte: Computed as (0x00 + address + 0x00) & 0xFF, which works out
      to the same as the address.
2. Then, both the servo and the programmer drive the line low. It stays this way
  for a period of anywhere from about 1ms-15ms, apparently at random. Then, the
  programmer stops driving the line low and instead weakly pulls it high
  (approximately 1.3mA drive strength).
3. Exactly 15.2ms after the end of the programmer's transmission, the servo
  responds with:
    - 0x69
    - mystery byte: I'm not sure what this is for. It's usually either 0x00,
      0xFF, or 0xFE.
    - address byte of register: The same as sent by the programmer.
    - 0x02
    - low byte of value
    - high byte of value
    - checksum byte: Computed as (mystery + address + 0x02 + low + high) & 0xFF.
4. After the servo completes its transmission, it stops driving the line low,
  allowing the programmer to pull it high. It stays that way until 16ms after
  the programmer originally began pulling the line high.
5. Then, the programmer starts driving the line low again, and the read is over.

Note, all valid registers are even-numbered. Attempting to read an odd-numbered
register gives 0xLLHH, where 0xHH is the high byte of the previous register, and
0xLL is the low byte of the next register. (Incidentally, attempting to read
register 0xFF returns 0x0000, even though register 0x00 has a non-zero low
byte.)

PWM behavior
------------
Of course, the servo also accepts PWM signals.

Pulses shorter than 850us or longer than 2350us are ignored. It's odd that the
upper limit is 2350us; since the lower limit is 850us, and the center is 1500us,
one would expect the upper limit to be 2150us. Also, the RANGE_RIGHT_APV
register is the point where it will go if it receives a 2150us pulse.
Nevertheless, the servo will accept pulses up to 2350us, and will move past the
position specified by RANGE_RIGHT_APV.

Bootup behavior
---------------
When the servo is first powered on, or the REBOOT register is written, it will
ignore both PWM inputs and serial commands for 1000ms. During this period, it
will drive the line low.

About 1.6ms after the servo is powered on, the servo suddenly draws a lot of
current for a few microseconds. The current drawn is more than when the servo
moves normally, but more a much shorter period. This causes the supply voltage
to dip. At the same time, the data line briefly glitches up to a few hundred mV
above ground. Because the serial connection uses inverted polarity, a UART
listening to the data line may interpret this glitch as an 0xFF byte.

If the servo receives a serial command, it will disregard any subsequent PWM
pulses it receives, until it's rebooted by powering off or writing REBOOT.
However, the reverse is not true; if the servo receives PWM pulses, followed by
a serial command, it will respect the serial command as normal.
*/

/*
Registers for settings
======================
*/

/* ID is an arbitrary user-settable identifier from 0 to 254. */
#define HD_REG_ID 0x32

/* DIRECTION sets whether longer PWM pulses make the servo move clockwise or
counterclockwise. */
#define HD_REG_DIRECTION 0x5E
#define HD_DIRECTION_CLOCKWISE 0 /* default */
#define HD_DIRECTION_COUNTERCLOCKWISE 1

/* SPEED sets the servo movement speed.
- SPEED=2 means 10% of max speed, SPEED=4 means 20% of max speed, and so on.
- As an exception, max speed (the default) is SPEED=0xFFF instead of SPEED=20.
- The programmer only allows setting increments of 10%, but in EPA setting mode
  the programmer sets SPEED=5, which would correspond to 25% speed. */
#define HD_REG_SPEED 0x54

/* DEADBAND_1, DEADBAND_2, and DEADBAND_3 together control the servo deadband.
- If deadband == 1, then DEADBAND_1=1; DEADBAND_2=5; and DEADBAND_3=11. This is
  the default setting.
- If deadband > 1, then:
  - DEADBAND_1 = 4*deadband-4
  - DEADBAND_2 = 4*deadband
  - DEADBAND_3 = 4*deadband+6 */
#define HD_REG_DEADBAND_1 0x4E
#define HD_REG_DEADBAND_2 0x66
#define HD_REG_DEADBAND_3 0x68

/* SOFT_START defines the servo's soft-start behavior. */
#define HD_REG_SOFT_START 0x60
#define HD_SOFT_START_20 1 /* default */
#define HD_SOFT_START_40 3
#define HD_SOFT_START_60 6
#define HD_SOFT_START_80 8
#define HD_SOFT_START_100 100

/* RANGE_LEFT_APV, RANGE_RIGHT_APV, and RANGE_CENTER_APV define the servo's
physical range of motion and its neutral point. See HitecDServo.h for an
explanation of "APV". */
#define HD_REG_RANGE_LEFT_APV 0xB2
#define HD_REG_RANGE_RIGHT_APV 0xB0
#define HD_REG_RANGE_CENTER_APV 0xC2

/* FAIL_SAFE defines the servo's fail-safe behavior. Its value is the fail-safe
PWM pulse width in microseconds, or one of the special constants FAIL_SAFE_LIMP
or FAIL_SAFE_OFF. */
#define HD_REG_FAIL_SAFE 0x4C
#define HD_FAIL_SAFE_LIMP 0
#define HD_FAIL_SAFE_OFF 1

/* POWER_LIMIT defines the maximum motor power that the servo can use. It ranges
from 0 (no power) to 2000 (max power). The DPC-11 represents max power as
POWER_LIMIT=0x0FFF; this is treated the same as POWER_LIMIT=2000. */
#define HD_REG_POWER_LIMIT 0x56

/* OVERLOAD_PROTECTION defines what percentage of max power the servo will use
if it detects an overload condition. It ranges from 0 to 100. */
#define HD_REG_OVERLOAD_PROTECTION 0x9C

/* SMART_SENSE_1 and SMART_SENSE_2 control whether Smart Sense is enabled:
- To enable, set SMART_SENSE_1=SS_ENABLE_1 and SMART_SENSE_2=SS_ENABLE_2.
- To disable, set SMART_SENSE_1=SS_DISABLE_1 and SMART_SENSE_2=SS_DISABLE_2. */
#define HD_REG_SMART_SENSE_1 0x44
#define HD_REG_SMART_SENSE_2 0x6C

/* The SS_{ENABLE,DISABLE}_{1,2} registers appear to be read-only. They
always return the same values. */
#define HD_REG_SS_ENABLE_1 0xD6
#define HD_SS_ENABLE_1_CONST 14000
#define HD_REG_SS_ENABLE_2 0xD4
#define HD_SS_ENABLE_2_CONST 2000
#define HD_REG_SS_DISABLE_1 0x8C
#define HD_SS_DISABLE_1_CONST 28000
#define HD_REG_SS_DISABLE_2 0x8A
#define HD_SS_DISABLE_2_CONST 4000

/* SENSITIVITY_RATIO sets the sensitivity ratio. */
#define HD_REG_SENSITIVITY_RATIO 0x64
#define HD_SENSITIVITY_RATIO_MIN 0x0333
#define HD_SENSITIVITY_RATIO_MAX 0x0FFF /* default */

/*
Other important registers
=========================
*/

/* MODEL_NUMBER is the servo model number, e.g. 485 for the Hitec D485HW. */
#define HD_REG_MODEL_NUMBER 0x00

/* Writing SAVE=SAVE_CONST instructs the servo to flush settings from SRAM to
EEPROM. If settings are not saved to EEPROM, they will be lost when the servo
loses power or the REBOOT register is written. */
#define HD_REG_SAVE 0x70
#define HD_SAVE_CONST 0xFFFF

/* Writing REBOOT=REBOOT_CONST reboots the servo's microcontroller.
- The programmer must reboot the servo after changing certain settings.
- Any settings not flushed to EEPROM will be lost.
- The servo will ignore commands for 1000ms after rebooting. (See "Bootup
  behavior" section above.) */
#define HD_REG_REBOOT 0x46
#define HD_REBOOT_CONST 1

/* Writing FACTORY_RESET=FACTORY_RESET_CONST resets the servo to its factory
settings. */
#define HD_REG_FACTORY_RESET 0x6E
#define HD_FACTORY_RESET_CONST 0x0F0F

/* Writing to TARGET instructs the servo to move. This is related to PWM pulse
widths as follows:
    TARGET = 3000 + 4 * (pwm_pulse_width - 1500)
Note, reading back the TARGET register returns a different value; the returned
value seems to be denominated in APV units. */
#define HD_REG_TARGET 0x1E

/* Reading CURRENT_APV returns the actual servo position, measured in APV units.
(This register is sort of the "definition" of APV units.) See HitecDServo.h for
an explanation of what "APV" means. */
#define HD_REG_CURRENT_APV 0x0C

/* The DPC-11 always writes MYSTERY_OP1=MYSTERY_OP1_CONST and
MYSTERY_OP2=MYSTERY_OP2_CONST whenever it changes the OVERLOAD_PROTECTION
setting or resets the servo. I don't know why; perhaps they configure
other parameters of the overload-protection system? (I've named them "_OP1" and
"_OP2" because they seem related to overload-protection somehow.) */
#define HD_REG_MYSTERY_OP1 0x98
#define HD_MYSTERY_OP1_CONST 200
#define HD_REG_MYSTERY_OP2 0x9A
#define HD_MYSTERY_OP2_CONST 3

/* The DPC-11 always writes MYSTERY_DB=MYSTERY_DB_CONST whenever it changes
the deadband or smart-sense settings. I have no idea what this does. Reading
back MYSTERY_DB always returns 0. (I've named it "_DB" because it seems related
to the deadband somehow.) */
#define HD_REG_MYSTERY_DB 0x72
#define HD_MYSTERY_DB_CONST 0x4E54

/*
Mysterious registers
====================

I don't know what the following registers are for. These are just rough notes
about how the registers appear to behave.

- Register 0x04: The DPC-11 always reads register 0x04 when it first connects to
  the servo, at the same time as it reads MODEL_NUMBER. Register 0x04 always
  returns 36. I don't know what this means; maybe a firmware version?

- Register 0x06: The DPC-11 reads register 0x06 at the same time as MODEL_NUMBER
  and register 0x04. It seems to always return a constant value for each servo,
  but that constant differs between different servos of the same model, even
  when reset to factory settings. The values are typically around 19000. I
  speculate these are manufacturing date codes; e.g. 19135 could indicate
  "135th day of the year 2019".

- Register 0xC4: The DPC-11 also reads register 0xC4 during the startup process,
  but at a different time from MODEL_NUMBER. Always returns 1300. I have no idea
  what this means.

- Registers 0x50 and 0x52: When the DPC-11 enters EPA settings mode, it sets
  register 0x50 to 0x3FFF and register 0x52 to 0x0000. Those numbers are the
  minimum and maximum of the APV range, which seems related? But nothing ever
  sets these registers to any other values.

- Register 0x22:
  - Appears to store the _effective_ power limit. In normal operation, this is
    the same as the POWER_LIMIT register, but capped at 2000. If overload
    protection kicks in, this is reduced by the overload protection amount. (For
    example, if POWER_LIMIT=1600 and OVERLOAD_PROTECTION=50, and then overload
    protection kicks in, then register 0x22 will read 800.)
  - Strangely, the DPC-11 sets 0x22 to 0x1000 after any time it changes the
    DIRECTION or RANGE_*_APV registers; after changing the ID register; and when
    it first connects to the servo. I have no idea why the DPC-11 does this,
    because it has no apparent effect. Reading back the 0x22 register always
    returns the effective power limit, not 0x1000.

- Register 0x10: Appears to store the actual motor power. Measured in the same
  units as POWER_LIMIT and register 0x22. Reads 0 if the motor is off, otherwise
  proportional to how much power the motor is exerting. Signed integer. If the
  motor is stalled, then abs(register_0x10)=register_0x22.

- Register 0xE4: When the TARGET register is written, register 0xE4 appears to
  be automatically set to the target point as measured in APV units.

- Register 0xEA: I think this might store the difference between CURRENT_APV and
  register 0xE4?

- Register 0xEC: Appears to always read 0x0000 when the servo is travling to
  higher APVs, and 0xFFFF when the servo is traveling to lower APVs.

- Registers 0xDC and 0xE0: Appear to store approximately the same value as
  CURRENT_APV. Perhaps these are past measurements used for calculating
  time-derivatives?

- Registers 0x0E and 0xDE: Signed integers. Typically 0, 1, or -1 when the servo
  is stationary; larger values when moving. Perhaps time-derivatives of
  CURRENT_APV?

- Register 0xFC: Cycles from 0->1->2->3->4->0->[repeat] with a total period of
  about 1 second.
*/

#endif /* HitecDServoInternal_h */
