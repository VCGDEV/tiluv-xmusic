# Hardware

Everything needed to build the phone. Licensed under GPL-3.0-or-later, like the rest of the repository.

| File | Contents |
|---|---|
| [BOM.md](BOM.md) | Bill of materials per phase, with suppliers in Mexico |
| [wiring.md](wiring.md) | Pin-by-pin wiring between the Orange Pi Zero 2W and each module |

Planned, as the project gets there (see `PENDING_ITEMS.md`):

- `dt/`: device tree overlays for the TFT, the I2S amp and the buttons (M2)
- `diagrams/`: KiCad or Fritzing wiring diagrams, once the pinout is verified on the board (M2)
- `case/`: FreeCAD/OpenSCAD files for the slider enclosure (M7)

## Datasheets and references

Linked rather than copied, since most datasheets can't be redistributed.

| Part | Link |
|---|---|
| Orange Pi Zero 2W (board, user manual, schematics) | [orangepi.org: Orange Pi Zero 2W](http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/service-and-support/Orange-Pi-Zero-2W.html) |
| Armbian for the Zero 2W | [armbian.com/orange-pi-zero-2w](https://www.armbian.com/orange-pi-zero-2w/) |
| ILI9341 display controller | [ILI9341 datasheet (Adafruit mirror)](https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf) |
| MAX98357A I2S amplifier | [analog.com: MAX98357A](https://www.analog.com/en/products/max98357a.html) |
