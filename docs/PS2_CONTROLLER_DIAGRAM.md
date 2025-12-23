# PS/2 Controller Architecture & Diagrams

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│ PS/2 Keyboard                                               │
│ (Hardware)                                                  │
└───────┬─────────────────────────────────────────────────────┘
        │
        Scancode (PS/2 Protocol)
        │
        ↓
┌─────────────────────────────────────────────────────────────┐
│ PS/2 Controller (8042)                                      │
│ ┌───────────────────────────────────────────────────────┐ │
│ │ Port 0x60: Data Port                                │ │
│ │ • Read: Scancode from keyboard                   │ │
│ │ • Write: Command to keyboard                     │ │
│ └───────────────────────────────────────────────────────┘ │
│                                                            │
│ ┌───────────────────────────────────────────────────────┐ │
│ │ Port 0x64: Status/Command Port                      │ │
│ │ • Read: Controller status (bits 0-7)              │ │
│ │ • Write: Controller commands (0x20, 0x60, etc)    │ │
│ └───────────────────────────────────────────────────────┘ │
└─────────────┬────────────────────────────────────────────────┘
        │
        Scancode (Translated)
        │
        ↓
┌─────────────────────────────────────────────────────────────┐
│ Kernel (kernel/kernel.c)                                   │
│ • get_key_async() reads scancode from 0x60           │
│ • Converts scancode to ASCII character               │
│ • Returns char code or -1 if no key                  │
└─────────────┬────────────────────────────────────────────────┘
        │
        ASCII Character
        │
        ↓
┌─────────────────────────────────────────────────────────────┐
│ Userspace (userspace/input_manager.cpp)                   │
│ • InputManager.getKey() returns character            │
└─────────────┬────────────────────────────────────────────────┘
        │
        Character Code
        │
        ↓
┌─────────────────────────────────────────────────────────────┐
│ Application (userspace/login_consumer.cpp)               │
│ • Display password dots                              │
│ • Handle backspace, enter, etc                       │
└─────────────────────────────────────────────────────────────┘
```

## Initialization Flow

```
kernel_main()
    ↓
init_ps2_keyboard()
    ↓
[·] Wait for controller ready (status bit 1 = 0)
    ↓
[1] Disable keyboard port (0xAD command)
    ↓
[·] Wait for controller ready
    ↓
[2] Read config byte (0x20 command)
    ↓
[·] Wait for data available (status bit 0 = 1)
    ↓
[3] Read config byte from 0x60
    ↓
[4] Modify config: Set bit 0 (interrupt), Clear bit 4 (disable), Set bit 6 (translate)
    ↓
[·] Wait for controller ready
    ↓
[5] Write config byte (0x60 command)
    ↓
[·] Wait for controller ready
    ↓
[6] Write modified config to 0x60
    ↓
[·] Wait for controller ready
    ↓
[7] Enable keyboard port (0xAE command)
    ↓
[·] Wait for controller ready
    ↓
[8] Send reset command to keyboard (0xFF)
    ↓
[·] Wait for data available
    ↓
[9] Read ACK byte (should be 0xFA)
    ↓
[·] Wait for data available
    ↓
[10] Read self-test result (should be 0xAA)
     ↓
[·] Flush remaining data in buffer
     ↓
Return to kernel_main() → userspace_main()
```

## Status Port Bit Layout

```
┌─────────────────────────────────────────────────────────────┐
│ PS/2 Controller Status Port (0x64)                            │
├─────────────────────────────────────────────────────────────┤
│ Bit 0: Output Buffer Full (1 = data available at 0x60)       │
│         → Check before reading from 0x60                    │
├─────────────────────────────────────────────────────────────┤
│ Bit 1: Input Buffer Full  (1 = controller busy)              │
│         → Wait for 0 before sending commands               │
├─────────────────────────────────────────────────────────────┤
│ Bit 2: System Flag       (1 = system initialized)            │
├─────────────────────────────────────────────────────────────┤
│ Bit 3: A20 Gate          (1 = A20 enabled)                  │
├─────────────────────────────────────────────────────────────┤
│ Bit 4: Keyboard Disabled (1 = keyboard locked)               │
├─────────────────────────────────────────────────────────────┤
│ Bit 5: Transmit Timeout (1 = timeout sending to keyboard)    │
├─────────────────────────────────────────────────────────────┤
│ Bit 6: Receive Timeout  (1 = timeout receiving from keyboard) │
├─────────────────────────────────────────────────────────────┤
│ Bit 7: Parity Error     (1 = parity error on last byte)      │
└─────────────────────────────────────────────────────────────┘

Usage Examples:
  while (inb(0x64) & 0x01) { } // Wait for data available
  while (inb(0x64) & 0x02) { } // Wait for controller ready
```

## Configuration Byte Bit Layout

```
┌─────────────────────────────────────────────────────────────┐
│ Controller Configuration Byte (Read 0x20, Write 0x60)          │
├─────────────────────────────────────────────────────────────┤
│ Bit 0: Keyboard Interrupt Enabled  [SET to 1]                 │
│         • 1 = IRQ1 enabled when data available             │
│         • REQUIRED for proper operation                     │
├─────────────────────────────────────────────────────────────┤
│ Bit 1: Mouse Interrupt Enabled     [NOT USED]                 │
├─────────────────────────────────────────────────────────────┤
│ Bit 2: System Flag                [LEAVE AS-IS]              │
├─────────────────────────────────────────────────────────────┤
│ Bit 3: Reserved                   [LEAVE AS-IS]              │
├─────────────────────────────────────────────────────────────┤
│ Bit 4: Keyboard Disabled          [CLEAR to 0]                │
│         • 1 = keyboard disabled                            │
│         • MUST BE 0 for keyboard to work                   │
├─────────────────────────────────────────────────────────────┤
│ Bit 5: Mouse Disabled             [NOT USED]                 │
├─────────────────────────────────────────────────────────────┤
│ Bit 6: Scancode Translation       [SET to 1]                 │
│         • 1 = XT scancodes translated to AT format        │
│         • Makes scancode_to_ascii lookup work correctly    │
├─────────────────────────────────────────────────────────────┤
│ Bit 7: Reserved                   [LEAVE AS-IS]              │
└─────────────────────────────────────────────────────────────┘

