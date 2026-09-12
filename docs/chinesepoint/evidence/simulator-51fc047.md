# X4 Pro simulator Home evidence — `51fc047`

This is local, non-flashing simulator evidence for source commit
`51fc0470d54c3217fb35fc2b16cb621b35e290a5`, produced on 2026-09-12 in the
isolated `ChinesePoint-Emulator` WSL distribution.

Each configured X4 Pro controller profile compiled, linked, started under
Xvfb, reached Home, and wrote a BMP at 4 seconds. The dependency-free
verifier accepted every capture as a 480x800, 32-bit, two-colour frame with
13,888 foreground pixels. Visual inspection of the SSD1677 frame shows the
complete Home list with the Browse Files selector; it is not blank, inverted,
or vertically mirrored in the simulator.

## SSD1677

Build duration: 173.71 s. Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-51fc047\ssd1677-home.bmp`.
SHA-256: `6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`.

## UC8179

Build duration: 168.93 s. Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-51fc047\uc8179-home.bmp`.
SHA-256: `6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`.

## UC8279

Build duration: 165.47 s. Capture:
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-12-51fc047\uc8279-home.bmp`.
SHA-256: `6e4ee65cadb961c41cdf2f99b9476b5da52650e8b07cf78c278b7598e82feac4`.

The fixture has no SD font package, so every run logged missing `/.fonts` and
`/fonts` before rendering the built-in-font Home screen. Builds also emitted
the pre-existing Miniz macro-redefinition warning. Neither condition failed a
build, but the absent SD fixture means this is not dictionary, CJK-font, or
reader acceptance evidence.

The simulator does not establish physical controller detection, waveform
timing, orientation, touch, partial refresh, frontlight, PSRAM, sleep/wake,
Wi-Fi, SD flashing, or the DOWN+POWER backup restoration. Those remain
physical release gates.
