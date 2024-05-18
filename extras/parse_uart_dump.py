"""If you use a logic analyzer to snoop on the traffic to/from the servo, decode
it as UART, and export it to a binary file, this script can parse it.

Warning: This script is very crude, especially if there is corruption or
glitches.

Usage: python parse_uart_dump.py some_binary_file.bin
"""

import sys

dump = open(sys.argv[1], "rb").read()

REG_NAMES = {
  0x32: "HD_REG_ID",
  0x5E: "HD_REG_DIRECTION",
  0x54: "HD_REG_SPEED",
  0x4E: "HD_REG_DEADBAND_1",
  0x66: "HD_REG_DEADBAND_2",
  0x68: "HD_REG_DEADBAND_3",
  0x60: "HD_REG_SOFT_START",
  0xB2: "HD_REG_RANGE_LEFT_APV",
  0xB0: "HD_REG_RANGE_RIGHT_APV",
  0xC2: "HD_REG_RANGE_CENTER_APV",
  0x4C: "HD_REG_FAIL_SAFE",
  0x56: "HD_REG_POWER_LIMIT",
  0x9C: "HD_REG_OVERLOAD_PROTECTION",
  0x44: "HD_REG_SMART_SENSE_1",
  0x6C: "HD_REG_SMART_SENSE_2",
  0xD6: "HD_REG_SS_ENABLE_1",
  0xD4: "HD_REG_SS_ENABLE_2",
  0x8C: "HD_REG_SS_DISABLE_1",
  0x8A: "HD_REG_SS_DISABLE_2",
  0x64: "HD_REG_SENSITIVITY_RATIO",
  0x00: "HD_REG_MODEL_NUMBER",
  0x70: "HD_REG_SAVE",
  0x46: "HD_REG_REBOOT",
  0x6E: "HD_REG_FACTORY_RESET",
  0x1E: "HD_REG_TARGET",
  0x0C: "HD_REG_CURRENT_APV",
  0x98: "HD_REG_MYSTERY_OP1",
  0x9A: "HD_REG_MYSTERY_OP2",
  0x72: "HD_REG_MYSTERY_DB",
}

def regname(reg: int) -> str:
  return REG_NAMES.get(reg, f"HD_REG[0x{reg:02x}]")

i = 0

def get():
  global i
  i += 1
  return dump[i-1]

def expect_get(expected, context):
  actual = get()
  if actual not in expected:
    expected_str = ",".join(f"0x{e:02x}" for e in expected)
    print(f"error in {context}! actual=0x{actual:02x} expected={expected_str}")
    return False
  return True


def skip_zeros():
  # random glitches can look like null bytes between commands; skip them
  global i
  while i < len(dump) and dump[i] == 0x00:
    i += 1


while i < len(dump):
  if not expect_get([0x96], "start of command"):
    continue
  expect_get([0x00, 0xFF], "mystery byte after start of command")
  reg = get()
  opcode = get()
  
  if opcode == 0x00:
    # reading
    expect_get([(0x00 + reg + 0x00) & 0xFF], "checksum")
    skip_zeros()
    if not expect_get([0x69], "start of reply"):
      continue
    mystery = get()
    expect_get([reg], "echoed reg")
    expect_get([0x02], "constant 0x02")
    low = get()
    high = get()
    data = low + (high << 8)
    msg = f"read {regname(reg)}=0x{data:04x}={data}"
    if not expect_get([(mystery + reg + 0x02 + low + high) & 0xFF], "checksum"):
      msg += " (INVALID CHECKSUM!)"
    print(msg)

  
  elif opcode == 0x02:
    # writing
    low = get()
    high = get()
    data = low + (high << 8)
    msg = f"write {regname(reg)}=0x{data:04x}={data}"
    if not expect_get([(0x00 + reg + 0x02 + low + high) & 0xFF], "checksum"):
      msg += " (INVALID CHECKSUM!)"
    print(msg)
    
  else:
    print(f"error in opcode! actual=0x{opcode:02x} expected=[0,2]")
    continue

  skip_zeros()
