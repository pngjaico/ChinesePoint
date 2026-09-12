# Simulator boot evidence — `1c904e4`

Local, non-flashing WSL simulator evidence for source commit
`1c904e4db9273916949b1b1cfd029df33e71aa82`, generated on 2026-09-12.
SSD1677, UC8179, and UC8279 each compiled, booted, and produced a valid
480×800 two-colour BMP with 9,977 foreground pixels.

## SSD1677

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-1c904e4\ssd1677-boot.bmp`

SHA-256:
`3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a`

## UC8179

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-1c904e4\uc8179-boot.bmp`

SHA-256:
`3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a`

## UC8279

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-1c904e4\uc8279-boot.bmp`

SHA-256:
`3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a`

The simulator fixture has no SD font package and logs missing `/.fonts` and
`/fonts` before rendering the built-in-font boot screen. This validates neither
CJK reader content nor a physical display. Panel timing, orientation, partial
refresh, touch, frontlight, PSRAM, sleep/wake, recovery, and SD backup restore
remain untested on hardware.
