# Logitech MX Mechanical Mini QMK (RP2040 Conversion)

This project converts the Logitech MX Mechanical Mini to run QMK using an RP2040.

Includes:
- Full matrix reverse engineering
- RP2040 wiring (rows/columns pinout)
- I2C LED driver support (0x30)
- Fix for key spam caused by LED refresh
- Combo lighting (Fn / Ctrl / Shift)
- Custom backlight behavior

Notes:
- LED ghosting is hardware-related (matrix/driver limitation)
- Requires delayed LED updates to avoid key spam
- This is not plug-and-play, requires hardware modding

## Build

This repo contains only the contents of a QMK keyboard folder.

1. Copy these files into:
   qmk_firmware/keyboards/<your_keyboard_name>/

2. Compile:
   qmk compile -kb <your_keyboard_name> -km default

## Flash

Copy the generated `.uf2` file to:
RPI-RP2

---

## RP2040 Wiring

Power:
VSYS (keyboard) → VBUS (RP2040)  
GND → GND  

Columns:

Col 0 → GP21  
Col 1 → GP12  
Col 2 → GP9  
Col 3 → GP10  
Col 4 → GP4  
Col 5 → GP1

Col 6 → GP17  
Col 7 → GP8  

Rows:

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
SDB → GP0  

Pull-ups (required):
GP2 (SDA) → 3.3V through 4.7kΩ  
GP3 (SCL) → 3.3V through 4.7kΩ  

If LEDs don’t work, check pull-ups first.

