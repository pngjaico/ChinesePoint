# Simulator boot evidence — `6d1c448`

Local, non-flashing WSL simulator evidence for source commit
`6d1c448cbc1a485f3c95ad490974641c86dde8c6`, generated on 2026-09-12.
SSD1677, UC8179, and UC8279 each compiled, booted, and produced a valid
480×800 two-colour BMP with 9,977 foreground pixels.

| Panel profile | Capture | SHA-256 |
| --- | --- | --- |
| SSD1677 | `D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-6d1c448\ssd1677-boot.bmp` | `3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a` |
| UC8179 | `D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-6d1c448\uc8179-boot.bmp` | `3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a` |
| UC8279 | `D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-6d1c448\uc8279-boot.bmp` | `3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a` |

The simulator fixture has no SD font package and logs missing `/.fonts` and
`/fonts` before rendering the built-in-font boot screen. This validates neither
CJK reader content nor a physical display. Panel timing, orientation, partial
refresh, touch, frontlight, PSRAM, sleep/wake, recovery, and SD backup restore
remain untested on hardware.
