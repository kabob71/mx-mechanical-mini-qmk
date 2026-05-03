MX Mechanical Mini QMK (RP2040)

This is a working QMK setup for the Logitech MX Mechanical Mini using an RP2040.

Includes:
- Working matrix
- Working LED driver
- Fixed key spam issue
- Combo lighting
- Custom brightness behavior

Notes:
- LED ghosting is hardware-related
- LED driver is I2C (0x30)
- Requires delayed LED updates to avoid key spam

Build:
qmk compile -kb caleb/rp2040 -km default

Flash:
copy .uf2 to RPI-RP2
