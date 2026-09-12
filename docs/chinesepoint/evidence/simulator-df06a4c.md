# Simulator boot evidence — `df06a4c`

This is local, non-flashing simulator evidence for source commit
`df06a4c90ff2bf5df23fa474a5fd8b6b50954483`, generated on 2026-09-12 in the
`ChinesePoint-Emulator` WSL distribution. Each native profile compiled, booted,
and generated a 480×800, two-colour BMP. The screenshot verifier reported
9,977 foreground pixels for every capture.

| Panel profile | Capture | SHA-256 |
| --- | --- | --- |
| SSD1677 | `D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-df06a4c\ssd1677-boot.bmp` | `3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a` |
| UC8179 | `D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-df06a4c\uc8179-boot.bmp` | `3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a` |
| UC8279 | `D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-df06a4c\uc8279-boot.bmp` | `3db92f5ea72c790d920b96b11ed91897c99c033afe7bab2d00efa866163ab74a` |

The simulator had no SD font package, so it logged missing `/.fonts` and
`/fonts` directories before showing the built-in-font boot screen. That is an
expected simulator fixture condition, not a successful dictionary or reader
test.

This evidence covers only host boot/render paths. It does **not** demonstrate
physical controller timing, panel orientation, partial refresh, frontlight,
touch, PSRAM, sleep/wake, recovery gesture, SD access, or the automatic backup
restore. Those remain physical release gates.
