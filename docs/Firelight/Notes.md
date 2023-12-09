SDL2 joystick GUID layout:
```
/* This GUID fits the standard form:
 * 16-bit bus
 * 16-bit CRC16 of the joystick name (can be zero)
 * 16-bit vendor ID
 * 16-bit zero
 * 16-bit product ID
 * 16-bit zero
 * 16-bit version
 * 8-bit driver identifier ('h' for HIDAPI, 'x' for XInput, etc.)
 * 8-bit driver-dependent type info
 */
```