Final Value: 0x41 (0b01000001)
  • Bit 0 = 1 (keyboard interrupt enabled)
  • Bit 6 = 1 (scancode translation enabled)
  • All others = 0
```

## Scancode Flow Example

```
User presses the 'A' key with Shift held

┌─────────────────────────────────────────────────────────────┐
│ Keyboard sends: Scancode 0x2A (Shift pressed)                  │
└─────┬───────────────────────────────────────────────────────┘
        │
        get_key_async():
        │
        1. Read status (0x64): bit 0 is set (data available)
        2. Read scancode (0x60): 0x2A
        3. Check: 0x2A == 0x2A (Shift pressed)?
        4. Set global: shift_pressed = 1
        5. Return: -1 (no character)
        │
┌────┬────────────────────────────────────────────────────────┐
│ Keyboard sends: Scancode 0x1E ('A' key pressed)                │
└────┬────────────────────────────────────────────────────────┘
        │
        get_key_async():
        │
        1. Read status (0x64): bit 0 is set
        2. Read scancode (0x60): 0x1E
        3. Check: 0x1E >= 0x80? No (key press, not release)
        4. Check: shift_pressed? Yes!
        5. Use: scancode_to_ascii_shift[0x1E] = 'A'
        6. Return: 65 (ASCII code for 'A')
        │
┌────┬────────────────────────────────────────────────────────┐
│ Keyboard sends: Scancode 0xB6 (Shift released)                 │
└────┬────────────────────────────────────────────────────────┘
        │
        get_key_async():
        │
        1. Read status (0x64): bit 0 is set
        2. Read scancode (0x60): 0xB6
        3. Check: 0xB6 == 0xB6 (Shift released)?
        4. Set global: shift_pressed = 0
        5. Return: -1 (no character)
        │
└─────────────────────────────────────────────────────────────┘

Result: LoginScreen receives 65 ('A'), displays one password dot
```

## Command Reference

### Controller Commands (written to 0x64)

```
┌─────────────────────────────────────────────────────────────┐
│ 0x20: Read Controller Configuration Byte                     │
│        Response: 1 byte at 0x60                               │
├─────────────────────────────────────────────────────────────┤
│ 0x60: Write Controller Configuration Byte                    │
│        Next write to 0x60: new config byte                   │
├─────────────────────────────────────────────────────────────┤
│ 0xAD: Disable First PS/2 Port (Keyboard)                      │
├─────────────────────────────────────────────────────────────┤
│ 0xAE: Enable First PS/2 Port (Keyboard)                       │
├─────────────────────────────────────────────────────────────┤
│ 0xA7: Disable Second PS/2 Port (Mouse)                        │
├─────────────────────────────────────────────────────────────┤
│ 0xA8: Enable Second PS/2 Port (Mouse)                         │
├─────────────────────────────────────────────────────────────┤
│ 0xD4: Send Command to Mouse (writes to 0x60)                  │
└─────────────────────────────────────────────────────────────┘
```

### Keyboard Device Commands (written to 0x60 after controller ready)

```
┌─────────────────────────────────────────────────────────────┐
│ 0xFF: Reset Keyboard                                          │
│        Keyboard responds: 0xFA (ACK), then 0xAA (self-test ok)  │
├─────────────────────────────────────────────────────────────┤
│ 0xF4: Enable Data Reporting                                   │
│        Keyboard responds: 0xFA (ACK)                            │
├─────────────────────────────────────────────────────────────┤
│ 0xF5: Disable Data Reporting                                  │
│        Keyboard responds: 0xFA (ACK)                            │
└─────────────────────────────────────────────────────────────┘

Response Codes:
  0xFA = Acknowledge (ACK) - command accepted
  0xFE = Resend request - error, send again
  0xAA = Self-test passed
  0xEE = Echo response
```

## Timing Considerations

```
Wait strategies used in init_ps2_keyboard():

Busy-wait for controller ready:
  while (inb(0x64) & 0x02) { }
  └─ Waits until input buffer is empty (bit 1 = 0)
  └─ Must do before sending commands
  └─ Typical wait: < 1ms on real hardware
  └─ On QEMU: usually instant

Busy-wait for data available:
  while ((inb(0x64) & 0x01) == 0) { }
  └─ Waits until output buffer has data (bit 0 = 1)
  └─ Must do before reading from 0x60
  └─ Typical wait: < 100ms on real hardware
  └─ Timeout safety: none (potential issue on slow keyboards)

Optional improvement: Add timeout counter:
  uint32_t timeout = 1000000;  // iterations
  while ((inb(0x64) & 0x01) == 0 && --timeout) { }
  if (!timeout) { /* keyboard not responding */ }
```

## Summary

This diagram shows:
- System architecture from keyboard to application
- Initialization sequence with timing points
- Status and configuration byte layouts
- Command reference
- Scancode flow example

For detailed explanation, see `INPUT_SYSTEM_FIX.md`
