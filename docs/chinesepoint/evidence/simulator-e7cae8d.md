# Simulator boot evidence — `e7cae8d`

Local, non-flashing WSL simulator evidence for source commit
`e7cae8d591b8be5fe4e7b2417565ddf21f97e2b2`, generated on 2026-09-12.

SSD1677, UC8179, and UC8279 each compiled, booted, and produced a valid
480×800 two-colour BMP with 9,977 foreground pixels.

## SSD1677

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-e7cae8d\ssd1677-boot.bmp`

SHA-256:
`3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a`

## UC8179

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-e7cae8d\uc8179-boot.bmp`

SHA-256:
`3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a`

## UC8279

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-e7cae8d\uc8279-boot.bmp`

SHA-256:
`3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a`

The simulator fixture has no SD font package and logs missing `/.fonts` and
`/fonts` before rendering the built-in-font boot screen. This validates neither
CJK reader content nor a physical display. Panel timing, orientation, partial
refresh, touch, frontlight, PSRAM, sleep/wake, Wi-Fi, recovery, and SD backup
restore remain untested on hardware.
