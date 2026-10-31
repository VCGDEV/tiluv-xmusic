# Wiring

> ⚠️ **Draft, not verified on the board.** Check every pin against the official Orange Pi Zero 2W user manual
> before applying power, then replace "TBD"/"verify" with the confirmed pin and GPIO name (e.g. `PH7`).

Header pin numbers are physical positions on the 40-pin header.

## ILI9341 TFT (SPI1)

| TFT pin | Signal | Header pin | GPIO | Verified |
|---|---|---|---|---|
| VCC | 3.3 V | 1 or 17 | — | no |
| GND | GND | 6 | — | no |
| SDI / MOSI | SPI1 MOSI | 19 | | no |
| SDO / MISO | SPI1 MISO (optional) | 21 | | no |
| SCK | SPI1 CLK | 23 | | no |
| CS | SPI1 CS0 | 24 | | no |
| DC / RS | GPIO | TBD | | no |
| RESET | GPIO | TBD | | no |
| LED (backlight) | PWM GPIO through a transistor | TBD | | no |

## MAX98357 amplifier (I2S)

⚠️ It's still open whether the H618 exposes I2S on the header at all (see `PENDING_ITEMS.md`, M2 → Audio).

| Amp pin | Signal | Header pin | GPIO | Verified |
|---|---|---|---|---|
| VIN | 5 V | 2 or 4 | — | no |
| GND | GND | 9 | — | no |
| BCLK | I2S bit clock | TBD | | no |
| LRC | I2S frame clock (LRCLK) | TBD | | no |
| DIN | I2S data out | TBD | | no |
| SD | Shutdown / channel select, to a GPIO | TBD | | no |
| GAIN | Unconnected = 9 dB | — | — | no |

## Buttons (gpio-keys)

To be defined in M2 once the free GPIOs are counted.
