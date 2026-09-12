# Simulator Home evidence — `da0511e`

Local, non-flashing WSL simulator evidence for source commit
`da0511ea72cfb25db2a45a5633705401e29c0244`, generated on 2026-09-12.

Each X4 Pro controller profile was rebuilt, launched under Xvfb, and captured
after the Home activity had replaced the boot screen. All captures are valid
480×800, two-colour BMPs with 13,888 foreground pixels and the same SHA-256.
The first SSD1677 capture at 1.8 seconds contained the valid transitional
BOOTING frame rather than Home, so it was rerun at 4 seconds; the retained
evidence below uses that later capture.

## SSD1677

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-da0511e\ssd1677-home.bmp`

SHA-256:
`6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`

## UC8179

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-da0511e\uc8179-boot.bmp`

SHA-256:
`6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`

## UC8279

Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-da0511e\uc8279-boot.bmp`

SHA-256:
`6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`

The simulator fixture has no SD font package and logs missing `/.fonts` and
`/fonts` before rendering the built-in-font Home screen. These results check
only simulated boot and render paths. They do not validate physical waveform
timing, orientation, partial refresh, touch, frontlight, PSRAM, sleep/wake,
Wi-Fi, recovery, or backup restoration.
