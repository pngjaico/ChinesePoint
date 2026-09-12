# Simulator boot evidence — `404ff1e`

Local, non-flashing WSL simulator evidence for source commit
`404ff1e87d610c92a580c5af8c5f00a07d20fe3d`, generated on 2026-09-12.

The simulator was built and launched separately with each X4 Pro controller
profile. Its SDL renderer ran through Xvfb; a scripted screenshot at 1.8 s and
clean quit at 2.6 s produced a valid 480×800, two-colour BMP with 13,888
foreground pixels for every profile.

## SSD1677

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-404ff1e\ssd1677-boot.bmp`

SHA-256:
`6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`

## UC8179

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-404ff1e\uc8179-boot.bmp`

SHA-256:
`6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`

## UC8279

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-404ff1e\uc8279-boot.bmp`

SHA-256:
`6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`

The simulator fixture has no SD font package and logs missing `/.fonts` and
`/fonts` before it renders the built-in-font boot screen. This proves neither
reader content nor a physical display: panel timing, orientation, partial
refresh, touch, frontlight, PSRAM, sleep/wake, Wi-Fi, recovery, and SD backup
restore remain untested on hardware.
