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

## RP2040 Wiring (Pinout)
vsys on keyboard  → vbus

ground → ground

### Columns → RP2040 Pins

Col 0 → GP21  
Col 1 → GP12  
Col 2 → GP9  
Col 3 → GP10  
Col 4 → GP4  
Col 5 → GP3  
Col 6 → GP17  
Col 7 → GP8  

---

### Rows → RP2040 Pins

Row 0 → GP20  
Row 1 → GP19  
Row 2 → GP26  
Row 3 → GP6  
Row 4 → GP16  
Row 5 → GP5  
Row 6 → GP13  
Row 7 → GP22  
Row 8 → GP7  
Row 9 → GP15  
Row 10 → GP14  
Row 11 → GP18  

---

## LED Driver (I2C)

SDA → GP2
SCL → GP3  
SDB (shutdown) → GP0  

---

### Pull-ups (required)

- GP2 (SDA) → 3.3V through 4.7kΩ  
- GP3 (SCL) → 3.3V through 4.7kΩ  

If LEDs don’t work, check these first
